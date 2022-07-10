#include <stdio.h>
#include "processor.h"
#include "consts.h"
#include "types.h"
#include "debug.h"
#include "thread.h"

int main() {
    Arch arch;

    initdisk(&arch.processor, "test.bin");
    initmem(&arch.processor, 0xFFFF);

    init_thr(&arch);
    thr_join(&arch);

    clean(&arch.processor);

    double seconds = (double)(clock() - arch.processor.start) / CLOCKS_PER_SEC;
    printf(DBGTAG_INFODUMP"finished %I64d instructions in %fs (%f instructions/sec)\n", CALC_TICKSDOT, arch.processor.instructions, seconds, arch.processor.instructions/seconds);

    
    #ifdef DEBUG
        printf(DBGTAG_INFODUMP"CPUDUMP:\n", CALC_TICKSDOT);
        printf(DBGTAG_INFODUMP"Registers:\n", CALC_TICKSDOT);
        printf(DBGTAG_INFODUMP" | GA=%08x GB=%08x GC=%08x GD=%08x\n"DBGTAG_INFODUMP" | GE=%08x GF=%08x GG=%08x\n",
            CALC_TICKSDOT, arch.processor.registers[ga], arch.processor.registers[gb], arch.processor.registers[gc], arch.processor.registers[gd],
            CALC_TICKSDOT, arch.processor.registers[ge], arch.processor.registers[gf], arch.processor.registers[gg]);
        printf(DBGTAG_INFODUMP" | pv=%d ", CALC_TICKSDOT, (arch.processor.registers[fl] & 0b1100000000000000) >> 14);
        if (testbit(arch.processor.registers[fl], fz)) printf("fz ");
        if (testbit(arch.processor.registers[fl], fc)) printf("fc ");
        if (testbit(arch.processor.registers[fl], fs)) printf("fs ");
        printf("\n");
        printf(DBGTAG_INFODUMP" | PC=%08x, SP=%08x, BP=%08x\n", CALC_TICKSDOT, arch.processor.registers[pc], arch.processor.registers[sp], arch.processor.registers[bp]);
    #endif

    return 0;
}