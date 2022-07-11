#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "consts.h"
#include "debug.h"
#include "types.h"

#define srcreg regbyte&0b1111
#define dstreg regbyte>>4

int opcode_lookup[256] = {
//  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 00
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 10
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 30
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // 40
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // 50
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // 60
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // 70
   -2,-2,-2, 0,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // 80
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // 90
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // A0
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // B0
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // C0
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // D0
   -1,-1,-1,-1,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // E0
   -2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2,-2, // F0
};

char regnames[16][5] = {
    "NULL", "GA", "GB", "GC", "GD", "GE", "GF", "GG",
    "BP", "SP", "PC", "FL", "NULL", "NULL", "NULL", "NULL"
};

void initmem(Processor *cpu, uint64_t memlimit) {
    uint8_t *memptr = malloc(memlimit * sizeof(uint8_t));
    cpu->memory = memptr;
    cpu->memlimit = memlimit-1;
}

void cleanproc(Processor *cpu) {
    free(cpu->memory);
}

uint32_t setflag(uint32_t i, int f) {
    return i | (1UL << f);
}

uint32_t clearflag(uint32_t i, int f) {
    return i & ~(1UL << f);
}

int testbit(uint32_t v, int b) {
    return (v & (1 << b)) >> b;
}

uint64_t lea(uint32_t addr) {
    return (uint64_t)addr;
}

uint8_t mgetb(Processor *cpu, uint32_t addr) {
    if (lea(addr) >= cpu->memlimit) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Out-of-bounds memory access (effective address $%I64x, limit $%I64x)\n", CALC_TICKS, lea(addr), cpu->memlimit);
        #endif

        return 0;
    }

    return cpu->memory[lea(addr)];
}

uint16_t mgetw(Processor *cpu, uint32_t addr) {
    if (lea(addr+1) > cpu->memlimit) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Out-of-bounds memory access (effective address $%I64x, limit $%I64x)\n", CALC_TICKS, lea(addr), cpu->memlimit);
        #endif

        return 0;
    }

    return (cpu->memory[lea(addr)]) | cpu->memory[lea(addr+1)] << 8;
}

uint32_t mgetdw(Processor *cpu, uint32_t addr) {
    if (lea(addr+3) > cpu->memlimit) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Out-of-bounds memory access (effective address $%I64x, limit $%I64x)\n", CALC_TICKS, lea(addr), cpu->memlimit);
        #endif

        return 0;
    }

    return cpu->memory[lea(addr)] | (cpu->memory[lea(addr+1)] << 8)
           | (cpu->memory[lea(addr+2)] << 16) | (cpu->memory[lea(addr+3)] << 24);
}

void msetb(Processor *cpu, uint32_t addr, uint8_t value) {
    if (lea(addr) > cpu->memlimit) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Out-of-bounds memory access (effective address $%I64x, limit $%I64x)\n", CALC_TICKS, lea(addr), cpu->memlimit);
        #endif

        return;
    }

    cpu->memory[lea(addr)] = value;
}

void msetw(Processor *cpu, uint32_t addr, uint16_t value) {
    if (lea(addr+1) > cpu->memlimit) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Out-of-bounds memory access (effective address $%I64x, limit $%I64x)\n", CALC_TICKS, lea(addr), cpu->memlimit);
        #endif

        return;
    }

    cpu->memory[lea(addr)] = value & 0xFF;
    cpu->memory[lea(addr+1)] = (value >> 8) & 0xFF;
}

void msetdw(Processor *cpu, uint32_t addr, uint16_t value) {
    if (lea(addr+3) > cpu->memlimit) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Out-of-bounds memory access (effective address $%I64x, limit $%I64x)\n", CALC_TICKS, lea(addr), cpu->memlimit);
        #endif

        return;
    }

    cpu->memory[lea(addr)] = value & 0xFF;
    cpu->memory[lea(addr+1)] = (value >> 8) & 0xFF;
    cpu->memory[lea(addr+2)] = (value >> 16) & 0xFF;
    cpu->memory[lea(addr+3)] = value >> 24;
}

