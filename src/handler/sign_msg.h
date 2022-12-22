#pragma once

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool

#include "../common/buffer.h"

/**
 * Handler for SIGN_MSG command. If successfully parse BIP32 path
 * and transaction, sign message and send APDU response.
 *
 * @see G_context.bip32_path, G_context.msg_info.msg,
 * G_context.msg_info.signature and G_context.msg_info.hash
 *
 * @param[in,out] cdata
 *   Command data with BIP32 path and raw transaction serialized.
 * @param[in]     chunk
 *   Index number of the APDU chunk.
 * @param[in]       more
 *   Whether more APDU chunk to be received or not.
 *
 * @return zero or positive integer if success, negative integer otherwise.
 *
 */
int handler_sign_msg(buffer_t *cdata, uint8_t chunk, bool more);
