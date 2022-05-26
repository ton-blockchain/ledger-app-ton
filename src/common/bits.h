#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t

typedef struct BitStream_t {
    uint8_t data[128];
    uint16_t data_cursor; // NOTE: In bits
} BitStream_t;

void BitStream_init(struct BitStream_t* self);
void BitStream_storeBit(struct BitStream_t* self, int8_t v);
void BitStream_storeUint(struct BitStream_t* self, uint64_t v, uint8_t bits);
void BitStream_storeCoins(struct BitStream_t* self, uint64_t v);
void BitStream_storeBuffer(struct BitStream_t* self, uint8_t *v, uint8_t length);
void BitStream_finalize(struct BitStream_t* self);