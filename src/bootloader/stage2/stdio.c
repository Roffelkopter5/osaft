#include "stdio.h"
#include "x86.h"

void putc(char c) {

}

void puts(const char *s) {
    while (*s) {
        putc(*s++);
    }
}