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

#include "deserialize.h"
#include "utils.h"
#include "types.h"
#include "../common/buffer.h"
#include "hash.h"
#include "cell.h"

#define SAFE(RES, CODE) \
    if (!RES) {         \
        return CODE;    \
    }

parser_status_e transaction_deserialize(buffer_t *buf, transaction_t *tx) {
    if (buf->size > MAX_TX_LEN) {
        return WRONG_LENGTH_ERROR;
    }

    // tag
    SAFE(buffer_read_u8(buf, &tx->tag), TAG_PARSING_ERROR);
    if (tx->tag != 0x00) {  // Only 0x00 is supported now
        return TAG_PARSING_ERROR;
    }

    // Basic Transaction parameters
    SAFE(buffer_read_u32(buf, &tx->seqno, BE), SEQ_PARSING_ERROR);
    SAFE(buffer_read_u32(buf, &tx->timeout, BE), TIMEOUT_PARSING_ERROR);
    SAFE(buffer_read_u64(buf, &tx->value, BE), VALUE_PARSING_ERROR);

    // address
    SAFE(buffer_read_u8(buf, &tx->to_chain), TO_PARSING_ERROR);
    SAFE(buffer_red_ref(buf, &tx->to_hash, 32), TO_PARSING_ERROR);

    // Flags
    SAFE(buffer_read_u8(buf, &tx->bounce), BOUNCE_PARSING_ERROR);
    SAFE(buffer_read_u8(buf, &tx->send_mode), SEND_MODE_PARSING_ERROR);

    // state-init
    SAFE(buffer_read_bool(buf, &tx->state_init), STATE_INIT_PARSING_ERROR);
    if (tx->state_init) {
        SAFE(buffer_read_u16(buf, &tx->state_init_depth, BE), STATE_INIT_PARSING_ERROR);
        SAFE(buffer_red_ref(buf, &tx->state_init_hash, 32), STATE_INIT_PARSING_ERROR);
    }

    // Payload
    SAFE(buffer_read_bool(buf, &tx->payload), PAYLOAD_PARSING_ERROR);
    if (tx->payload) {
        SAFE(buffer_read_u16(buf, &tx->payload_depth, BE), PAYLOAD_PARSING_ERROR);
        SAFE(buffer_red_ref(buf, &tx->payload_hash, 32), PAYLOAD_PARSING_ERROR);
    }

    // Hints
    SAFE(buffer_read_bool(buf, &tx->hints), HINTS_PARSING_ERROR);
    if (tx->hints) {
        SAFE(buffer_read_u64(buf, &tx->hints_type, BE), HINTS_PARSING_ERROR);
        SAFE(buffer_read_u16(buf, &tx->hints_len, BE), HINTS_PARSING_ERROR);
        SAFE(buffer_red_ref(buf, &tx->hints_data, tx->hints_len), PAYLOAD_PARSING_ERROR);
    }

    return (buf->offset == buf->size) ? PARSING_OK : WRONG_LENGTH_ERROR;
}
