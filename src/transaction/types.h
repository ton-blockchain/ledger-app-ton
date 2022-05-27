#pragma once

#include <stddef.h>  // size_t
#include <stdint.h>  // uint*_t

#define MAX_TX_LEN   510
#define ADDRESS_LEN  36
#define MAX_MEMO_LEN 465  // 510 - ADDRESS_LEN - 2*SIZE(U64) - SIZE(MAX_VARINT)

typedef enum {
    PARSING_OK = 1,
    SEQ_PARSING_ERROR = -1,
    TIMEOUT_PARSING_ERROR = -2,
    TO_PARSING_ERROR = -3,
    VALUE_PARSING_ERROR = -4,
    WRONG_LENGTH_ERROR = -5,
} parser_status_e;

typedef struct {
    uint32_t seqno;    /// seqno (4 bytes)
    uint32_t timeout;  /// timeout (4 bytes)
    uint64_t value;    /// amount value (8 bytes)
    uint8_t to_chain;  /// target chain (1 byte)
    uint8_t *to_hash;  /// target address (32 bytes)
    // uint8_t *payload;   /// payload hash (32 bytes)
} transaction_t;
