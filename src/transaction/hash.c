#include "hash.h"
#include "cx.h"

bool hash_tx(transaction_ctx_t *ctx) {

    cx_sha3_t keccak256;
    cx_keccak_init(&keccak256, 256);
    cx_hash((cx_hash_t *) &keccak256,
            CX_LAST,
            ctx->raw_tx,
            ctx->raw_tx_len,
            ctx->m_hash,
            sizeof(ctx->m_hash));

    return true;
}