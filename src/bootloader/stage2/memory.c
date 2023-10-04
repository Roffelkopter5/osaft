#include "memory.h"
#include "type.h"

void far *memset(void far *ptr, uint8_t v, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++)
    {
        *((uint8_t far *)ptr + i) = v;
    }
    return ptr;
}

void far *memcpy(void far *dest, const void far *src, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++)
    {
        *((uint8_t far *)dest + i) = *((uint8_t far *)src + i);
    }
    return dest;
}

bool memcmp(const void far *ptrA, const void far *ptrB, uint16_t n)
{
    for (uint16_t i = 0; i < n; i++)
    {
        if (*((uint8_t far *)ptrA + i) != *((uint8_t far *)ptrB + i))
            return false;
    }
    return true;
}
