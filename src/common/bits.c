#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "bits.h"

void BitStream_init(struct BitStream_t* self) {
    self->data_cursor = 0;
}

void BitStream_storeBit(struct BitStream_t* self, int8_t v) {
    if (v > 0) {
        // this.#buffer[(n / 8) | 0] |= 1 << (7 - (n % 8));
        self->data[(self->data_cursor / 8) | 0] |= (1 << (7 - (self->data_cursor % 8)));
    } else {
        // this.#buffer[(n / 8) | 0] &= ~(1 << (7 - (n % 8)));
        self->data[(self->data_cursor / 8) | 0] &= ~(1 << (7 - (self->data_cursor % 8)));
    }
    self->data_cursor++;
}