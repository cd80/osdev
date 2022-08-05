#include "utility.h"

void progress(int x, int y, const char *string, BOOL success) {
    CHARACTER *screen = (CHARACTER *)0xB8000;
    char *msg_success = success ? "PASS" : "FAIL";
    char attr = success ? 0x07 : 0x0c;
    DWORD str_len = 0;

    if (success != TRUE && success != FALSE) {
        // Wrong call to progress
        // ex) progress(0, 0, "haha", func); => progress(0, 0, "haha", func());
        for (int i = 0; i < 80; ++i) {
            screen[i].character = 0;
            screen[i].attr = 0x0c;
        }
        printat(0, 0, "wrong call to progress: ");
        printat(24, 0, string);
    }
    screen += x + y * 80;
    str_len = printat(x, y, string);
    for(; str_len < 75; ++str_len) {
        screen[str_len].character = '.';
    }
    screen[74].character ='[';

    for(int i=0; msg_success[i] != 0; ++i) {
        screen[i+75].character = msg_success[i];
        screen[i+75].attr = attr;
    }

    screen[79].character = ']';
    
    if(msg_success[0] == 'F') {
        while (1) {}
    }
}

DWORD printat(int x, int y, const char *string) {
    DWORD str_len = 0;
    CHARACTER *screen = (CHARACTER *)0xB8000;
    screen += x + y * 80;
    for(int i=0; string[i] != 0; ++i) {
        str_len++;
        screen[i].character = string[i];
    }

    return str_len;
}

void memset(void *dest, BYTE data, unsigned int size) {
    for(int i = 0; i < size; ++i) {
        ((char *)dest)[i] = data;
    }
}

unsigned int memcpy(void *dest, void *src, unsigned int size) {
    for(int i = 0; i < size; ++i) {
        ((char *)dest)[i] = ((char *)src)[i];
    }

    return size;
}

int memcmp(void *dest, void *src, unsigned int size) {
    int diff = 0;
    for(int i = 0; i < size; ++i) {
        diff = ((char *)dest)[i] - ((char *)src)[i];
        if(diff) {
            return diff;
        }
    }
    return 0;
}