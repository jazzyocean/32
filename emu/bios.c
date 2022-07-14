#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "types.h"
#include "consts.h"
#include "processor.h"

#define PORT_BIOS_CTRL 1
#define PORT_BIOS_CLOCK 2
#define PORT_BIOS_DATA 3

static uint8_t clockprev = 0;

int findAvailableDisk(Bios *bios) {
    int i;
    for (i = 0; !(bios->drives[i].signature & DRIVE_EMPTY) && (i <= 16); i++);
    return i;
}   

void initdisk(Drive *drive, char* filepath) {
    FILE *fp = fopen(filepath, "r");
    fseek(fp, 0, SEEK_END);
    uint64_t disklength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *diskptr = malloc(disklength * sizeof(uint8_t));
    fread(diskptr, sizeof(uint8_t), disklength, fp);
    drive->disk = diskptr;
    drive->limit = disklength;
}

void displayDrive(Drive *drive, int num) {
    if (drive->signature & DRIVE_EMPTY) {
        printf("[dsc]   ...    0B\n");
    } else {
        if (drive->signature & DRIVE_GENERAL)   printf("[hd%x]   ", num);
        else if (drive->signature & DRIVE_REG)  printf("[rg%x]   ", num);
        else if (drive->signature & DRIVE_LOAD) printf("[ld%x]   ", num);
        
        if (drive->signature & DRIVE_R) printf("R"); else printf(".");
        if (drive->signature & DRIVE_W) printf("W"); else printf(".");
        if (drive->signature & DRIVE_SYS) printf("S"); else printf(".");

        printf("    %I64dB\n", drive->limit);
    }
}


void biosp_loop(Arch *arch) {
    for (int i = 0; i < 16; i++) {
        arch->bios.devices[i].signature = 0;
        arch->bios.devices[i].size = 0;
        arch->bios.devmapin[i] = 0;
        arch->bios.devmapout[i] = 0;
    }

    while (arch->processor.on) {
        for (int i = 0; i < 16; i++) {
            Device dev = arch->bios.devices[i];
            for (int j = 0; j < dev.size; j++)
                arch->processor.memory[portsloc+arch->bios.devmapin[i]+j] = dev.buf[j];
        }
    }
}

uint8_t inb(Arch *arch, int add) {
    uint8_t clockbyte = 0;
    clockprev = arch->processor.memory[portsloc+PORT_BIOS_CLOCK];

    do {
        clockbyte = arch->processor.memory[portsloc+PORT_BIOS_CLOCK];
    } while ((clockbyte == clockprev) && (arch->processor.on));
    
    return arch->processor.memory[portsloc+PORT_BIOS_DATA+add];
}

uint16_t inw(Arch *arch) {
    uint8_t clockbyte = 0;
    clockprev = arch->processor.memory[portsloc+PORT_BIOS_CLOCK];

    do {
        clockbyte = arch->processor.memory[portsloc+PORT_BIOS_CLOCK];
    } while (clockbyte == clockprev && arch->processor.on);
    
    return arch->processor.memory[portsloc+PORT_BIOS_DATA] | (arch->processor.memory[portsloc+PORT_BIOS_DATA+1] << 8);
}

uint32_t indw(Arch *arch) {
    uint8_t clockbyte = 0;
    clockprev = arch->processor.memory[portsloc+PORT_BIOS_CLOCK];

    do {
        clockbyte = arch->processor.memory[portsloc+PORT_BIOS_CLOCK];
    } while (clockbyte == clockprev && arch->processor.on);
    
    return arch->processor.memory[portsloc+PORT_BIOS_DATA] | (arch->processor.memory[portsloc+PORT_BIOS_DATA+1] << 8) |
           (arch->processor.memory[portsloc+PORT_BIOS_DATA+2] << 16) | (arch->processor.memory[portsloc+PORT_BIOS_DATA+3] << 24);
}

void sendb(Arch *arch, uint8_t data) {
    arch->processor.memory[portsloc+PORT_BIOS_DATA] = data;
    arch->processor.memory[portsloc+PORT_BIOS_CLOCK]++;
}


void sendw(Arch *arch, uint16_t data) {
    arch->processor.memory[portsloc+PORT_BIOS_DATA] = data & 0xFF;
    arch->processor.memory[portsloc+PORT_BIOS_DATA+1] = data << 8;
    arch->processor.memory[portsloc+PORT_BIOS_CLOCK]++;
}


void senddw(Arch *arch, uint32_t data) {
    arch->processor.memory[portsloc+PORT_BIOS_DATA] = data & 0xFF;
    arch->processor.memory[portsloc+PORT_BIOS_DATA] = (data << 8) & 0xFF;
    arch->processor.memory[portsloc+PORT_BIOS_DATA] = (data << 16) & 0xFF;
    arch->processor.memory[portsloc+PORT_BIOS_DATA] = (data << 24) & 0xFF;
    arch->processor.memory[portsloc+PORT_BIOS_CLOCK]++;
}

void biosc_loop(Arch *arch) {
    arch->processor.memory[portsloc+PORT_BIOS_CLOCK] = 0;
    while (arch->processor.on) {

        uint8_t command = inb(arch, -2);
        if (!arch->processor.on) return;
        printf("Command: %x\n", command);
        switch (command) {
            case 0xA0: {
                // Send number of connected drives.
                int count = 0;
                for (int i = 0; i < 16; i++) {
                    if (!arch->bios.drives[i].signature&DRIVE_EMPTY) {
                        count++;
                    }
                }
                sendb(arch, count);
                break;
            } case 0xA1: {
                // Send number of logical drives.
                sendb(arch, 16);
            } case 0xA2: {
                // Cache drive
                arch->processor.memory[portsloc+PORT_BIOS_CLOCK]++;
                arch->bios.cached_disk = inb(arch, 0);
            } case 0xA3: {
                // Set drive pointer
                arch->bios.drives[arch->bios.cached_disk].ptr = indw(arch);
            } case 0xA4: {
                // return byte
                Drive *drive = &arch->bios.drives[arch->bios.cached_disk];
                sendb(arch, drive->disk[drive->ptr]);
                drive->ptr += 1;
            } case 0xA5: {
                // return word
                Drive *drive = &arch->bios.drives[arch->bios.cached_disk];
                sendw(arch, drive->disk[drive->ptr] | (drive->disk[drive->ptr+1] << 8));
                drive->ptr += 2;
            } case 0xA6: {
                // return dword
                Drive *drive = &arch->bios.drives[arch->bios.cached_disk];
                senddw(arch, drive->disk[drive->ptr] | (drive->disk[drive->ptr+1] << 8) |
                            (drive->disk[drive->ptr+2] << 16) | (drive->disk[drive->ptr+3] << 24));
                drive->ptr += 4;
            }
        }

        arch->processor.memory[portsloc+PORT_BIOS_CLOCK]++;
    }
}