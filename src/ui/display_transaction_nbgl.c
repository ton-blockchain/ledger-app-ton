#ifdef HAVE_NBGL

#include <stdbool.h>  // bool
#include <string.h>   // memset

#include "os.h"
#include "glyphs.h"
#include "os_io_seproxyhal.h"
#include "nbgl_use_case.h"

#include "display.h"
#include "constants.h"
#include "../globals.h"
#include "../io.h"
#include "../sw.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../transaction/hints.h"
#include "../common/bip32.h"
#include "../common/base64.h"
#include "../common/format_bigint.h"
#include "../common/format_address.h"
#include "menu.h"
#include "helpers/display_transaction.h"

static char g_operation[G_OPERATION_LEN];
static char g_amount[G_AMOUNT_LEN];
static char g_address[G_ADDRESS_LEN];
static char g_payload[G_PAYLOAD_LEN];

static char g_hint_title_buffer[32 * MAX_HINTS];
static char g_hint_buffer[64 * MAX_HINTS];

static nbgl_layoutTagValue_t pairs[3+MAX_HINTS];
static nbgl_layoutTagValueList_t pairList;

static nbgl_pageInfoLongPress_t infoLongPress;

static void confirm_transaction_rejection(void) {
    // display a status page and go back to main
    ui_action_validate_transaction(false);
    nbgl_useCaseStatus("Transaction rejected", false, ui_menu_main);
}

static void ask_rejection_confirmation(void) {
    // display a choice to confirm/cancel rejection
    nbgl_useCaseConfirm("Reject transaction?",
                        NULL,
                        "Yes, reject",
                        "Go back to transaction",
                        confirm_transaction_rejection);
}

// called when long press button on 3rd page is long-touched or when reject footer is touched
static void on_review_choice(bool confirm) {
    if (confirm) {
        // display a status page and go back to main
        ui_action_validate_transaction(true);
        nbgl_useCaseStatus("TRANSACTION\nSIGNED", true, ui_menu_main);
    } else {
        ask_rejection_confirmation();
    }
}

static void start_regular_review(void) {
    int pairIndex = 0;

    pairs[pairIndex].item = "Transaction type";
    pairs[pairIndex].value = g_operation;
    pairIndex++;

    pairs[pairIndex].item = "Amount";
    pairs[pairIndex].value = g_amount;
    pairIndex++;

    pairs[pairIndex].item = "Address";
    pairs[pairIndex].value = g_address;
    pairIndex++;

    size_t hint_buffer_offset = 0;
    size_t hint_title_buffer_offset = 0;
    for (uint16_t i = 0; i < G_context.tx_info.transaction.hints_count; i++) {
        print_hint(&G_context.tx_info.transaction,
                   i,
                   &g_hint_title_buffer[hint_title_buffer_offset],
                   sizeof(g_hint_title_buffer) - hint_title_buffer_offset,
                   &g_hint_buffer[hint_buffer_offset],
                   sizeof(g_hint_buffer) - hint_buffer_offset);
        pairs[pairIndex].item = &g_hint_title_buffer[hint_title_buffer_offset];
        pairs[pairIndex].value = &g_hint_buffer[hint_buffer_offset];
        pairIndex++;
        hint_title_buffer_offset += strnlen(&g_hint_title_buffer[hint_title_buffer_offset], sizeof(g_hint_title_buffer) - hint_title_buffer_offset) + 1;
        hint_buffer_offset += strnlen(&g_hint_buffer[hint_buffer_offset], sizeof(g_hint_buffer) - hint_buffer_offset) + 1;
    }

    pairList.pairs = pairs;
    pairList.nbPairs = pairIndex;
    pairList.smallCaseForValue = false;

    infoLongPress.icon = &C_ledger_stax_ton_64;
    infoLongPress.text = "Sign transaction\nto send TON";
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Reject transaction", on_review_choice);
}

static void show_blind_warning_if_needed(void) {
    if (G_context.tx_info.transaction.is_blind) {
        nbgl_useCaseReviewStart(
            &C_round_warning_64px,
            "Blind Signing",
            "This transaction cannot be\nsecurely interpreted by Ledger\nStax. It might put "
            "your assets\nat risk.",
            "Reject transaction",
            start_regular_review,
            ask_rejection_confirmation);
    } else {
        start_regular_review();
    }
}

// Public function to start the transaction review
// - Check if the app is in the right state for transaction review
// - Format the amount and address strings in g_amount and g_address buffers
// - Display the first screen of the transaction review
int ui_display_transaction() {
    if (G_context.req_type != CONFIRM_TRANSACTION || G_context.state != STATE_PARSED) {
        G_context.state = STATE_NONE;
        return io_send_sw(SW_BAD_STATE);
    }

    if (!display_transaction(g_operation,
                             sizeof(g_operation),
                             g_amount,
                             sizeof(g_amount),
                             g_address,
                             sizeof(g_address),
                             g_payload,
                             sizeof(g_payload))) {
        return -1;
    }

    // Start review
    nbgl_useCaseReviewStart(&C_ledger_stax_ton_64,
                            "Review transaction\nto send TON",
                            NULL,
                            "Reject transaction",
                            show_blind_warning_if_needed,
                            ask_rejection_confirmation);

    return 0;
}

#endif
