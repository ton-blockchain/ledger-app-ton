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

parser_status_e transaction_deserialize(buffer_t *buf, transaction_t *tx) {
    if (buf->size > MAX_TX_LEN) {
        return WRONG_LENGTH_ERROR;
    }

    // tag
    if (!buffer_read_u8(buf, &tx->tag)) {
        return TAG_PARSING_ERROR;
    }
    if (tx->tag != 0x00) {  // Only 0x00 is supported now
        return TAG_PARSING_ERROR;
    }

    // seqno
    if (!buffer_read_u32(buf, &tx->seqno, BE)) {
        return SEQ_PARSING_ERROR;
    }

    // timeout
    if (!buffer_read_u32(buf, &tx->timeout, BE)) {
        return TIMEOUT_PARSING_ERROR;
    }

    // amount value
    if (!buffer_read_u64(buf, &tx->value, BE)) {
        return VALUE_PARSING_ERROR;
    }

    // address
    if (!buffer_read_u8(buf, &tx->to_chain)) {
        return TO_PARSING_ERROR;
    }
    tx->to_hash = (uint8_t *) (buf->ptr + buf->offset);
    if (!buffer_seek_cur(buf, 32)) {
        return TO_PARSING_ERROR;
    }

    // bounce
    if (!buffer_read_u8(buf, &tx->bounce)) {
        return BOUNCE_PARSING_ERROR;
    }

    // send mode
    if (!buffer_read_u8(buf, &tx->send_mode)) {
        return SEND_MODE_PARSING_ERROR;
    }

    // Payload
    if (!buffer_read_u8(buf, &tx->payload)) {
        return PAYLOAD_PARSING_ERROR;
    }
    if (tx->payload > 0) {
        if (!buffer_read_u16(buf, &tx->payload_depth, BE)) {
            return PAYLOAD_PARSING_ERROR;
        }
        tx->payload_hash = (uint8_t *) (buf->ptr + buf->offset);
        if (!buffer_seek_cur(buf, 32)) {
            return PAYLOAD_PARSING_ERROR;
        }
    }

    return (buf->offset == buf->size) ? PARSING_OK : WRONG_LENGTH_ERROR;
}
