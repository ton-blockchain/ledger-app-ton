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

void BitStream_storeUint(struct BitStream_t* self, uint64_t v, uint8_t bits) {
    for(int i = 0; i < bits; i++) {
        int8_t b = (v >> (bits - i - 1)) & 0x01;
        BitStream_storeBit(self, b);
    }
}

void BitStream_storeCoins(struct BitStream_t* self, uint64_t v) {

    // Measure length
    uint8_t len = 0;
    uint64_t r = v;
    for(int i = 0; i < 8; i++) {
        if (r > 0) {
            len++;
            r = r >> 8;
        } else {
            break;
        }
    }

    // Write length
    BitStream_storeUint(self, len, 4);

    // Write remaining
    r = v;
    for(int i = 0; i < len; i++) {
        BitStream_storeUint(self, v >> ((len - i - 1) * 8), 8);
    }
}

void BitStream_storeBuffer(struct BitStream_t* self, uint8_t *v, uint8_t length) {
    for(int i = 0; i < length; i++) {
        BitStream_storeUint(self, v[i], 8);
    }
}

void BitStream_finalize(struct BitStream_t* self) {
    uint8_t padBytes = self->data_cursor % 8;
    if (padBytes > 0) {
        padBytes = padBytes - 1;
        BitStream_storeBit(self, 1);
        while(padBytes > 0) {
            padBytes = padBytes - 1;
            BitStream_storeBit(self, 0);
        }
    }
}