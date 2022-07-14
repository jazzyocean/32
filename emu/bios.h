#ifndef BIOSH
#define BIOSH

#include "types.h"

void biosc_loop(Arch *arch);
void biosp_loop(Arch *arch);
void initdisk(Drive *bios, char* filepath);
int findAvailableDisk(Bios *bios);
void displayDrive(Drive *drive, int num);

#endif