#ifndef __mode_switch_h__
#define __mode_switch_h__

#include "types.h"

void read_cpuid(DWORD cmd, DWORD *eax, DWORD *ebx, DWORD *ecx, DWORD *edx);
void switch_to_64bit(void);

#endif