void pushb(Processor *cpu, uint8_t val) {
    cpu->memory[cpu->registers[sp]++] = val;
    if (cpu->registers[sp] > cpu->memlimit) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Out-of-bounds stack pointer (SP=$%x, memory limit $%I64x)\n", CALC_TICKS, cpu->registers[sp], cpu->memlimit);
        #endif

        return;
    }
}

void pushw(Processor *cpu, uint16_t val) {
    pushb(cpu, val & 0xFF);
    pushb(cpu, (val >> 8) & 0xFF);
}

void pushdw(Processor *cpu, uint16_t val) {
    pushb(cpu, val & 0xFF);
    pushb(cpu, (val >> 8) & 0xFF);
    pushb(cpu, (val >> 16) & 0xFF);
    pushb(cpu, val >> 24);
}

uint8_t popb(Processor *cpu) {
    if (cpu->registers[sp]-1 < cpu->registers[bp]) {
        cpu->registers[fl] = setflag(cpu->registers[fl], fo);
        #ifdef DEBUG
            printf(DBGTAG_MEMACCERR"Stack underflow (SP=$%x, limit $%x)\n", CALC_TICKS, cpu->registers[sp]-1, cpu->registers[bp]);
        #endif
        return 0;
    }
    return cpu->memory[--cpu->registers[sp]];
}

uint16_t popw(Processor *cpu) {
    return (popb(cpu) << 8) | popb(cpu);
}

uint32_t popdw(Processor *cpu) {
    return (popb(cpu) << 24) | (popb(cpu) << 16) | (popb(cpu) << 8) | popb(cpu);
}

void skipinstruction(Processor *cpu) {
    int temp = 0;
    
    do {
        temp = opcode_lookup[cpu->registers[pc]++];
    } while (temp < 0);

    cpu->registers[pc] += temp;
}

