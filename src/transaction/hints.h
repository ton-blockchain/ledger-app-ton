#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include "types.h"

typedef enum {
    TRANSACTION_COMMENT = 0,
    TRANSACTION_TRANSFER_JETTON = 1,
    TRANSACTION_TRANSFER_NFT = 2,
} transaction_hint_type_e;

bool process_hints(transaction_t* tx);

void print_hint(transaction_t* tx,
                uint16_t index,
                char* title,
                size_t title_len,
                char* body,
                size_t body_len);