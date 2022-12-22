#pragma once

#include "../types.h"

/**
 * Computes message hash for signing
 *
 * @param[out]     ctx
 *   Pointer to message structure.
 *
 * @return true if success, false otherwise.
 *
 */
bool hash_msg(message_ctx_t *ctx);