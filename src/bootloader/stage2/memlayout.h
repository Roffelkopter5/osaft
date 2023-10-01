#pragma once

#define MEMORY_MIN 0x00000500
#define MEMORY_MAX 0x00080000

#define MEMORY_FAT_ADDR ((void far *)0x05000000) // segment:offset (SSSS_OOOO)
#define MEMORY_FAT_SIZE 0x00010000
