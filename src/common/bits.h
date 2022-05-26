#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t

typedef struct BitStream_t {
    uint8_t data[128];
    uint16_t data_cursor; // NOTE: In bits
} BitStream_t;

void BitStream_init(struct BitStream_t* self);
void BitStream_storeBit(struct BitStream_t* self, int8_t v);