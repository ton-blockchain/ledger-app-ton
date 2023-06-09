#pragma once

#include <stdint.h>  // uint*_t
#include <stdbool.h>

#include "../common/bits.h"
#include "../constants.h"

typedef struct CellRef_t {
    uint16_t max_depth;
    uint8_t hash[HASH_LEN];
} CellRef_t;

bool hash_Cell(BitString_t *bits, CellRef_t *refs, uint8_t refs_count, CellRef_t *out);
