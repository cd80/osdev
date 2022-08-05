#ifndef __utility_h__
#define __utility_h__

#include "types.h"

void progress(int x, int y, const char *string, BOOL success);
DWORD printat(int x, int y, const char *string);
void memset(void *dest, BYTE data, unsigned int size);
unsigned int memcpy(void *dest, void *src, unsigned int size);
int memcmp(void *dest, void *src, unsigned int size);

#endif