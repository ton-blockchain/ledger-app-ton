#include "hints.h"
#include "utils.h"
#include <string.h>
#include "os.h"
#include "ux.h"
#include "../common/buffer.h"
#include "../common/base64.h"
#include "../common/format.h"
#include "../constants.h"
#include "deserialize.h"

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

static void add_hint_text(transaction_t* tx, char* title, char* text, size_t text_len) {
    // Configure
    tx->hints[tx->hints_count].title = title;
    tx->hints[tx->hints_count].kind = SummaryItemString;
    tx->hints[tx->hints_count].string.string = text;
    tx->hints[tx->hints_count].string.length = text_len;

    // Next
    tx->hints_count++;
}

static void add_hint_hash(transaction_t* tx, char* title, uint8_t* data) {
    // Configure
    tx->hints[tx->hints_count].title = title;
    tx->hints[tx->hints_count].kind = SummaryHash;
    memmove(tx->hints[tx->hints_count].hash, data, 32);

    // Next
    tx->hints_count++;
}

static void add_hint_amount(transaction_t* tx, char* title, uint64_t amount) {
    // Configure
    tx->hints[tx->hints_count].title = title;
    tx->hints[tx->hints_count].kind = SummaryItemAmount;
    tx->hints[tx->hints_count].u64 = amount;

    // Next
    tx->hints_count++;
}

#define SAFE(RES)     \
    if (!RES) {       \
        return false; \
    }

#define CHECK_END()               \
    if (buf.offset != buf.size) { \
        return false;             \
    }

bool process_hints(transaction_t* tx) {
    // Default title
    snprintf(tx->title, sizeof(tx->title), "Transaction");

    // No payload
    if (!tx->has_payload) {
        tx->is_blind = false;
        return true;
    }
    // No hints
    if (!tx->has_hints) {
        tx->is_blind = true;
        return true;
    }

    // Default state
    tx->is_blind = true;
    CellRef_t cell;
    BitString_t bits;
    bool hasCell = false;
    bool tmp = false;
    tx->hints_count = 0;
    buffer_t buf = {.ptr = tx->hints_data, .size = tx->hints_len, .offset = 0};

    //
    // Comment
    //

    if (tx->hints_type == 0x0) {
        // Max size of a comment is 120 symbols
        if (tx->hints_len > 120) {
            return false;
        }

        // Check ASCII
        if (!transaction_utils_check_encoding(tx->hints_data, tx->hints_len)) {
            return true;
        }

        // Buld cell
        BitString_init(&bits);
        BitString_storeUint(&bits, 0, 32);
        BitString_storeBuffer(&bits, tx->hints_data, tx->hints_len);
        hash_Cell(&bits, NULL, 0, &cell);
        hasCell = true;

        // Change title of operation
        add_hint_text(tx, "Comment", (char*) tx->hints_data, tx->hints_len);
    }

    //
    // Upgrade
    //

    if (tx->hints_type == 0x01) {
        // Building cell
        BitString_init(&bits);
        BitString_storeUint(&bits, 0xdbfaf817, 32);

        // query_id
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        }

        // gas_limit
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t gas_limit;
            SAFE(buffer_read_u64(&buf, &gas_limit, BE));
            BitString_storeCoins(&bits, gas_limit);
        }

        // Code
        CellRef_t code;
        SAFE(buffer_read_cell_ref(&buf, &code));
        CHECK_END();

        // Complete
        struct CellRef_t refs[1] = {code};
        hash_Cell(&bits, refs, 1, &cell);
        hasCell = true;

        // Change title of operation
        snprintf(tx->title, sizeof(tx->title), "Upgrade Code");

        // Add code hints
        add_hint_hash(tx, "Code", code.hash);
    }

    //
    // Deposit
    //

    if (tx->hints_type == 0x02) {
        // Building cell
        BitString_init(&bits);
        BitString_storeUint(&bits, 0x7bcd1fef, 32);

        // query_id
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        }

        // gas_limit
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t gas_limit;
            SAFE(buffer_read_u64(&buf, &gas_limit, BE));
            BitString_storeCoins(&bits, gas_limit);
        }

        CHECK_END();

        // Complete
        hash_Cell(&bits, NULL, 0, &cell);
        hasCell = true;

        // Change title of operation
        snprintf(tx->title, sizeof(tx->title), "Deposit");
    }

    //
    // Withdraw
    //

    if (tx->hints_type == 0x03) {
        // Building cell
        BitString_init(&bits);
        BitString_storeUint(&bits, 0xda803efd, 32);

        // query_id
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        }

        // gas_limit
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t gas_limit;
            SAFE(buffer_read_u64(&buf, &gas_limit, BE));
            BitString_storeCoins(&bits, gas_limit);
        }

        // Amount
        uint64_t amount;
        SAFE(buffer_read_u64(&buf, &amount, BE));
        BitString_storeCoins(&bits, amount);
        CHECK_END();

        // Complete
        hash_Cell(&bits, NULL, 0, &cell);
        hasCell = true;

        // Change title of operation
        snprintf(tx->title, sizeof(tx->title), "Withdraw");

        // Add amount hint
        add_hint_amount(tx, "Withdraw", amount);
    }

    // Check hash
    if (hasCell) {
        if (memcmp(cell.hash, tx->payload.hash, 32) != 0) {
            return false;
        }
        tx->is_blind = false;
    }

    return true;
}

int print_string(const char* in, char* out, size_t out_length) {
    strncpy(out, in, out_length);
    int rc = (out[--out_length] != '\0');
    if (rc) {
        /* ensure the output is NUL terminated */
        out[out_length] = '\0';
        if (out_length != 0) {
            /* signal truncation */
            out[out_length - 1] = '~';
        }
    }
    return rc;
}

int print_sized_string(const SizedString_t* string, char* out, size_t out_length) {
    size_t len = MIN(out_length, string->length);
    strncpy(out, string->string, len);
    if (string->length < out_length) {
        out[string->length] = '\0';
        return 0;
    } else {
        out[--out_length] = '\0';
        if (out_length != 0) {
            /* signal truncation */
            out[out_length - 1] = '~';
        }
        return 1;
    }
}

void print_hint(transaction_t* tx,
                uint16_t index,
                char* title,
                size_t title_len,
                char* body,
                size_t body_len) {
    Hint_t hint = tx->hints[index];

    // Title
    print_string(hint.title, title, title_len);

    // Body
    if (hint.kind == SummaryItemString) {
        print_sized_string(&hint.string, body, body_len);
    } else if (hint.kind == SummaryHash) {
        base64_encode(hint.hash, 32, body, body_len);
    } else if (hint.kind == SummaryItemAmount) {
        char amount[30] = {0};
        format_fpu64(amount, sizeof(amount), hint.u64, EXPONENT_SMALLEST_UNIT);
        snprintf(body, body_len, "TON %.*s", sizeof(amount), amount);
    } else {
        print_string("<unknown>", body, body_len);
    }
}