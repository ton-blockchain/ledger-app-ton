#pragma once

#include "os.h"

#include "../common/macros.h"

/**
 * Length of public key.
 */
#define PUBKEY_LEN (MEMBER_SIZE(pubkey_ctx_t, raw_public_key))

/**
 * Helper to send APDU response with public key and chain code.
 *
 * response = G_context.pk_info.public_key (PUBKEY_LEN)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_pubkey(void);

/**
 * Helper to send APDU response with signature or a tx
 *
 * response = G_context.tx_info.signature_len (1) ||
 *            G_context.tx_info.signature (G_context.tx_info.signature_len)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_sig(void);


/**
 * Helper to send APDU response with signature of a message
 *
 * response = G_context.msg_info.signature_len (1) ||
 *            G_context.msg_info.signature (G_context.msg_info.signature_len)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_sig_msg(void);
