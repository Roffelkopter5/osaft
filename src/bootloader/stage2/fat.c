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
#define MAX_NAME_LENGTH 11
#define MAX_ENTRY_COUNT 5
#define ENTRY_ATTR_DIR 0x10

#define MIN(a, b) ((a < b) ? a : b)
#define UPPER(c) (('a' <= c && c <= 'z') ? c - 'a' + 'A' : c)

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
    FAT_File file;
    bool isOpen;
    uint32_t startCluster;
    uint32_t currentCluster;
    uint32_t currentSector;
    uint8_t buffer[SECTOR_SIZE];
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
    FAT_FileData far *rootDir;
    uint32_t dataSection;
} FAT_Data;

static FAT_Data far *g_Data = NULL;
static uint8_t far *g_Fat = NULL;

uint32_t FAT_clusterToSector(uint32_t cluster)
{
    return g_Data->dataSection + (cluster - 2) * g_Data->BS.bootSector.sectorsPerCluster;
}

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
    g_Data->rootDir = &(g_Data->openFiles[ROOT_DIR_HANDLE]);
    g_Data->rootDir->file.handle = ROOT_DIR_HANDLE;
    g_Data->rootDir->file.index = 0;
    g_Data->rootDir->file.isDirectory = true;
    g_Data->rootDir->file.size = g_Data->BS.bootSector.rootDirEntries * sizeof(FAT_DirEntry);
    g_Data->rootDir->isOpen = true;
    g_Data->rootDir->startCluster = g_Data->BS.bootSector.reservedSectors + g_Data->BS.bootSector.fatCount * g_Data->BS.bootSector.sectorsPerFat;
    g_Data->rootDir->currentCluster = g_Data->rootDir->startCluster;
    g_Data->rootDir->currentSector = 0;

    DISK_readSectors(g_Data->disk, g_Data->rootDir->startCluster, 1, g_Data->rootDir->buffer);

    g_Data->dataSection = g_Data->rootDir->startCluster + (g_Data->rootDir->file.size + g_Data->BS.bootSector.bytesPerSector - 1) / g_Data->BS.bootSector.bytesPerSector;

    return true;
}

