#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include "types.h"

bool process_hints(transaction_t* tx);

void print_hint(transaction_t* tx,
                uint16_t index,
                char* title,
                size_t title_len,
                char* body,
                size_t body_len);