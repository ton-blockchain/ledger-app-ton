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

#include "sign_msg.h"
#include "../sw.h"
#include "../globals.h"
#include "../crypto.h"
#include "../ui/display.h"
#include "../common/buffer.h"
#include "../msg/hash_msg.h"

int handler_sign_msg(buffer_t *cdata, uint8_t chunk, bool more) {
    if (chunk == 0) {  // first APDU, parse BIP32 path
        explicit_bzero(&G_context, sizeof(G_context));
        G_context.req_type = CONFIRM_MESSAGE;
        G_context.state = STATE_NONE;

        if (!buffer_read_u8(cdata, &G_context.bip32_path_len) ||
            !buffer_read_bip32_path(cdata,
                                    G_context.bip32_path,
                                    (size_t) G_context.bip32_path_len)) {
            return io_send_sw(SW_WRONG_DATA_LENGTH);
        }

        return io_send_sw(SW_OK);
    } else {  // parse transaction
        if (G_context.req_type != CONFIRM_MESSAGE) {
            return io_send_sw(SW_BAD_STATE);
        }

        if (more) {  // more APDUs with transaction part
            if (G_context.msg_info.msg_len + cdata->size > MAX_COMMENT_LEN &&  //
                !buffer_move(cdata,
                             G_context.msg_info.msg + G_context.msg_info.msg_len,
                             cdata->size)) {
                return io_send_sw(SW_WRONG_TX_LENGTH);
            }

            G_context.msg_info.msg_len += cdata->size;

            return io_send_sw(SW_OK);
        } else {  // last APDU, let's parse and sign
            if (G_context.msg_info.msg_len + cdata->size > MAX_COMMENT_LEN || //
                G_context.msg_info.msg_len + cdata->size == 0 || //
                !buffer_move(cdata,
                             G_context.msg_info.msg + G_context.msg_info.msg_len,
                             cdata->size)) {
                return io_send_sw(SW_WRONG_TX_LENGTH);
            }

            G_context.msg_info.msg_len += cdata->size;

            // Hash
            if (!hash_msg(&G_context.msg_info)) {
                return io_send_sw(SW_TX_PARSING_FAIL);
            }

            G_context.state = STATE_PARSED;

            PRINTF("Hash: %.*H\n", sizeof(G_context.msg_info.m_hash), G_context.msg_info.m_hash);

            return ui_display_message();
        }
    }

    return 0;
}
