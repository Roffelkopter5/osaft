#include "type.h"
#include "printf.h"

void _cdecl cstart_(uint16_t bootDrive)
{
    uint16_t *p = &bootDrive;
    printf("Hell%c %s %d %d %u %u %x %p %f", 'o', "World", 134, -134, 134, -134, 134, p);
}
