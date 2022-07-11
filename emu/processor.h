#ifndef EMUH
#define EMUH

#include <stdint.h>
#include <time.h>
#include "types.h"

void initdisk(Processor *cpu, char* filepath);
void initmem(Processor *cpu, uint64_t memlimit);
int testbit(uint32_t v, int b);
void cleanproc(Processor *cpu);
void mainloop(Arch *arch);
void displaymem(uint8_t *buf, uint64_t length);

#endif