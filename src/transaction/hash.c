#include "hash.h"
#include "cx.h"

bool hash_tx(transaction_ctx_t *ctx) {

    cx_sha256_t state;
    cx_sha256_init(&state);
    cx_hash((cx_hash_t *) &state, 
        CX_LAST, 
        ctx->raw_tx,
        ctx->raw_tx_len, 
        ctx->m_hash, 
        sizeof(ctx->m_hash)
    );

    return true;
}