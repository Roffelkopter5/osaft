#include "type.h"
#include "x86.h"

#define PRINT_NEUTRAL 0
#define PRINT_FLAGS 1
#define PRINT_WIDTH 2
#define PRINT_LENGTH 3
#define PRINT_SPECIFIER 4

#define PRINT_LENGTH_SHORT_SHORT 0
#define PRINT_LENGTH_SHORT 1
#define PRINT_LENGTH_DEFAULT 2
#define PRINT_LENGTH_LONG 3
#define PRINT_LENGTH_LONG_LONG 4

static char *g_hexchars = "0123456789ABCDEF";

void putc(char c)
{
    x86_Video_WriteChar(c, 1);
}

void puts(const char *s)
{
    while (*s)
    {
        putc(*s++);
    }
}

void putsr(const char *s, unsigned int length)
{
    while (length)
    {
        putc(s[--length]);
    }
}

uint32_t strlen(const char *s)
{
    uint32_t len = 0;
    while (*s)
    {
        s++;
        len++;
    }
    return len;
}

void printf_print(const char *buffer, unsigned int length, unsigned int width, uint8_t flags)
{
    if (flags & 1)
    {
        if (flags & 32)
            putsr(buffer, length);
        else
            puts(buffer);
        while (length < width)
        {
            putc(flags & 16 ? '0' : ' ');
            length++;
        }
    }
    else
    {
        while (length < width)
        {
            putc(flags & 16 ? '0' : ' ');
            length++;
        }
        if (flags & 32)
            putsr(buffer, length);
        else
            puts(buffer);
    }
}

int *printf_number(int *args, int width, int length, uint8_t flags, int base)
{
    unsigned int offset;
    uint64_t number;
    char buffer[32];
    unsigned int pos = 0;
    uint8_t sign = flags & 32;
    switch (length)
    {
    case PRINT_LENGTH_SHORT_SHORT:
    case PRINT_LENGTH_SHORT:
    case PRINT_LENGTH_DEFAULT:
        if (sign)
        {
            signed int n = *args;
            if (n < 0)
            {
                n = -n;
                sign = 1;
            }
            else
            {
                sign = 0;
            }
            number = (uint64_t)n;
        }
        else
        {
            number = (uint64_t)((unsigned int)*args);
        }
        offset = 1;
        break;
    case PRINT_LENGTH_LONG:
        if (sign)
        {
            signed long int n = *args;
            if (n < 0)
            {
                n = -n;
                sign = 1;
            }
            else
            {
                sign = 0;
            }
            number = (uint64_t)n;
        }
        else
        {
            number = (uint64_t)((unsigned long int)*args);
        }
        offset = 2;
        break;
    case PRINT_LENGTH_LONG_LONG:
        if (sign)
        {
            signed long long int n = *args;
            if (n < 0)
            {
                n = -n;
                sign = 1;
            }
            else
            {
                sign = 0;
            }
            number = (uint64_t)n;
        }
        else
        {
            number = (uint64_t)*args;
        }
        offset = 4;
        break;
    }
    do
    {
        uint32_t rem;
        x86_div64_32(number, base, &number, &rem);
        buffer[pos++] = g_hexchars[rem];
    } while (number > 0);
    if (sign)
    {
        buffer[pos] = '-';
    }
    else if (flags & 2)
    {
        buffer[pos] = '+';
    }
    else if (flags & 4)
    {
        buffer[pos] = ' ';
    }
    else
    {
        pos--;
    }

    printf_print(buffer, pos + 1, width, flags | 32);

    return args + offset;
}

void printf(const char *fmt, ...)
{
    uint8_t state = PRINT_NEUTRAL;
    uint8_t flags = 0;
    int width = 0;
    int *args = (int *)&fmt + 1;
    char *to_print = 0;
    unsigned int size, base, i, j, length;
    while (*fmt)
    {
        switch (state)
        {
        case PRINT_NEUTRAL:
            if (*fmt == '%')
                state = PRINT_FLAGS;
            else
                putc(*fmt);
            fmt++;
            flags = 0;
            length = 2;
            width = 0;
            size = 0;
            to_print = 0;
            break;
        case PRINT_FLAGS:
            switch (*fmt)
            {
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
            if (state == PRINT_FLAGS)
                fmt++;
            break;
        case PRINT_WIDTH:
            if (*fmt == '*')
            {
                state = PRINT_LENGTH;
                fmt++;
                break;
            };
            i = 0;
            while (*fmt >= '0' && *fmt <= '9')
            {
                i++;
                fmt++;
            }
            j = 1;
            base = 1;
            while (j <= i)
            {
                width += (*(fmt - j) - '0') * base;
                base *= 10;
                j++;
            }
            state = PRINT_LENGTH;
            break;
        case PRINT_LENGTH:
            if (length == 0 || length == 4)
            {
                state = PRINT_NEUTRAL;
            }
            else
            {
                switch (*fmt)
                {
                case 'h':
                    length--;
                    break;
                case 'l':
                    length++;
                    break;
                default:
                    state = PRINT_SPECIFIER;
                    break;
                }
                if (state == PRINT_LENGTH)
                    fmt++;
            }
            break;
        case PRINT_SPECIFIER:
            switch (*fmt)
            {
            case '%':
                putc('%');
                break;
            case 's':
                printf_print(*(char **)args, strlen(*(char **)args), width, flags);
                args++;
                break;
            case 'c':
                printf_print((char *)args, 1, width, flags);
                args++;
                break;
            case 'd':
            case 'i':
                args = printf_number(args, width, length, flags | 32, 10);
                break;
            case 'u':
                args = printf_number(args, width, length, flags, 10);
                break;
            case 'o':
                args = printf_number(args, width, length, flags, 8);
                break;
            case 'x':
            case 'X':
                args = printf_number(args, width, length, flags, 16);
                break;
            }
            fmt++;
            state = PRINT_NEUTRAL;
            break;
        }
    }
}
