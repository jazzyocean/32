#ifndef TYPESH
#define TYPESH

#include <stdint.h>
#include <time.h>
#include <pthread.h>

typedef struct _device {
    uint32_t signature;
    uint8_t buf[8];
    uint8_t size;
} Device;

typedef struct _vram {
    uint32_t signature;
    uint8_t buf[8];
    uint8_t size;
} Vram;

typedef struct drive {
    uint8_t *disk;
    uint64_t limit;
    uint32_t signature;
} Drive;

typedef enum DRIVESIGNATURE {
    DRIVE_EMPTY = 0b1, DRIVE_R = 0b10, DRIVE_RW = 0b100, DRIVE_SYS = 0b1000,
    DRIVE_GENERAL = 0b10000, DRIVE_REG = 0b100000, DRIVE_LOAD = 0b1000000
} DRIVESIGTYPE;

typedef struct _bios {
    uint8_t cpuports[65536];
    Device devices[16];
    Drive drives[16];
    Vram vram;
    uint16_t devmapin[17];
    uint16_t devmapout[17];
    pthread_t biosc_thr_id;
    pthread_t biosp_thr_id;
} Bios;

typedef struct _processor {
    uint64_t instructions;
    uint32_t registers[16];
    uint8_t *memory;
    uint64_t memlimit;
    uint8_t intid;
    clock_t start;
    pthread_t cpu_thr_id;
    int on;
} Processor;

typedef struct _arch {
    Processor processor;
    Bios bios;
    pthread_barrier_t barrier;
} Arch;

#endif