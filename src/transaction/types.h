#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include "cell.h"     // CellRef_t
#include "../constants.h"

#define MAX_TX_LEN             510
#define ADDRESS_LEN            36
#define MAX_MEMO_LEN           120
#define MAX_STORED_STRING_SIZE 64

typedef enum {
    PARSING_OK = 0,
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
    HINTS_PARSING_ERROR = -11,
} parser_status_e;

typedef struct {
    uint8_t chain;
    uint8_t hash[HASH_LEN];
} address_t;

enum HintKind {
    SummaryItemNone = 0,  // SummaryItemNone always zero
    SummaryItemAmount,
    SummaryItemString,
    SummaryAddress,
    SummaryHash
};

typedef struct {
    char* string;
    size_t length;
} SizedString_t;

typedef struct {
    char ticker[MAX_TICKER_LEN + 1];
    uint8_t value[MAX_VALUE_BYTES_LEN];
    uint8_t value_len;
    uint8_t decimals;
} Amount_t;

typedef struct {
    const char* title;
    enum HintKind kind;
    union {
        Amount_t amount;
        uint64_t u64;
        SizedString_t string;
        uint8_t hash[HASH_LEN];
        address_t address;
    };
} Hint_t;

typedef struct {
    uint8_t tag;                             // tag (1 byte)
    uint32_t seqno;                          // seqno (4 bytes)
    uint32_t timeout;                        // timeout (4 bytes)
    uint8_t value_buf[MAX_VALUE_BYTES_LEN];  // big endian transaction value
    uint8_t value_len;                       // length of transaction value
    bool bounce;                             // bounce
    uint8_t send_mode;                       // send_mode (1 byte)
    address_t to;                            // receiver
    bool has_state_init;                     // true if state_init exists
    CellRef_t state_init;                    // state_init if exists
    bool has_payload;                        // true if payload exists
    CellRef_t payload;                       // payload if exists
    bool has_hints;                          // true if payload exists
    uint32_t hints_type;                     // hints type if exists
    uint16_t hints_len;                      // hints len if exists
    uint8_t* hints_data;                     // hints data if exists
    bool is_blind;                           // eoes transaction require blind signing
    Hint_t hints[MAX_HINTS];
    uint8_t hints_count;
    char title[128];
} transaction_t;
