#pragma once

#include <stddef.h>   // size_t
#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include "cell.h"     // CellRef_t

#define MAX_TX_LEN   510
#define ADDRESS_LEN  36
#define MAX_MEMO_LEN 465  // 510 - ADDRESS_LEN - 2*SIZE(U64) - SIZE(MAX_VARINT)

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
    uint8_t hash[32];
} address_t;

enum HintKind {
    SummaryItemNone = 0,  // SummaryItemNone always zero
    SummaryItemAmount,
    SummaryItemU64,
    SummaryItemString,
    SummaryHash
};

typedef struct {
    char* string;
    size_t length;
} SizedString_t;

typedef struct {
    const char* title;
    enum HintKind kind;
    union {
        uint64_t amount;
        uint64_t u64;
        SizedString_t string;
        uint8_t hash[32];
    };
} Hint_t;

typedef struct {
    uint8_t tag;           // tag (1 byte)
    uint32_t seqno;        // seqno (4 bytes)
    uint32_t timeout;      // timeout (4 bytes)
    uint64_t value;        // amount value (8 bytes)
    uint8_t bounce;        // bounce (1 byte)
    uint8_t send_mode;     // send_mode (1 byte)
    address_t to;          // target
    bool has_state_init;   // true if state_init exist
    CellRef_t state_init;  // state_init if exist
    bool has_payload;      // true if payload exist
    CellRef_t payload;     // payload if exist
    bool has_hints;        // true if payload exist
    uint32_t hints_type;   // hints type if exist
    uint16_t hints_len;    // hints len if exist
    uint8_t* hints_data;   // hints data if exist
    bool is_blind;         // Is transaction requires blind signing
    Hint_t hints[16];
    uint8_t hints_count;
    char title[128];
} transaction_t;
