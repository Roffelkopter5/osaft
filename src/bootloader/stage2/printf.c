#include "type.h"
#include "x86.h"
#include "printf.h"

static char digits[17] = "0123456789ABCDEF";

void print_int(int value, unsigned int base, bool sign)
{
    char buffer[16];
    uint16_t x;
    if (sign && (sign = value < 0))
    {
        x = -value;
    }
    else
    {
        x = value;
    }
    int i = 0;
    do
    {
        buffer[i++] = digits[x % base];
    } while ((x /= base) != 0);
    if (sign)
        x86_Video_WriteChar('-', 1);
    for (; i > 0; i--)
    {
        x86_Video_WriteChar(buffer[i - 1], 1);
    }
}

void print_ptr(uint16_t x)
{
    int i;
    x86_Video_WriteChar('0', 1);
    x86_Video_WriteChar('x', 1);
    for (i = 0; i < (sizeof(uint16_t) * 2); i++, x <<= 4)
        x86_Video_WriteChar(digits[x >> (sizeof(uint16_t) * 8 - 4)], 1);
}

void printf(const char *fmt, ...)
{
    char c;
    char *s;
    int *args = (int *)&fmt + 1;
    for (; (c = *fmt) != 0; fmt++)
    {
        if (c != '%')
        {
            x86_Video_WriteChar(c, 1);
            continue;
        }
        c = *(++fmt);
        switch (c)
        {
        case 'd':
            print_int(*(args++), 10, true);
            break;
        case 'u':
            print_int(*(args++), 10, false);
            break;
        case 'x':
            print_int(*(args++), 16, false);
            break;
        case 'p':
            print_ptr((uint16_t) * (args++));
            break;
        case 'c':
            x86_Video_WriteChar(*(args++), 1);
            break;
        case 's':
            if ((s = (char *)*(args++)) == 0)
                s = "(null)";
            for (; *s; s++)
                x86_Video_WriteChar(*s, 1);
            break;
        default:
            x86_Video_WriteChar('%', 1);
            x86_Video_WriteChar(c, 1);
            break;
        }
    }
}
