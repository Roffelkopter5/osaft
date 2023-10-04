#pragma once
#include "type.h"
#include "disk.h"

#pragma pack(push, 1)
typedef struct FAT_DirEntry
{
    uint8_t name[11];
    uint8_t attributes;
    uint8_t _reserved;
    uint8_t createdTimeTenths;
    uint16_t createdTime;
    uint16_t createdDate;
    uint16_t accessedDate;
    uint16_t firstClusterHigh;
    uint16_t modifiedTime;
    uint16_t modifiedDate;
    uint16_t firstClusterLow;
    uint32_t size;
} FAT_DirEntry;
#pragma pack(pop)

typedef struct FAT_File
{
    int handle;
    bool isDirectory;
    uint32_t index;
    uint32_t size;
} FAT_File;

bool FAT_initialize(DISK *disk);
FAT_File far *FAT_openFile(const char *path);
uint32_t FAT_readFile(FAT_File far *file, uint32_t bytes, void *dataOut);
bool FAT_closeFile(FAT_File far *file);