void displaymem(uint8_t *buf, uint64_t length) {
    printf("         00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    char text[17] = {0};
    int textptr = 0;
    unsigned i;
    for (i = 0; i < length; i++) {
        if (i % 16 == 0) {
            textptr = 0;
            printf("  %s\n%08x ", text, i);
        }
        printf("%02x ", buf[i]);
        text[textptr++] = (buf[i] != 0xa && buf[i] != 0xd && buf[i] !=  0) ? buf[i] : '.';
    }
    if (i % 16 != 0) {
        
    }
    printf("\n");
}

void instruction(Processor *cpu) {
    cpu->instructions++;

    #ifdef VERBOSE
        uint32_t starting_pc = cpu->registers[pc];
    #endif
    uint8_t byte = mgetb(cpu, cpu->registers[pc]++);
    int length = opcode_lookup[byte];

    if (length == -1) {
        int flag = 0;
        int test = 0;
        switch (byte) {
            case 0xe0: {
                flag = fz;
                test = 1;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> czr\n", CALC_TICKS, starting_pc);
                #endif
                break;
            } case 0xe1: {
                flag = fz;
                test = 0;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> cnzr\n", CALC_TICKS, starting_pc);
                #endif
                break;
            } case 0xe2: {
                flag = fc;
                test = 1;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> ccr\n", CALC_TICKS, starting_pc);
                #endif
                break;
            } case 0xe3: {
                flag = fc;
                test = 0;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> cncr\n", CALC_TICKS, starting_pc);
                #endif
                break;
            }
        }

        if (testbit(cpu->registers[fl], flag) == test)
            instruction(cpu);
        else
            skipinstruction(cpu);
    } else if (length == -2) {
        #ifdef DEBUG
            printf(DBGTAG_INVALIDOP"0x%08x>> Invalid opcode %02x\n", CALC_TICKS, cpu->registers[pc], byte);
        #endif
    } else {
        uint8_t regbyte = 0;
        uint32_t imm = 0;

        if (byte < 0x28 || byte == 0x2F || (byte >= 0x34 && byte != 0x37 && byte != 0x3B && byte != 0x3E && byte != 0x3F)) {
            regbyte = mgetb(cpu, cpu->registers[pc]++);
        }

        if (byte == 0x01 || byte == 0x11 || byte == 0x15 || byte == 0x19 ||
            byte == 0x1d || byte == 0x21 || byte == 0x25 || byte == 0x28 ||
            byte == 0x2c || byte == 0x33) {
                imm = mgetb(cpu, cpu->registers[pc]++);
            }
        else if (byte == 0x02 || byte == 0x12 || byte == 0x16 || byte == 0x1A ||
                byte == 0x1e || byte == 0x22 || byte == 0x26 || byte == 0x29 ||
                byte == 0x2d || byte == 0x37) {
                    imm = mgetw(cpu, cpu->registers[pc]);
                    cpu->registers[pc] += 2;
                }
        else if (byte == 0x03 || byte == 0x13 || byte == 0x17 || byte == 0x1B ||
                byte == 0x1F || byte == 0x23 || byte == 0x27 || byte == 0x2A ||
                byte == 0x2e || byte == 0x38) {
                    imm = mgetdw(cpu, cpu->registers[pc]);
                    cpu->registers[pc] += 4;
                }

        switch (byte) {
            case 0x00: {    // mov reg, reg
                cpu->registers[dstreg] = cpu->registers[srcreg];
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> mov %s, %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x01: case 0x02: case 0x03: {    // mov imm(3), reg
                cpu->registers[dstreg] = imm;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> mov $%x, %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            }
            
            case 0x04: {    // ldr byte ptr imm32, reg
                cpu->registers[dstreg] = mgetb(cpu, imm);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> ldr byte [%08x], %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            } case 0x05: {    // ldr word ptr imm32, reg
                cpu->registers[dstreg] = mgetw(cpu, imm);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> ldr word [%08x], %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            } case 0x06: {    // ldr dword ptr imm32, reg
                cpu->registers[dstreg] = mgetdw(cpu, imm);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> ldr dword [%08x], %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            } case 0x07: {    // ldr byte ptr reg, reg
                cpu->registers[dstreg] = mgetb(cpu, cpu->registers[srcreg]);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> ldr byte [%s], %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x08: {    // ldr word ptr reg, reg
                cpu->registers[dstreg] = mgetw(cpu, cpu->registers[srcreg]);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> ldr word [%s], %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x09: {    // ldr dword ptr reg, reg
                cpu->registers[dstreg] = mgetdw(cpu, cpu->registers[srcreg]);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> ldr dword [%s], %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            }

            case 0x0A: {    // str reg, byte ptr imm32
                msetb(cpu, imm, cpu->registers[srcreg] & 0xFF);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> str %s, byte [%08x]\n", CALC_TICKS, starting_pc, regnames[dstreg], imm);
                #endif
                break;
            } case 0x0B: {    // str reg, word ptr imm32
                msetw(cpu, imm, cpu->registers[srcreg] & 0xFFFF);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> str %s, word [%08x]\n", CALC_TICKS, starting_pc, regnames[dstreg], imm);
                #endif
                break;
            } case 0x0C: {    // str dword ptr imm32, reg
                msetdw(cpu, imm, cpu->registers[srcreg]);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> str %s, dword [%08x]\n", CALC_TICKS, starting_pc, regnames[dstreg], imm);
                #endif
                break;
            } case 0x0D: {    // str byte ptr reg, reg
                msetb(cpu, cpu->registers[dstreg], cpu->registers[srcreg]&0xFF);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> str %s, byte [%s]\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x0E: {    // str word ptr reg, reg
                msetw(cpu, cpu->registers[dstreg], cpu->registers[srcreg]&0xFFFF);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> str %s, word [%s]\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x0F: {    // str dword ptr reg, reg
                msetdw(cpu, cpu->registers[dstreg], cpu->registers[srcreg]&0xFFFFFFFF);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> str %s, dword [%s]\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            }
            
            case 0x10: {    // add reg, reg
                // uint32_t before = cpu->registers[dstreg];
                // cpu->registers[dstreg] += cpu->registers[srcreg];
                // if (cpu->registers[dstreg] < before) {
                //     cpu->registers[fl] = setflag(cpu->registers[fl], fc);
                // }

                uint64_t res = cpu->registers[dstreg] + cpu->registers[srcreg];
                if (res & 0b100000000000000000000000000000000) {
                    cpu->registers[fl] = setflag(cpu->registers[fl], fc);
                } else {
                    cpu->registers[fl] = clearflag(cpu->registers[fl], fc);
                }
                cpu->registers[dstreg] = (uint32_t)res;

                // cpu->registers[dstreg] += cpu->registers[srcreg];                

                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> add %s, %s (%08x)\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg], cpu->registers[dstreg]);
                #endif
                break;
            } case 0x11: case 0x12: case 0x13: {    // add imm(3), reg
                uint64_t res = cpu->registers[dstreg] + imm;
                if (res & 0b100000000000000000000000000000000) {
                    cpu->registers[fl] = setflag(cpu->registers[fl], fc);
                } else {
                    cpu->registers[fl] = clearflag(cpu->registers[fl], fc);
                }
                cpu->registers[dstreg] = (uint32_t)res;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> add $%x, %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            } case 0x14: {    // adc reg, reg
                uint64_t res = cpu->registers[dstreg] + imm + testbit(cpu->registers[fl], fc);
                if (res & 0b100000000000000000000000000000000) {
                    cpu->registers[fl] = setflag(cpu->registers[fl], fc);
                } else {
                    cpu->registers[fl] = clearflag(cpu->registers[fl], fc);
                }
                cpu->registers[dstreg] = (uint32_t)res;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> adc %s, %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x15: case 0x16: case 0x17: {    // adc imm(3), reg
                cpu->registers[dstreg] += imm + testbit(cpu->registers[fl], fc);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> adc $%x, %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            }

            case 0x18: {    // sub reg, reg
                cpu->registers[dstreg] -= cpu->registers[srcreg];
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> sub %s, %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x19: case 0x1A: case 0x1B: {    // sub imm(3), reg
                cpu->registers[dstreg] -= imm;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> sub $%x, %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            } case 0x1C: {    // sbc reg, reg
                cpu->registers[dstreg] -= cpu->registers[srcreg];
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> sbc %s, %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x1D: case 0x1E: case 0x1F: {    // sbc imm(3), reg
                cpu->registers[dstreg] -= imm - testbit(cpu->registers[fl], fc);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> sbc $%x, %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            }
            
            case 0x20: {    // mul reg, reg
                uint64_t res = (uint64_t)cpu->registers[srcreg] * (uint64_t)cpu->registers[dstreg];
                cpu->registers[dstreg] = res & 0xFFFFFFFF;
                cpu->registers[ga] = res >> 32;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> mul %s, %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x21: case 0x22: case 0x23: {    // mul imm(3), reg
                uint64_t res = (uint64_t)imm * (uint64_t)cpu->registers[dstreg];
                cpu->registers[dstreg] = res & 0xFFFFFFFF;
                cpu->registers[ga] = res >> 32;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> mul $%x, %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            }
            
            case 0x24: {    // div reg, reg
                cpu->registers[dstreg] /= cpu->registers[srcreg];
                cpu->registers[ga] = cpu->registers[dstreg] % cpu->registers[srcreg];
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> div %s, %s\n", CALC_TICKS, starting_pc, regnames[srcreg], regnames[dstreg]);
                #endif
                break;
            } case 0x25: case 0x26: case 0x27: {    // div imm(3), reg
                cpu->registers[dstreg] /= imm;
                cpu->registers[ga] = cpu->registers[dstreg] % imm;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> div $%x, %s\n", CALC_TICKS, starting_pc, imm, regnames[dstreg]);
                #endif
                break;
            }

            case 0x28: case 0x29: case 0x2A: {
                pushdw(cpu, cpu->registers[pc]);
                cpu->registers[pc] = imm;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> jsr $%x\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            }

            case 0x2B: {
                cpu->registers[pc] = popdw(cpu);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> rts\n", CALC_TICKS, starting_pc);
                #endif
                break;
            }

            case 0x2C: {
                pushb(cpu, imm);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push $%x\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            } case 0x2D: {
                pushw(cpu, imm);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push $%x\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            } case 0x2E: {
                pushdw(cpu, imm);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push $%x\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            } case 0x2F: {
                pushdw(cpu, cpu->registers[srcreg]);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push %s\n", CALC_TICKS, starting_pc, regnames[srcreg]);
                #endif
                break;
            }

            case 0x30: {
                pushb(cpu, mgetb(cpu, imm));
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push byte [$%x]\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            } case 0x31: {
                pushw(cpu, mgetw(cpu, imm));
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push word [$%x]\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            } case 0x32: {
                pushdw(cpu, mgetdw(cpu, imm));
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push dword [$%x]\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            }

            case 0x34: {
                pushb(cpu, mgetb(cpu, cpu->registers[srcreg]));
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push byte [%s]\n", CALC_TICKS, starting_pc, regnames[srcreg]);
                #endif
                break;
            } case 0x35: {
                pushw(cpu, mgetw(cpu, cpu->registers[srcreg]));
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push word [%s]\n", CALC_TICKS, starting_pc, regnames[srcreg]);
                #endif
                break;
            } case 0x36: {
                pushdw(cpu, mgetdw(cpu, cpu->registers[srcreg]));
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> push dword [%s]\n", CALC_TICKS, starting_pc, regnames[srcreg]);
                #endif
                break;
            }

            case 0x38: {
                cpu->registers[dstreg] = popb(cpu);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> popb %s\n", CALC_TICKS, starting_pc, regnames[dstreg]);
                #endif
                break;
            } case 0x39: {
                cpu->registers[dstreg] = popw(cpu);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> popw %s\n", CALC_TICKS, starting_pc, regnames[dstreg]);
                #endif
                break;
            } case 0x3A: {
                cpu->registers[dstreg] = popdw(cpu);
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> pop %s\n", CALC_TICKS, starting_pc, regnames[dstreg]);
                #endif
                break;
            }

            case 0x33: case 0x37: case 0x3B: {
                if (cpu->registers[gc] != 0)
                    cpu->registers[pc] = imm;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> jcnz $%x\n", CALC_TICKS, starting_pc, imm);
                #endif
                break;
            }

            case 0x3E:
                regbyte = gc<<4;
            case 0x3C: {
                cpu->registers[dstreg]++;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> inc %s\n", CALC_TICKS, starting_pc, regnames[dstreg]);
                #endif
                break;
            }
            
            case 0x3F:
                regbyte = gc<<4;    // a
            case 0x3D: {
                cpu->registers[dstreg]--;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> dec %s\n", CALC_TICKS, starting_pc, regnames[dstreg]);
                #endif
                break;
            }

            case 0x83: {
                cpu->on = 0;
                #ifdef VERBOSE
                    printf(DBGTAG_VERBOSE"0x%08x>> hlt\n", CALC_TICKS, starting_pc);
                #endif
                break;
            }

            default: {
                cpu->registers[pc]++;
                #ifdef DEBUG
                    printf(DBGTAG_INVALIDOP"%08x>> Invalid opcode (how tf) %02x\n", cpu->registers[pc], CALC_TICKS, byte);
                #endif
                break;
            }
        }
    }
}

void mainloop(Arch *arch) {
    Processor *cpu = &arch->processor;
    cpu->registers[pc] = 0x7E00;
    cpu->registers[fl] = 0;
    cpu->start = clock();

    cpu->on = 1;
    while (cpu->on) {
        cpu->registers[0] = 0;
        instruction(cpu);

        if (testbit(cpu->registers[fl], fi)) {
            pushdw(cpu, cpu->registers[pc]);
            cpu->registers[pc] = 4*cpu->intid;
            cpu->registers[fl] = clearflag(cpu->registers[fl], fi);
        }
    }
}