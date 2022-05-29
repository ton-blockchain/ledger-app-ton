#include "hash.h"
#include "cx.h"
#include "cell.h"
#include "../common/bits.h"
#include <string.h>  // memmove

bool hash_tx(transaction_ctx_t *ctx) {
    BitString_t bits;
    struct CellRef_t payload_ref;
    struct CellRef_t state_init_ref;

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

    // Refs
    if (ctx->transaction.payload > 0 && ctx->transaction.state_init > 0) {
        BitString_storeBit(&bits, 1);  // state-init
        BitString_storeBit(&bits, 1);  // state-init ref
        BitString_storeBit(&bits, 1);  // body in ref

        // Create refs
        payload_ref.max_depth = ctx->transaction.payload_depth;
        memmove(payload_ref.hash, ctx->transaction.payload_hash, 32);
        state_init_ref.max_depth = ctx->transaction.state_init_depth;
        memmove(state_init_ref.hash, ctx->transaction.state_init_hash, 32);

        // Hash cell
        struct CellRef_t internalMessageRefs[2] = {state_init_ref, payload_ref};
        hash_Cell(&bits, internalMessageRefs, 2, &internalMessageRef);
    } else if (ctx->transaction.payload > 0) {
        BitString_storeBit(&bits, 0);  // no state-init
        BitString_storeBit(&bits, 1);  // body in ref

        // Create ref
        payload_ref.max_depth = ctx->transaction.payload_depth;
        memmove(payload_ref.hash, ctx->transaction.payload_hash, 32);

        // Hash cell
        struct CellRef_t internalMessageRefs[1] = {payload_ref};
        hash_Cell(&bits, internalMessageRefs, 1, &internalMessageRef);
    } else if (ctx->transaction.state_init > 0) {
        BitString_storeBit(&bits, 1);  // no state-init
        BitString_storeBit(&bits, 1);  // state-init ref
        BitString_storeBit(&bits, 0);  // body inline

        // Create ref
        state_init_ref.max_depth = ctx->transaction.state_init_depth;
        memmove(state_init_ref.hash, ctx->transaction.state_init_hash, 32);

        // Hash cell
        struct CellRef_t internalMessageRefs[1] = {state_init_ref};
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