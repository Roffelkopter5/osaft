#include "type.h"
#include "fat.h"
#include "disk.h"
#include "memlayout.h"
#include "printf.h"
#include "string.h"
#include "memory.h"

#define SECTOR_SIZE 512
#define MAX_OPEN_FILES 10
#define ROOT_DIR_HANDLE 0
#define MAX_NAME_LENGTH 12

#define MIN(a, b) ((a < b) ? a : b)

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

typedef struct FAT_FileData
{
    uint8_t buffer[SECTOR_SIZE];
    FAT_File file;
    bool isOpen;
    uint32_t startCluster;
    uint32_t currentCluster;
    uint32_t currentSector;
} FAT_FileData;

typedef struct FAT_Data
{
    union BS
    {
        FAT_BootSector bootSector;
        uint8_t bootSectorBytes[SECTOR_SIZE];
    } BS;
    DISK *disk;
    FAT_FileData openFiles[MAX_OPEN_FILES + 1];
    FAT_FileData *rootDir;
} FAT_Data;

static FAT_Data far *g_Data = NULL;
static uint8_t far *g_Fat = NULL;
bool FAT_readBootSector()
{
    return DISK_readSectors(g_Data->disk, 0, 1, g_Data->BS.bootSectorBytes);
}

bool FAT_readFat()
{
    return DISK_readSectors(g_Data->disk, g_Data->BS.bootSector.reservedSectors, g_Data->BS.bootSector.sectorsPerFat, g_Fat);
}

bool FAT_openRootDir()
{
    g_Data->rootDir = &g_Data->openFiles[ROOT_DIR_HANDLE];
    g_Data->rootDir->file.handle = ROOT_DIR_HANDLE;
    g_Data->rootDir->file.index = 0;
    g_Data->rootDir->file.isDirectory = true;
    g_Data->rootDir->file.size = g_Data->BS.bootSector.rootDirEntries * sizeof(FAT_DirEntry);
    g_Data->rootDir->isOpen = true;
    // g_Data->rootDir->startCluster = g_Data->BS.bootSector.reservedSectors + g_Data->BS.bootSector.fatCount * g_Data->BS.bootSector.sectorsPerFat;
    // g_Data->rootDir->currentSector = g_Data->rootDir->startCluster;
    // g_Data->rootDir->currentSector = 0;

    DISK_readSectors(g_Data->disk, g_Data->BS.bootSector.reservedSectors + g_Data->BS.bootSector.fatCount * g_Data->BS.bootSector.sectorsPerFat, 1, g_Data->rootDir->buffer);

    printf("F");

    return true;
}

bool FAT_initialize(DISK *disk)
{
    g_Data = (FAT_Data far *)MEMORY_FAT_ADDR;
    g_Data->disk = disk;
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
    if (!FAT_readFat())
    {
        printf("Failed to read fat\n\r");
        return false;
    }
    if (!FAT_openRootDir())
    {
        printf("Failed to open root dir\n\r");
        return false;
    }
    return true;
}

bool FAT_findFile(FAT_File *dir, const char *name, FAT_DirEntry *entryOut)
{
    return false;
}

FAT_File far *FAT_openFile(const char *path)
{
    printf("Opening: %s\n\r", path);

    // remove leading slash
    if (*path == '/')
        path++;

    char *slash;
    char name[MAX_NAME_LENGTH];
    uint16_t n;
    FAT_File *currentDir = &g_Data->rootDir->file;
    FAT_DirEntry entry;

    do
    {
        slash = strchr(path, '/');
        n = slash ? slash - path : MIN(MAX_NAME_LENGTH - 1, strlen(path));
        printf("Slash at %p (offset: %d)\n\r", slash, n);
        memcpy(name, path, n);
        path += n + 1;
        name[n] = 0;
        if (FAT_findFile(currentDir, name, &entry))
        {
        }
        else
        {
            printf("Error: '%s' is not a directory!\n\r", name);
            return NULL;
        }
    } while (slash);

    return NULL;
}
