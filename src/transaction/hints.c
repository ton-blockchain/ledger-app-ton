#include "hints.h"
#include "utils.h"
#include <string.h>
#include "os.h"
#include "ux.h"

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

#define SAFE(RES)     \
    if (!RES) {       \
        return false; \
    }

#define CHECK_END()                 \
    if (buf->offset != buf->size) { \
        return false;               \
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
    tx->hints_count = 0;
    buffer_t buf = {.ptr = tx->hints_data, .size = tx->hints_len, .offset = 0};

    // Comment
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

    // Empty transactions
    if (tx->hints_type == 0x47bbe425 || tx->hints_type == 0x7bcd1fef) {
        if (tx->hints_len != 0) {
            return false;
        }

        // Build cell
        BitString_init(&bits);
        BitString_storeUint(&bits, tx->hints_type, 32);
        hash_Cell(&bits, NULL, 0, &cell);
        hasCell = true;

        // Change title of operation
        if (tx->hints_type == 0x47bbe425) {
            add_hint_text(tx, "Donate", (char*) tx->hints_data, tx->hints_len);
        }
        if (tx->hints_type == 0x7bcd1fef) {
            add_hint_text(tx, "Deposit Stake", (char*) tx->hints_data, tx->hints_len);
        }
    }

    // Upgrade
    if (tx->hint_type == 0xdbfaf817) {
        // Parsing
        CellRef_t code;
        SAFE(buffer_read_cell_ref(buf, &code));
        CHECK_END();

        // Build cell
        struct CellRef_t codeRefs[1] = {code};
        BitString_init(&bits);
        BitString_storeUint(&bits, 0xdbfaf817, 32);
        hash_Cell(&bits, codeRefs, 1, &cell);
        hasCell = true;

        // Change title of operation
        add_hint_text(tx, "Upgrade Code", (char*) tx->hints_data, tx->hints_len);
    }

    // Update
    if (tx->hint_type == 0x023cd52c) {
        // Parsing
        CellRef_t data;
        SAFE(buffer_read_cell_ref(buf, &data));
        CHECK_END();

        // Build cell
        struct CellRef_t dataRefs[1] = {data};
        BitString_init(&bits);
        BitString_storeUint(&bits, 0x023cd52c, 32);
        hash_Cell(&bits, dataRefs, 1, &cell);
        hasCell = true;

        // Change title of operation
        add_hint_text(tx, "Update Contract", (char*) tx->hints_data, tx->hints_len);
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
    } else {
        print_string("<unknown>", body, body_len);
    }
}