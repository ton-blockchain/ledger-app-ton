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

#define MAX_PAIRS_PER_PAGE 3

static char g_hint_titles[MAX_PAIRS_PER_PAGE][HINT_TITLE_SIZE];
static char g_hint_bodies[MAX_PAIRS_PER_PAGE][HINT_BODY_SIZE];

static uint8_t g_pages;

static nbgl_layoutTagValue_t pairs[MAX_PAIRS_PER_PAGE];

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

static bool display_review_page(uint8_t page, nbgl_pageContent_t *content) {
    if (page == 0) {
        // general info
        pairs[0].item = "Transaction type";
        pairs[0].value = g_operation;
        pairs[1].item = "Amount";
        pairs[1].value = g_amount;
        pairs[2].item = "Address";
        pairs[2].value = g_address;
        content->type = TAG_VALUE_LIST;
        content->tagValueList.nbPairs = 3;
        content->tagValueList.pairs = (nbgl_layoutTagValue_t *) pairs;
        content->tagValueList.smallCaseForValue = false;
    } else if (page < g_pages - 1) {
        if (!G_context.tx_info.transaction.has_payload) {
            return false;
        }
        if (G_context.tx_info.transaction.is_blind) {
            pairs[0].item = "Payload hash";
            pairs[0].value = g_payload;
            content->type = TAG_VALUE_LIST;
            content->tagValueList.nbPairs = 1;
            content->tagValueList.pairs = (nbgl_layoutTagValue_t *) pairs;
            content->tagValueList.smallCaseForValue = false;
        } else {
            uint16_t startHintIndex =
                (page - 1) * MAX_PAIRS_PER_PAGE;  // 1st page is general params
            uint16_t nextHintIndex = startHintIndex + MAX_PAIRS_PER_PAGE;
            if (nextHintIndex > G_context.tx_info.transaction.hints_count) {
                nextHintIndex = G_context.tx_info.transaction.hints_count;
            }
            for (uint16_t hintIndex = startHintIndex; hintIndex < nextHintIndex; hintIndex++) {
                uint16_t hintCharIndex = hintIndex - startHintIndex;
                print_hint(&G_context.tx_info.transaction,
                           hintIndex,
                           g_hint_titles[hintCharIndex],
                           HINT_TITLE_SIZE,
                           g_hint_bodies[hintCharIndex],
                           HINT_BODY_SIZE);
                pairs[hintCharIndex].item = g_hint_titles[hintCharIndex];
                pairs[hintCharIndex].value = g_hint_bodies[hintCharIndex];
            }
            content->type = TAG_VALUE_LIST;
            content->tagValueList.nbPairs = nextHintIndex - startHintIndex;
            content->tagValueList.pairs = (nbgl_layoutTagValue_t *) pairs;
            content->tagValueList.smallCaseForValue = false;
        }
    } else if (page == g_pages - 1) {
        content->type = INFO_LONG_PRESS, content->infoLongPress.icon = &C_ledger_stax_ton_64;
        content->infoLongPress.text = "Sign transaction\nto send TON";
        content->infoLongPress.longPressText = "Hold to sign";
        content->infoLongPress.longPressToken = 0;
    } else {
        return false;
    }

    return true;
}

static void start_regular_review(void) {
    g_pages = 2;  // 1st page is general params, last page is final confirmation
    if (G_context.tx_info.transaction.has_payload) {
        if (G_context.tx_info.transaction.is_blind) {
            g_pages++;  // display payload hash
        } else {
            g_pages += (G_context.tx_info.transaction.hints_count + MAX_PAIRS_PER_PAGE - 1) /
                       MAX_PAIRS_PER_PAGE;  // 1 page per 3 hints
        }
    }

    nbgl_useCaseRegularReview(0,
                              g_pages,
                              "Reject transaction",
                              NULL,  // no buttons because no value is too long to fit
                              display_review_page,
                              on_review_choice);
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