bool FAT_initialize(DISK *disk)
{
    g_Data = (FAT_Data far *)MEMORY_FAT_ADDR;
    g_Data->disk = disk;
    if (!FAT_readBootSector())
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

bool FAT_closeFile(FAT_File far *file)
{
    if (file->handle == ROOT_DIR_HANDLE)
    {
        g_Data->rootDir->currentCluster = g_Data->rootDir->startCluster;
        file->index = 0;
    }
    else
    {
        g_Data->openFiles[file->handle].isOpen = false;
    }
}

FAT_File far *FAT_openEntry(FAT_DirEntry *entry)
{
    uint8_t handle = 0;

    for (int i = 1; i <= MAX_OPEN_FILES; i++)
    {
        if (!g_Data->openFiles[i].isOpen)
        {
            handle = i;
            break;
        }
    }

    if (handle == 0)
    {
        printf("No more file handles available\n\r");
        return NULL;
    }

    printf("Handle: %d\n\r", handle);

    FAT_FileData far *fileData = &(g_Data->openFiles[handle]);

    fileData->file.handle = handle;
    fileData->file.index = 0;
    fileData->file.isDirectory = entry->attributes & ENTRY_ATTR_DIR;
    fileData->file.size = entry->size;
    fileData->startCluster = entry->firstClusterLow && ((uint32_t)entry->firstClusterHigh << 16);
    fileData->currentCluster = fileData->startCluster;
    fileData->currentSector = 0;

    if (!DISK_readSectors(g_Data->disk, FAT_clusterToSector(fileData->currentCluster), 1, fileData->buffer))
    {
        printf("Failed to read sector\n\r");
        return NULL;
    }

    fileData->isOpen = true;

    return &fileData->file;
}

bool FAT_findFile(FAT_File far *dir, const char *name, FAT_DirEntry *entryOut)
{
    for (int i = 0; i < MAX_ENTRY_COUNT; i++)
    {
        if (FAT_readFile(dir, sizeof(FAT_DirEntry), entryOut) != sizeof(FAT_DirEntry))
        {
            printf("Failed to read dir entry\n\r");
            return false;
        }
        char c = entryOut->name[10];
        entryOut->name[10] = 0;
        printf("Name: %s%c\n\r", entryOut->name, c);
        entryOut->name[10] = c;
        if (memcmp(entryOut->name, name, 11))
            return true;
    }
    return false;
}

FAT_File far *FAT_openFile(const char *path)
{
    printf("Opening: %s\n\r", path);

    // remove leading slash
    if (*path == '/')
        path++;

    char name[MAX_NAME_LENGTH + 1];
    uint16_t i = 0;
    FAT_File far *currentDir = &(g_Data->rootDir->file);
    FAT_DirEntry entry;

    name[MAX_NAME_LENGTH] = 0;

    // traverse path/directory tree
    while (*path)
    {
        switch (*path)
        {
        case '/':
            for (; i < MAX_NAME_LENGTH; i++)
                name[i] = ' ';
            if (FAT_findFile(currentDir, name, &entry))
            {
                if (!FAT_closeFile(currentDir))
                {
                    printf("Failed to close dir\n\r");
                    return NULL;
                }
                if (!(entry.attributes & ENTRY_ATTR_DIR))
                {
                    printf("wrong attr: %x\n\r", entry.attributes);
                    return NULL;
                }
                currentDir = FAT_openEntry(&entry);
            }
            else
            {
                printf("No dir named '%s'\n\r", name);
                return NULL;
            }
            i = 0;
            break;
        case '.':
            if (i >= 8)
            {
                printf("Name too long\n\r");
                return NULL;
            }
            for (; i < 8; i++)
                name[i] = ' ';

            break;
        default:
            if (i >= MAX_NAME_LENGTH)
            {
                printf("Name too long\n\r");
                return NULL;
            }
            name[i++] = UPPER(*path);
            break;
        }
        path++;
    }

    // open file
    for (; i < MAX_NAME_LENGTH; i++)
        name[i] = ' ';
    if (FAT_findFile(currentDir, name, &entry))
    {
        if (!FAT_closeFile(currentDir))
        {
            printf("Failed to close dir\n\r");
            return NULL;
        }
        if (entry.attributes & ENTRY_ATTR_DIR)
        {
            printf("'%s' not a file\n\r", name);
            return NULL;
        }
        printf("Opening file '%s'\n\r", name);
        return FAT_openEntry(&entry);
    }
    else
    {
        printf("File '%s' not found\n\r", name);
        return NULL;
    }
}

uint32_t FAT_nextCluster(uint32_t cluster)
{
    uint32_t fatIndex = cluster * 3 / 2;

    if (cluster % 2 == 0)
        return (*(uint16_t far *)(g_Fat + fatIndex)) & 0x0FFF;
    else
        return (*(uint16_t far *)(g_Fat + fatIndex)) >> 4;
}

uint32_t FAT_readFile(FAT_File far *file, uint32_t bytes, void *dataOut)
{
    uint32_t bytesRead = 0;
    bytes = MIN(bytes, file->size - file->index);
    FAT_FileData far *fileData = &(g_Data->openFiles[file->handle]);
    for (int i = 0; i < bytes; i++)
    {
        printf("%c", fileData->buffer[file->index + i]);
    }
    printf("|| \n\r");
    while (bytesRead < bytes)
    {
        uint32_t bytesToRead = MIN(SECTOR_SIZE - (file->index % SECTOR_SIZE), bytes - bytesRead);
        memcpy((uint8_t *)dataOut + bytesRead, fileData->buffer + file->index % SECTOR_SIZE, bytesToRead);
        file->index += bytesToRead;
        bytesRead += bytesToRead;
        if (file->index % SECTOR_SIZE == 0)
        {
            if (file->handle == ROOT_DIR_HANDLE)
            {
                if (!DISK_readSectors(g_Data->disk, ++fileData->currentCluster, 1, fileData->buffer))
                {
                    printf("Error: failed to read sector\n\r");
                }
            }
            else
            {
                if (++fileData->currentSector >= g_Data->BS.bootSector.sectorsPerCluster)
                {
                    fileData->currentSector = 0;
                    fileData->currentCluster = FAT_nextCluster(fileData->currentCluster);
                }
                if (!DISK_readSectors(g_Data->disk, FAT_clusterToSector(fileData->currentCluster) + fileData->currentSector, 1, fileData->buffer))
                {
                    printf("Error: failed to read sector\n\r");
                }
            }
        }
    }
    return bytesRead;
}