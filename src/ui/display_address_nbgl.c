#ifdef HAVE_NBGL

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "nbgl_use_case.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../common/bip32.h"
#include "../common/format.h"
#include "../common/base64.h"
#include "../menu.h"

static char g_address[49];

static void confirm_address_rejection(void) {
    // display a status page and go back to main
    ui_action_validate_pubkey(false);
    nbgl_useCaseStatus("Address verification\ncancelled", false, ui_menu_main);
}

static void confirm_address_approval(void) {
    // display a success status page and go back to main
    ui_action_validate_pubkey(true);
    nbgl_useCaseStatus("ADDRESS\nVERIFIED", true, ui_menu_main);
}

static void review_choice(bool confirm) {
    if (confirm) {
        confirm_address_approval();
    } else {
        confirm_address_rejection();
    }
}

static void continue_review(void) {
    nbgl_useCaseAddressConfirmation(g_address, review_choice);
}

int ui_display_address(uint8_t flags) {
    if (G_context.req_type != CONFIRM_ADDRESS || G_context.state != STATE_NONE) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    // Format address
    memset(g_address, 0, sizeof(g_address));
    uint8_t address[ADDRESS_LEN] = {0};
    bool bounceable = true;
    bool testnet = false;
    uint8_t chain = 0;
    if (flags & 0x01) {
        bounceable = false;
    }
    if (flags & 0x02) {
        testnet = true;
    }
    if (flags & 0x04) {
        chain = 0xff;
    }
    if (!address_from_pubkey(G_context.pk_info.raw_public_key,
                             chain,
                             bounceable,
                             testnet,
                             address,
                             sizeof(address))) {
        return io_send_sw(SW_DISPLAY_ADDRESS_FAIL);
    }
    base64_encode(address, sizeof(address), g_address, sizeof(g_address));

    nbgl_useCaseReviewStart(&C_ledger_stax_ton_64,
                            "Verify TON address",
                            NULL,
                            "Cancel",
                            continue_review,
                            confirm_address_rejection);

    return 0;
}

#endif
