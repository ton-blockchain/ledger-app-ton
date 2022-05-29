#pragma once

#include <stdint.h>   // uint*_t
#include <stddef.h>   // size_t
#include <stdbool.h>  // bool

#define INT256_LENGTH 32

typedef struct Uint256_t {
    uint8_t value[INT256_LENGTH];
    uint8_t length;
} Uint256_t;

bool uint256_to_decimal(const uint8_t *value, size_t value_len, char *out, size_t out_len);

static __attribute__((no_instrument_function)) inline int allzeroes(void *buf, size_t n) {
    uint8_t *p = (uint8_t *) buf;
    for (size_t i = 0; i < n; ++i) {
        if (p[i]) {
            return 0;
        }
    }
    return 1;
}