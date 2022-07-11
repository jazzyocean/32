#ifndef BIOSH
#define BIOSH

#include "types.h"

void biosc_loop(Arch *arch);
void biosp_loop(Arch *arch);
void initdisk(Bios *bios, char* filepath);

#endif