#ifndef __types_h__
#define __types_h__

#define BYTE    unsigned char
#define WORD    unsigned short
#define DWORD   unsigned int
#define QWORD   unsigned long long
#define BOOL    unsigned char

#define TRUE    1
#define FALSE   0
#define NULL    0

#pragma pack(push, 1)

typedef struct _CHARACTER {
    BYTE character;
    BYTE attr;
} CHARACTER;

#pragma pack(pop)

#endif