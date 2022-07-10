#include "debug.h"
#include "types.h"
#include "consts.h"
#include "stdio.h"

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
        printf("B\n");
        //uint8_t opbyte;
        uint8_t clockbyte = 0;

        do {
            clockbyte = arch->processor.memory[portsloc+0];
        } while (clockbyte != prevclock);
    }
}