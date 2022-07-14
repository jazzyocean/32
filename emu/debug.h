#ifndef DEBUGH
#define DEBUGH

#define DEBUG
#define VERBOSE

#define DBGTAG_MEMACCERR "%012I32de[MEMV  ] "
#define DBGTAG_INVALIDOP "%012I32de[INVOP ] "
#define DBGTAG_VERBOSE   "%012I32di[INST  ] "
#define DBGTAG_INFODUMP  "%012I32di[CPU   ] "
#define DBGTAG_THREAD    "%012I32di[THREAD] "
#define DBGTAG_BIOS      "%012I32di[BIOS  ] "

#define CALC_TICKS (uint32_t)((double)(clock() - cpu->start) / CLOCKS_PER_SEC*1000)
#define CALC_TICKSDOT (uint32_t)((double)(clock() - arch.processor.start) / CLOCKS_PER_SEC*1000)
#define CALC_TICKSARR (uint32_t)((double)(clock() - arch->processor.start) / CLOCKS_PER_SEC*1000)

#endif