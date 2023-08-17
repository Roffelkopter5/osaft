#include "stdio.h"
#include "x86.h"

#define PRINT_NEUTRAL 0
#define PRINT_FLAGS 1
#define PRINT_WIDTH 2
#define PRINT_LENGTH 3
#define PRINT_SPECIFIER 4

void putc(char c) {
    x86_Video_WriteChar(c, 1);
}

void puts(const char *s) {
    while (*s) {
        putc(*s++);
    }
}


void printf(const char *fmt, ...) {
    uint8_t state = PRINT_NEUTRAL;
    uint8_t flags = 0;
    uint8_t width = 0;
    while (*fmt) {
        switch (state) {
            case PRINT_NEUTRAL:
                if (*fmt == '%') state = PRINT_FLAGS;
                else putc(*fmt);
                fmt++;
                break;
            case PRINT_FLAGS:
                switch (*fmt) {
                    case '-':
                        flags |= 1;
                        break;
                    case '+':
                        flags |= 2;
                        break;
                    case ' ':
                        flags |= 4;
                        break;
                    case '#':
                        flags |= 8;
                        break;
                    case '0':
                        flags |= 16;
                        break;
                    default:
                        state = PRINT_WIDTH;
                }
                if (state == PRINT_FLAGS) fmt++;
                break;
            case PRINT_WIDTH:
                if (*fmt == '*') {
                    flags |= 32;
                    state = PRINT_LENGTH;
                    fmt++;
                    break;
                }
                uint8_t i = 0;
                while (*fmt >= '0' && *fmt <= '9') {
                    i += 1;
                    fmt++;
                }
                uint8_t base = 1;
                for (int j = 1; j <= i; j++) {
                    width += (*(fmt - j) - '0') * base;
                    base *= 10;
                }
                state = PRINT_LENGTH;
                break;
            case PRINT_LENGTH:
                switch (*fmt) {
                    case 'h':
                        break;
                    case 'l':
                        break;
                    default:
                        state = PRINT_SPECIFIER;
                        break;
                }
                if (state == PRINT_LENGTH) fmt++;
                break;
            case PRINT_SPECIFIER:
                break;
        }
    }
}