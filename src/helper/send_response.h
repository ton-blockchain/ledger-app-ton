#pragma once

#include "os.h"

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
 * Helper to send APDU response with signature of a tx
 *
 * response = SIG_LEN (1) ||
 *            G_context.tx_info.signature (SIG_LEN) ||
 *            HASH_LEN (1) ||
 *            G_context.tx_info.m_hash (HASH_LEN)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_sig(void);

/**
 * Helper to send APDU response with signature of a proof
 *
 * response = SIG_LEN (1) ||
 *            G_context.proof_info.signature (SIG_LEN) ||
 *            HASH_LEN (1) ||
 *            G_context.proof_info.hash (HASH_LEN)
 *
 * @return zero or positive integer if success, -1 otherwise.
 *
 */
int helper_send_response_sig_proof(void);
