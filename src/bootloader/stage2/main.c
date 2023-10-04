#include "type.h"
#include "printf.h"
#include "disk.h"
#include "fat.h"
#include "memory.h"

void _cdecl cstart_(uint16_t bootDrive)
{
    DISK floppy;
    DISK_initialize(&floppy, bootDrive);
    FAT_initialize(&floppy);
    FAT_File *far file = FAT_openFile("test.txt");
    char buffer[32];
    FAT_readFile(file, 31, buffer);
    buffer[31] = 0;
    printf("%s\n\r", buffer);
    for (;;)
        ;
}
