#ifndef THREADH
#define THREADH

#include "types.h"
#include "processor.h"

void init_thr(Arch *arch);
int thr_join(Arch *arch);

#endif