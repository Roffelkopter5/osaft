#include "stdio.h"
#include "x86.h"

void putc(char c) {
    x86_Video_WriteChar(c, 1);
}

void puts(const char *s) {
    while (*s) {
        putc(*s++);
    }
}
