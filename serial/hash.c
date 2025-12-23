#ifndef __HASH_C__
#define __HASH_C__

#include <stdint.h>

uint32_t FNV(const char* key)
{
    uint32_t h = 2166136261UL;
    const uint8_t* data = (const uint8_t*)key;
    for(int i = 0; data[i] != '\0'; i++)
    {
        h ^= data[i];
        h *= 16777619;
    }
    return h;
}
#endif
