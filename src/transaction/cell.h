#pragma once

#include <stdint.h>  // uint*_t
#include "../common/bits.h"

typedef struct CellRef_t {
    uint16_t max_depth;
    uint8_t hash[32];
} CellRef_t;

void hash_BitString(BitString_t *self, uint8_t *out);
void hash_Cell(BitString_t *bits, CellRef_t *refs, uint8_t refs_count, CellRef_t *out);