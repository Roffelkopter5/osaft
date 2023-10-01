#include "string.h"

char *strchr(const char *s, char c)
{
    while (*s && *s != c)
        s++;
    return (*s == c) ? (char *)s : NULL;
}

uint16_t strlen(const char *s)
{
    const char *s0 = s;
    while (*s)
        s++;
    return s - s0;
}