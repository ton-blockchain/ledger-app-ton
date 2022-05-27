#include "hash.h"
#include "cx.h"
#include "../common/bits.h"
#include <string.h>  // memmove

struct CellRef_t {
    uint16_t max_depth;
    uint8_t hash[32];
} CellRef_t;

void hash_BitString(struct BitString_t *self, uint8_t *out) {
    cx_sha256_t state;
    cx_sha256_init(&state);
    cx_hash((cx_hash_t *) &state, CX_LAST, self->data, self->data_cursor / 8, out, 32);
}

void hash_Cell(struct BitString_t *bits,
               struct CellRef_t *refs,
               uint8_t refs_count,
               struct CellRef_t *out) {
    cx_sha256_t state;
    cx_sha256_init(&state);

    // Data and descriptors
    uint16_t len = bits->data_cursor;
    uint8_t d1 = refs_count;                     // refs descriptor
    uint8_t d2 = (len >> 3) + ((len + 7) >> 3);  // bits descriptor
    uint8_t d[2] = {d1, d2};
    BitString_finalize(bits);

    // Hash data and descriptors
    cx_hash((cx_hash_t *) &state, 0, d, 2, NULL, 0);
    cx_hash((cx_hash_t *) &state, 0, bits->data, bits->data_cursor / 8, NULL, 0);

    // Hash ref depths
    for (int i = 0; i < refs_count; i++) {
        struct CellRef_t md = refs[i];
        uint8_t mdd[2] = {md.max_depth / 256, md.max_depth % 256};
        cx_hash((cx_hash_t *) &state, 0, mdd, 2, NULL, 0);
    }

    // Hash ref depths
    for (int i = 0; i < refs_count; i++) {
        struct CellRef_t md = refs[i];
        cx_hash((cx_hash_t *) &state, 0, md.hash, 32, NULL, 0);
    }

    // Finalize
    cx_hash((cx_hash_t *) &state, CX_LAST, NULL, 0, out->hash, 32);

    // Depth
    out->max_depth = 0;
    if (refs_count > 0) {
        for (int i = 0; i < refs_count; i++) {
            struct CellRef_t md = refs[i];
            if (md.max_depth > out->max_depth) {
                out->max_depth = md.max_depth;
            }
        }
        out->max_depth = out->max_depth + 1;
    }
}

bool hash_tx(transaction_ctx_t *ctx) {
    BitString_t bits;
    struct CellRef_t payload_ref;

    //
    // Internal Message
    //

    struct CellRef_t internalMessageRef;
    BitString_init(&bits);
    BitString_storeBit(&bits, 0);                        // tag
    BitString_storeBit(&bits, 1);                        // ihr_disabled
    BitString_storeBit(&bits, ctx->transaction.bounce);  // bounce
    BitString_storeBit(&bits, 0);                        // bounced
    BitString_storeAddressNull(&bits);                   // from
    BitString_storeAddress(&bits, ctx->transaction.to_chain, ctx->transaction.to_hash);  // to
    BitString_storeCoins(&bits, ctx->transaction.value);                                 // amount
    BitString_storeBit(&bits, 0);       // Currency collection (not supported)
    BitString_storeCoins(&bits, 0);     // ihr_fees
    BitString_storeCoins(&bits, 0);     // fwd_fees
    BitString_storeUint(&bits, 0, 64);  // CreatedLT
    BitString_storeUint(&bits, 0, 32);  // CreatedAt

    // Payload
    if (ctx->transaction.payload > 0) {
        BitString_storeBit(&bits, 0);  // no state-init
        BitString_storeBit(&bits, 1);  // body in ref

        // Create ref
        payload_ref.max_depth = ctx->transaction.payload_depth;
        memmove(payload_ref.hash, ctx->transaction.payload_hash, 32);

        // Hash cell
        struct CellRef_t internalMessageRefs[1] = {payload_ref};
        hash_Cell(&bits, internalMessageRefs, 1, &internalMessageRef);
    } else {
        BitString_storeBit(&bits, 0);  // no state-init
        BitString_storeBit(&bits, 0);  // body inline

        // Hash cell
        hash_Cell(&bits, NULL, 0, &internalMessageRef);
    }

    //
    // Order
    //

    struct CellRef_t orderRef;
    BitString_init(&bits);
    BitString_storeUint(&bits, 698983191, 32);                  // Wallet ID
    BitString_storeUint(&bits, ctx->transaction.timeout, 32);   // Timeout
    BitString_storeUint(&bits, ctx->transaction.seqno, 32);     // Seqno
    BitString_storeUint(&bits, 0, 8);                           // Simple order
    BitString_storeUint(&bits, ctx->transaction.send_mode, 8);  // Send Mode
    struct CellRef_t orderRefs[1] = {internalMessageRef};
    hash_Cell(&bits, orderRefs, 1, &orderRef);

    // Result
    memmove(ctx->m_hash, orderRef.hash, 32);

    return true;
}