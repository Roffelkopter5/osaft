#include "type.h"
#include "printf.h"
#include "disk.h"
#include "fat.h"

void _cdecl cstart_(uint16_t bootDrive)
{
    DISK floppy;
    DISK_initialize(&floppy, bootDrive);
    FAT_initialize(&floppy);
    for (;;)
        ;
}
