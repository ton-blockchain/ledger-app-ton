#pragma once

#include <stdbool.h>  // bool

#include "types.h"

typedef enum {
    TRANSACTION_COMMENT = 0,
    TRANSACTION_TRANSFER_JETTON = 1,
    TRANSACTION_TRANSFER_NFT = 2,
    TRANSACTION_BURN_JETTON = 3,
    TRANSACTION_ADD_WHITELIST = 4,
} transaction_hint_type_e;

bool process_hints(transaction_t* tx);
