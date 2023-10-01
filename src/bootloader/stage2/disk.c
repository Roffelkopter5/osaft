#include "disk.h"
#include "type.h"
#include "x86.h"
#include "printf.h"

bool DISK_initialize(DISK *disk, uint8_t driveNumber)
{
    uint8_t driveType;
    uint16_t cylinders, heads, sectors;
    printf("Reading disk parameters\n\r");
    if (!x86_Disk_GetParams(driveNumber, &driveType, &cylinders, &heads, &sectors))
    {
        printf("Failed to read disk parameters\n\r");
        return false;
    }
    disk->id = driveNumber;
    disk->cylinders = cylinders + 1;
    disk->heads = heads + 1;
    disk->sectors = sectors;
    printf("DISK: driveNumber: %d cylinders: %d heads: %d sectors: %d\n\r", disk->id, disk->cylinders, disk->heads, disk->sectors);
    return true;
}

void DISK_LBA2CHS(DISK *disk, uint32_t lba, uint16_t *cylinderOut, uint16_t *headOut, uint16_t *sectorOut)
{
    *sectorOut = (lba % disk->sectors) + 1;
    *cylinderOut = (lba / disk->sectors) / disk->heads;
    *headOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_readSectors(DISK *disk, uint32_t lba, uint8_t sectors, uint8_t far *dataOut)
{
    uint16_t cylinder, head, sector;

    printf("Reading sectors %p to %p \n\r", (uint16_t)lba, (uint16_t)lba + sectors);

    DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);

    printf("Starting at cylinder: %d, head: %d, sector: %d\n\r", cylinder, head, sector);

    for (int i = 0; i < 3; i++)
    {
        if (x86_Disk_Read(disk->id, cylinder, head, sector, sectors, dataOut))
        {
            printf("Finished reading %u sector(s)\n\r", (uint16_t)sectors);
            return true;
        }
        x86_Disk_Reset(disk->id);
        printf("Retrying\n\r");
    }

    return false;
}
