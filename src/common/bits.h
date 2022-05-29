#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t
#include "int256.h"

typedef struct BitString_t {
    uint8_t data[128];
    uint16_t data_cursor;  // NOTE: In bits
} BitString_t;

void BitString_init(BitString_t* self);
void BitString_storeBit(BitString_t* self, int8_t v);
void BitString_storeUint(BitString_t* self, uint64_t v, uint8_t bits);
void BitString_storeCoins(BitString_t* self, uint64_t v);
void BitString_storeBuffer(BitString_t* self, uint8_t* v, uint8_t length);
void BitString_storeAddress(BitString_t* self, uint8_t chain, uint8_t* hash);
void BitString_storeAddressNull(BitString_t* self);
void BitString_finalize(BitString_t* self);