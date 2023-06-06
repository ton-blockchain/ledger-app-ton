#ifdef HAVE_NBGL

#pragma GCC diagnostic ignored "-Wformat-invalid-specifier"  // snprintf
#pragma GCC diagnostic ignored "-Wformat-extra-args"         // snprintf

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
#include "../address.h"
#include "action/validate.h"
#include "../transaction/types.h"
#include "../transaction/hints.h"
#include "../common/bip32.h"
#include "../common/format.h"
#include "../common/base64.h"
#include "../common/format_bigint.h"
#include "../menu.h"

static char g_operation[64];
static char g_amount[30];
static char g_address[49];
static char g_payload[64];

#define HINT_TITLE_SIZE 32
#define HINT_BODY_SIZE 256

static char g_hint_title[HINT_TITLE_SIZE];
static char g_hint_body[HINT_BODY_SIZE];

static char g_hint_titles[3][HINT_TITLE_SIZE];
static char g_hint_bodies[3][HINT_BODY_SIZE];

static uint8_t g_pages;

static nbgl_layoutTagValue_t pairs[16];
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

static void review_continue(void) {
    // Setup list
    pairList.nbMaxLinesForValue = 0;

    // Info long press
    infoLongPress.icon = &C_ledger_stax_ton_64;
    infoLongPress.text = "Sign transaction\nto send TON";
    infoLongPress.longPressText = "Hold to sign";

    nbgl_useCaseStaticReview(&pairList, &infoLongPress, "Reject transaction", on_review_choice);
}

static void review_general_params(void) {
    // Operation
    memset(g_operation, 0, sizeof(g_operation));
    snprintf(g_operation, sizeof(g_operation), "%s", G_context.tx_info.transaction.title);

    pairs[0].item = "Operation";
    pairs[0].value = g_operation;

    // Amount
    memset(g_amount, 0, sizeof(g_amount));
    if ((G_context.tx_info.transaction.send_mode & 128) != 0) {
        snprintf(g_amount, sizeof(g_amount), "ALL YOUR TONs");
    } else {
        char amount[30] = {0};
        if (!format_fpu64(amount,
                          sizeof(amount),
                          G_context.tx_info.transaction.value,
                          EXPONENT_SMALLEST_UNIT)) {
            return;
        }
        snprintf(g_amount, sizeof(g_amount), "TON %.*s", sizeof(amount), amount);
    }

    pairs[1].item = "Amount";
    pairs[1].value = g_amount;

    // Address
    uint8_t address[ADDRESS_LEN] = {0};
    address_to_friendly(G_context.tx_info.transaction.to.chain,
                        G_context.tx_info.transaction.to.hash,
                        true,
                        false,
                        address,
                        sizeof(address));
    memset(g_address, 0, sizeof(g_address));
    base64_encode(address, sizeof(address), g_address, sizeof(g_address));

    pairs[2].item = "Address";
    pairs[2].value = g_address;

    pairList.nbMaxLinesForValue = 0;
    pairList.nbPairs = 3;
    pairList.pairs = pairs;
}

static void blind_warning_callback(bool confirm) {
    if (confirm) {
        review_general_params();
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
        content->tagValueList.pairs = (nbgl_layoutTagValue_t *)pairs;
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
            content->tagValueList.pairs = (nbgl_layoutTagValue_t *)pairs;
            content->tagValueList.smallCaseForValue = false;
        } else {
            uint16_t startHintIndex = (page - 1) * 3; // 1st page is general params
            uint16_t nextHintIndex = startHintIndex + 3;
            if (nextHintIndex > G_context.tx_info.transaction.hints_count) {
                nextHintIndex = G_context.tx_info.transaction.hints_count;
            }
            for (uint16_t hintIndex = startHintIndex; hintIndex < nextHintIndex; hintIndex++) {
                uint16_t hintCharIndex = hintIndex - startHintIndex;
                print_hint(&G_context.tx_info.transaction, hintIndex, g_hint_titles[hintCharIndex], HINT_TITLE_SIZE, g_hint_bodies[hintCharIndex], HINT_BODY_SIZE);
                pairs[hintCharIndex].item = g_hint_titles[hintCharIndex];
                pairs[hintCharIndex].value = g_hint_bodies[hintCharIndex];
            }
            content->type = TAG_VALUE_LIST;
            content->tagValueList.nbPairs = nextHintIndex - startHintIndex;
            content->tagValueList.pairs = (nbgl_layoutTagValue_t *)pairs;
            content->tagValueList.smallCaseForValue = false;
        }
    } else if (page == g_pages - 1) {
        content->type = INFO_LONG_PRESS,
        content->infoLongPress.icon = &C_ledger_stax_ton_64;
        content->infoLongPress.text = "Sign transaction\nto send TON";
        content->infoLongPress.longPressText = "Hold to sign";
        content->infoLongPress.longPressToken = 0;
    } else {
        return false;
    }

    return true;
}

static void start_regular_review(void) {
    g_pages = 2; // 1st page is general params, last page is final confirmation
    if (G_context.tx_info.transaction.has_payload) {
        if (G_context.tx_info.transaction.is_blind) {
            g_pages++; // display payload hash
        } else {
            g_pages += (G_context.tx_info.transaction.hints_count + 2) / 3; // 1 page per 3 hints
        }
    }

    nbgl_useCaseRegularReview(0, g_pages, "Reject transaction",
                            NULL, // no buttons because no value is too long to fit
                            display_review_page, on_review_choice);
}

static void show_blind_warning_if_needed(void) {
    if (G_context.tx_info.transaction.is_blind) {
        nbgl_useCaseReviewStart(&C_round_warning_64px,
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

    // Operation
    memset(g_operation, 0, sizeof(g_operation));
    snprintf(g_operation, sizeof(g_operation), "%s", G_context.tx_info.transaction.title);

    // Amount
    memset(g_amount, 0, sizeof(g_amount));
    if ((G_context.tx_info.transaction.send_mode & 128) != 0) {
        snprintf(g_amount, sizeof(g_amount), "ALL YOUR TONs");
    } else {
        if (!amountToString(G_context.tx_info.transaction.value_buf,
                    G_context.tx_info.transaction.value_len,
                    EXPONENT_SMALLEST_UNIT,
                    "TON",
                    g_amount,
                    sizeof(g_amount))) {
            return io_send_sw(SW_DISPLAY_AMOUNT_FAIL);
        }
    }

    // Address
    uint8_t address[ADDRESS_LEN] = {0};
    address_to_friendly(G_context.tx_info.transaction.to.chain,
                        G_context.tx_info.transaction.to.hash,
                        true,
                        false,
                        address,
                        sizeof(address));
    memset(g_address, 0, sizeof(g_address));
    base64_encode(address, sizeof(address), g_address, sizeof(g_address));

    // Payload
    memset(g_payload, 0, sizeof(g_payload));
    if (G_context.tx_info.transaction.has_payload) {
        base64_encode(G_context.tx_info.transaction.payload.hash, 32, g_payload, sizeof(g_payload));
    } else {
        snprintf(g_payload, sizeof(g_payload), "Nothing");
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
