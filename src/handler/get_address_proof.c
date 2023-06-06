/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>   // uint*_t
#include <stdbool.h>  // bool
#include <stddef.h>   // size_t
#include <string.h>   // memset, explicit_bzero

#include "os.h"
#include "cx.h"

#include "get_address_proof.h"
#include "../globals.h"
#include "../types.h"
#include "../io.h"
#include "../sw.h"
#include "../crypto.h"
#include "../common/buffer.h"
#include "../ui/display.h"
#include "../helper/send_response.h"

static const uint8_t TON_PROOF_ITEM_STR[] = "ton-proof-item-v2/";
static const uint8_t TON_CONNECT_STR[] = "\xff\xffton-connect";

int handler_get_address_proof(uint8_t flags, buffer_t *cdata, bool display) {
    explicit_bzero(&G_context, sizeof(G_context));
    G_context.req_type = GET_PROOF;
    G_context.state = STATE_NONE;

    cx_ecfp_private_key_t private_key = {0};
    cx_ecfp_public_key_t public_key = {0};

    if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
        !buffer_read_bip32_path(cdata, G_context.bip32_path, (size_t) G_context.bip32_path_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    // derive private key according to BIP32 path
    crypto_derive_private_key(&private_key, G_context.bip32_path, G_context.bip32_path_len);
    // generate corresponding public key
    crypto_init_public_key(&private_key, &public_key, G_context.proof_info.raw_public_key);
    // reset private key
    explicit_bzero(&private_key, sizeof(private_key));


    G_context.proof_info.workchain = (flags & 0x04) ? -1 : 0;

    if (!buffer_read_u8(cdata, &G_context.proof_info.domain_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    if (G_context.proof_info.domain_len > sizeof(G_context.proof_info.domain)) {
        return io_send_sw(SW_REQUEST_TOO_LONG);
    }

    if (!buffer_read_buffer(cdata, G_context.proof_info.domain, G_context.proof_info.domain_len)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    uint64_t timestamp;

    if (!buffer_read_u64(cdata, &timestamp, BE)) {
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    size_t payload_len = buffer_remaining(cdata);
    uint8_t payload[128];

    if (payload_len > sizeof(payload)) {
        return io_send_sw(SW_REQUEST_TOO_LONG);
    }

    if (!buffer_move(cdata, payload, payload_len)) {
        // impossible
        return io_send_sw(SW_WRONG_DATA_LENGTH);
    }

    cx_sha256_t state;
    cx_sha256_init(&state);

    // sizeof - 1 because const strings are null terminated
    cx_hash((cx_hash_t *) &state, 0, TON_PROOF_ITEM_STR, sizeof(TON_PROOF_ITEM_STR) - 1, NULL, 0);

    for (int i = 3; i >= 0; i--) {
        uint8_t wc_part = (G_context.proof_info.workchain >> (i * 8)) & 0xff;
        cx_hash((cx_hash_t *) &state, 0, &wc_part, 1, NULL, 0);
    }
    cx_hash((cx_hash_t *) &state, 0, G_context.proof_info.raw_public_key, sizeof(G_context.proof_info.raw_public_key), NULL, 0);

    for (int i = 3; i >= 0; i--) {
        uint8_t domain_len_part = (G_context.proof_info.domain_len >> (i * 8)) & 0xff;
        cx_hash((cx_hash_t *) &state, 0, &domain_len_part, 1, NULL, 0);
    }
    cx_hash((cx_hash_t *) &state, 0, G_context.proof_info.domain, G_context.proof_info.domain_len, NULL, 0);

    for (int i = 7; i >= 0; i--) {
        uint8_t ts_part = (timestamp >> (i * 8)) & 0xff;
        cx_hash((cx_hash_t *) &state, 0, &ts_part, 1, NULL, 0);
    }

    uint8_t inner[HASH_LEN];

    cx_hash((cx_hash_t *) &state, CX_LAST, payload, payload_len, inner, sizeof(inner));

    cx_sha256_init(&state);

    // sizeof - 1 because const strings are null terminated
    cx_hash((cx_hash_t *) &state, 0, TON_CONNECT_STR, sizeof(TON_CONNECT_STR) - 1, NULL, 0);

    cx_hash((cx_hash_t *) &state, CX_LAST, inner, sizeof(inner), G_context.proof_info.hash, sizeof(G_context.proof_info.hash));

    if (display) {
        return ui_display_proof(flags);
    }

    int r = crypto_sign_proof();
    if (r != 0) {
        return r;
    }

    return helper_send_response_sig_proof();
}
