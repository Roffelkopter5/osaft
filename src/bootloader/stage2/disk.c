#include "disk.h"
#include "x86.h"

bool DISK_Initialize(DISK *disk, uint8_t driveNumber)
{
    uint8_t driveType;
    uint16_t cylinders, heads, sectors;

    disk->id = driveNumber;
    if (!x86_Disk_GetParams(driveNumber, &driveType, &cylinders, &heads, &sectors))
        return false;
    disk->cylinders = cylinders;
    disk->heads = heads;
    disk->sectors = sectors;
    return true;
}

void DISK_LBA2CHS(DISK *disk, uint32_t lba, uint16_t *cylinderOut, uint16_t *headOut, uint16_t *sectorOut)
{
    *sectorOut = (lba % disk->sectors) + 1;
    *cylinderOut = (lba / disk->sectors) / disk->heads;
    *headOut = (lba / disk->sectors) % disk->heads;
}

bool DISK_Read_Sectors(DISK *disk, uint32_t lba, uint8_t sectors, uint8_t far *dataOut)
{
    uint16_t cylinder, head, sector;

    DISK_LBA2CHS(disk, lba, &cylinder, &head, &sector);

    for (int i = 0; i < 3; i++)
    {
        if (x86_Disk_Read(disk->id, cylinder, head, sector, sectors, dataOut))
            return true;
        x86_Disk_Reset(disk->id);
    }

    return false;
}
