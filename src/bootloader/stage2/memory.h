#pragma once
#include "type.h"

void far *memset(void far *ptr, uint8_t v, uint16_t n);
void far *memcpy(void far *dest, const void far *src, uint16_t n);
bool memcmp(const void far *ptrA, const void far *ptrB, uint16_t n);