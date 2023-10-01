#include "type.h"
#include "fat.h"
#include "disk.h"
#include "memlayout.h"
#include "printf.h"

#define SECTOR_SIZE 512

#pragma pack(push, 1)
typedef struct FAT_BootSector
{
    uint8_t jumpInstruction[3];
    uint8_t oemIdentifier[8];
    uint16_t bytesPerSector;
    uint8_t sectorsPerCluster;
    uint16_t reservedSectors;
    uint8_t fatCount;
    uint16_t rootDirEntries;
    uint16_t totalSectors; // zero if to large, then largeTotalSectors
    uint8_t mediaDescriptor;
    uint16_t sectorsPerFat;
    uint16_t sectorsPerTrack;
    uint16_t heads;
    uint32_t hiddenSectors;
    uint32_t largeTotalSectors;

    uint8_t driveNumber;
    uint8_t _reserved;
    uint8_t signature;
    uint32_t volumeID;
    uint8_t volumeLabel[11];     // padded with spaces
    uint8_t systemIdentifier[8]; // should not be trusted
} FAT_BootSector;
#pragma pack(pop)

typedef struct FAT_Data
{
    union BS
    {
        FAT_BootSector bootSector;
        uint8_t bootSectorBytes[SECTOR_SIZE];
    } BS;
    DISK *disk;
} FAT_Data;

static FAT_Data far *g_Data = NULL;
static uint8_t far *g_Fat = NULL;
static FAT_DirEntry far *g_RootDir = NULL;

bool FAT_readBootSector(DISK *disk)
{
    return DISK_readSectors(disk, 0, 1, g_Data->BS.bootSectorBytes);
}

bool FAT_readFat()
{
    return DISK_readSectors(g_Data->disk, g_Data->BS.bootSector.reservedSectors, g_Data->BS.bootSector.sectorsPerFat, g_Fat);
}

bool FAT_initialize(DISK *disk)
{
    g_Data = (FAT_Data far *)MEMORY_FAT_ADDR;
    g_Data->disk = disk;
    printf("Reading boot sector\n\r");
    if (!FAT_readBootSector(disk))
    {
        printf("Failed to read bootsector\n\r");
        return false;
    }
    g_Fat = (uint8_t far *)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.bootSector.bytesPerSector * g_Data->BS.bootSector.sectorsPerFat;
    if (fatSize + sizeof(FAT_Data) >= MEMORY_FAT_SIZE)
    {
        printf("Not enough memory to load fat\n\r");
        return false;
    }
    printf("Reading fat\n\r");
    if (!FAT_readFat())
    {
        printf("Failed to read fat\n\r");
        return false;
    }
    g_RootDir = (FAT_DirEntry far *)(g_Fat + fatSize);
    return true;
}
