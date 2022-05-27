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
    TAG_PARSING_ERROR = -6,
    SEND_MODE_PARSING_ERROR = -7,
    BOUNCE_PARSING_ERROR = -8,
    PAYLOAD_PARSING_ERROR = -9,
    STATE_INIT_PARSING_ERROR = -10,
} parser_status_e;

typedef struct {
    uint8_t tag;                /// tag (1 byte)
    uint32_t seqno;             /// seqno (4 bytes)
    uint32_t timeout;           /// timeout (4 bytes)
    uint64_t value;             /// amount value (8 bytes)
    uint8_t bounce;             /// bounce (1 byte)
    uint8_t send_mode;          /// send_mode (1 byte)
    uint8_t to_chain;           /// target chain (1 byte)
    uint8_t *to_hash;           /// target address (32 bytes)
    uint8_t payload;            // if payload exist (1 byte)
    uint16_t payload_depth;     // payload depth (2 bytes)
    uint8_t *payload_hash;      /// payload hash (32 bytes)
    uint8_t state_init;         // if state_init exist (1 byte)
    uint16_t state_init_depth;  // state_init depth (2 bytes)
    uint8_t *state_init_hash;   /// state_init hash (32 bytes)
} transaction_t;
