#include <stdlib.h>
#include <stdio.h>
#include "debug.h"
#include "types.h"
#include "consts.h"

int findAvailableDisk(Bios *bios) {
    int i;
    for (i = 0; (bios->drives[i].signature & DRIVE_EMPTY) && i <= 16; i++);
    return i;
}   

void initdisk(Bios *bios, char* filepath) {
    int disknum = findAvailableDisk(bios);
    if (disknum == 16) return;

    FILE *fp = fopen(filepath, "r");
    fseek(fp, 0, SEEK_END);
    uint64_t disklength = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *diskptr = malloc(disklength * sizeof(uint8_t));
    fread(diskptr, sizeof(uint8_t), disklength, fp);
    bios->drives[disknum].disk = diskptr;
    bios->drives[disknum].limit = disklength;
}

void biosp_loop(Arch *arch) {
    while (arch->processor.on) {
        for (int i = 0; i < 16; i++) {
            Device dev = arch->bios.devices[i];
            for (int j = 0; j < dev.size; j++)
                arch->processor.memory[portsloc+arch->bios.devmapin[i]+j] = dev.buf[j];
        }
    }
}

void biosc_loop(Arch *arch) {
    uint8_t prevclock = 0;
    arch->processor.memory[portsloc+0] = 0;
    while (arch->processor.on) {
        //uint8_t opbyte;
        uint8_t clockbyte = 0;

        do {
            clockbyte = arch->processor.memory[portsloc+0];
        } while (clockbyte != prevclock);
    }
}