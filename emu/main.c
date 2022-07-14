#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "processor.h"
#include "bios.h"
#include "consts.h"
#include "types.h"
#include "debug.h"
#include "thread.h"
#include "bootrom.h"

int main() {
    Arch arch;

    Drive empty = {.disk = NULL, .limit = 0, .signature = DRIVE_EMPTY};
    for (int i = 0; i < 16; i++) arch.bios.drives[i] = empty;

    initmem(&arch.processor, 0x7FFFFF+1);
    
    Drive onload = {.disk = malloc(512), .limit = 512, .signature = DRIVE_SYS | DRIVE_R | DRIVE_LOAD};
    memcpy(&arch.processor.memory[0x7E00], bootrom, 512);
    memcpy(onload.disk, bootrom, 512);
    arch.bios.drives[15] = onload;

    Drive hd0 = {.disk = NULL, .limit = 0, .signature = DRIVE_GENERAL | DRIVE_R | DRIVE_W };
    initdisk(&hd0, "test.bin");
    int drivenumber = findAvailableDisk(&arch.bios);
    if (drivenumber < 16) {
        printf(DBGTAG_BIOS"Loading %s as hd%x\n", CALC_TICKSDOT, "test.bin", drivenumber);
        arch.bios.drives[drivenumber] = hd0;
    }

    printf(DBGTAG_BIOS"DRIVE   PRMS   SIZE\n", CALC_TICKSDOT);
    for (int i = 0; i < 16; i++) {
        printf(DBGTAG_BIOS, CALC_TICKSDOT);
        displayDrive(&arch.bios.drives[i], i);
    }

    init_thr(&arch);
    thr_join(&arch);

    cleanproc(&arch.processor);

    double seconds = (double)(clock() - arch.processor.start) / CLOCKS_PER_SEC;
    printf(DBGTAG_INFODUMP"finished %I64d instructions in %fs (%f instructions/sec)\n", CALC_TICKSDOT, arch.processor.instructions, seconds, arch.processor.instructions/seconds);

    
    //#ifdef DEBUG
        printf(DBGTAG_INFODUMP"CPUDUMP:\n", CALC_TICKSDOT);
        printf(DBGTAG_INFODUMP"Registers:\n", CALC_TICKSDOT);
        printf(DBGTAG_INFODUMP" | GA=%08x GB=%08x GC=%08x GD=%08x\n"DBGTAG_INFODUMP" | GE=%08x GF=%08x GG=%08x\n",
            CALC_TICKSDOT, arch.processor.registers[ga], arch.processor.registers[gb], arch.processor.registers[gc], arch.processor.registers[gd],
            CALC_TICKSDOT, arch.processor.registers[ge], arch.processor.registers[gf], arch.processor.registers[gg]);
        printf(DBGTAG_INFODUMP" | pv=%d", CALC_TICKSDOT, (arch.processor.registers[fl] & 0b1100000000000000) >> 14);
        if (arch.processor.registers[fl] & fz) printf(", Z");
        if (arch.processor.registers[fl] & fc) printf(", C");
        if (arch.processor.registers[fl] & fs) printf(", S");
        if (arch.processor.registers[fl] & fo) printf(", O");
        if (arch.processor.registers[fl] & fi) printf(", I");
        if (arch.processor.registers[fl] & fe) printf(", E");
        printf("\n");
        printf(DBGTAG_INFODUMP" | PC=%08x, SP=%08x, BP=%08x\n", CALC_TICKSDOT, arch.processor.registers[pc], arch.processor.registers[sp], arch.processor.registers[bp]);
   //#endif
    printf("\n\n%I64d\n\n", sizeof(Arch));
    return 0;
}