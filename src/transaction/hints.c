#include "hints.h"
#include "utils.h"
#include <string.h>
#include "os.h"
#include "ux.h"
#include "../common/buffer.h"
#include "../common/base64.h"
#include "../common/format.h"
#include "../common/format_bigint.h"
#include "../constants.h"
#include "../address.h"
#include "deserialize.h"

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

static void add_hint_text(transaction_t* tx, const char* title, char* text, size_t text_len) {
    // Configure
    tx->hints[tx->hints_count].title = title;
    tx->hints[tx->hints_count].kind = SummaryItemString;
    tx->hints[tx->hints_count].string.string = text;
    tx->hints[tx->hints_count].string.length = text_len;

    // Next
    tx->hints_count++;
}

static void add_hint_hash(transaction_t* tx, const char* title, uint8_t* data) {
    // Configure
    tx->hints[tx->hints_count].title = title;
    tx->hints[tx->hints_count].kind = SummaryHash;
    memmove(tx->hints[tx->hints_count].hash, data, 32);

    // Next
    tx->hints_count++;
}

static void add_hint_amount(transaction_t* tx, const char* title, const char* ticker, uint8_t* value, uint8_t value_len, uint8_t decimals) {
    // Configure
    tx->hints[tx->hints_count].title = title;
    tx->hints[tx->hints_count].kind = SummaryItemAmount;
    size_t ticker_len = strnlen(ticker, MAX_TICKER_LEN);
    memmove(tx->hints[tx->hints_count].amount.ticker, ticker, ticker_len);
    tx->hints[tx->hints_count].amount.ticker[ticker_len] = '\0';
    memmove(tx->hints[tx->hints_count].amount.value, value, value_len);
    tx->hints[tx->hints_count].amount.value_len = value_len;
    tx->hints[tx->hints_count].amount.decimals = decimals;

    // Next
    tx->hints_count++;
}

static void add_hint_address(transaction_t* tx, const char* title, address_t address) {
    // Configure
    tx->hints[tx->hints_count].title = title;
    tx->hints[tx->hints_count].kind = SummaryAddress;
    tx->hints[tx->hints_count].address = address;

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
        snprintf(tx->title, sizeof(tx->title), "Transfer");
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

    if (tx->hints_type == TRANSACTION_COMMENT) {
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
        snprintf(tx->title, sizeof(tx->title), "Transfer");

        // Add code hints
        add_hint_text(tx, "Comment", (char*) tx->hints_data, tx->hints_len);
    }

    if (tx->hints_type == TRANSACTION_TRANSFER_JETTON || tx->hints_type == TRANSACTION_TRANSFER_NFT) {
        int ref_count = 0;
        CellRef_t refs[2] = { 0 };

        BitString_init(&bits);
        BitString_storeUint(&bits, tx->hints_type == TRANSACTION_TRANSFER_JETTON ? 0x0f8a7ea5 : 0x5fcc3d14, 32);

        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            uint64_t query_id;
            SAFE(buffer_read_u64(&buf, &query_id, BE));
            BitString_storeUint(&bits, query_id, 64);
        } else {
            BitString_storeUint(&bits, 0, 64);
        }

        if (tx->hints_type == TRANSACTION_TRANSFER_JETTON) {
            uint8_t amount_size;
            uint8_t amount_buf[MAX_VALUE_BYTES_LEN];
            SAFE(buffer_read_varuint(&buf, &amount_size, amount_buf, MAX_VALUE_BYTES_LEN));
            BitString_storeCoinsBuf(&bits, amount_buf, amount_size);
            uint8_t amount_decimals;
            SAFE(buffer_read_u8(&buf, &amount_decimals));
            uint8_t ticker_size;
            SAFE(buffer_read_u8(&buf, &ticker_size));
            if (ticker_size > MAX_TICKER_LEN) {
                return false;
            }
            uint8_t ticker[MAX_TICKER_LEN+1];
            SAFE(buffer_read_buffer(&buf, ticker, ticker_size));
            ticker[ticker_size] = 0;
            if (!transaction_utils_check_encoding(ticker, ticker_size)) {
                return false;
            }

            add_hint_amount(tx, "Jetton amount", (char*) ticker, amount_buf, amount_size, amount_decimals);
        }

        address_t destination;
        SAFE(buffer_read_address(&buf, &destination));
        BitString_storeAddress(&bits, destination.chain, destination.hash);

        add_hint_address(tx, tx->hints_type == TRANSACTION_TRANSFER_JETTON ? "Send jetton to" : "New owner", destination);

        address_t response;
        SAFE(buffer_read_address(&buf, &response));
        BitString_storeAddress(&bits, response.chain, response.hash);

        add_hint_address(tx, "Send excess to", response);

        // custom payload
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            SAFE(buffer_read_cell_ref(&buf, &refs[ref_count]));

            add_hint_hash(tx, "Custom payload", refs[ref_count].hash);

            BitString_storeBit(&bits, 1);
            ref_count++;
        } else {
            BitString_storeBit(&bits, 0);
        }

        uint8_t fwd_amount_size;
        uint8_t fwd_amount_buf[MAX_VALUE_BYTES_LEN];
        SAFE(buffer_read_varuint(&buf, &fwd_amount_size, fwd_amount_buf, MAX_VALUE_BYTES_LEN));
        BitString_storeCoinsBuf(&bits, fwd_amount_buf, fwd_amount_size);

        add_hint_amount(tx, "Forward amount", "TON", fwd_amount_buf, fwd_amount_size, EXPONENT_SMALLEST_UNIT);

        // forward payload
        SAFE(buffer_read_bool(&buf, &tmp));
        if (tmp) {
            SAFE(buffer_read_cell_ref(&buf, &refs[ref_count]));

            add_hint_hash(tx, "Forward payload", refs[ref_count].hash);

            BitString_storeBit(&bits, 1);
            ref_count++;
        } else {
            BitString_storeBit(&bits, 0);
        }

        CHECK_END();

        // Build cell
        hash_Cell(&bits, refs, ref_count, &cell);
        hasCell = true;

        // Operation
        snprintf(tx->title, sizeof(tx->title), tx->hints_type == TRANSACTION_TRANSFER_JETTON ? "Transfer jetton" : "Transfer NFT");
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
        amountToString(hint.amount.value, hint.amount.value_len, hint.amount.decimals, hint.amount.ticker, body, body_len);
    } else if (hint.kind == SummaryAddress) {
        uint8_t address[ADDRESS_LEN] = {0};
        address_to_friendly(hint.address.chain,
                            hint.address.hash,
                            true,
                            false,
                            address,
                            sizeof(address));
        memset(body, 0, body_len);
        base64_encode(address, sizeof(address), body, body_len);
    } else {
        print_string("<unknown>", body, body_len);
    }
}