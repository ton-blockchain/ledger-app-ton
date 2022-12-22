#include "hash.h"
#include "cx.h"
#include "cell.h"
#include "../common/bits.h"
#include <string.h>  // memmove

bool hash_msg(message_ctx_t *ctx) {
    cx_sha256_t state;
    cx_sha256_init(&state);
    cx_hash((cx_hash_t *) &state, 0, ctx->msg, ctx->msg_len, NULL, 0);
    cx_hash((cx_hash_t *) &state, CX_LAST, NULL, 0, ctx->m_hash, 32);
    return true;
}