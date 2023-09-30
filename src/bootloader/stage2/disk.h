#pragma once

#include "stdint.h"

typedef struct DISK
{
    uint8_t id;
    uint16_t cylinders;
    uint16_t heads;
    uint16_t sectors;
} DISK;

bool DISK_Initialize(DISK *disk, uint8_t driveNumber);
bool DISK_Read_Sectors(DISK *disk, uint32_t lba, uint8_t sectors, uint8_t far *dataOut);
