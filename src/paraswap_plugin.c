#include "paraswap_plugin.h"

static const uint8_t PARASWAP_SWAP_ON_UNISWAP_SELECTOR[SELECTOR_SIZE] = {0x58, 0xb9, 0xd1, 0x79};
static const uint8_t PARASWAP_SWAP_ON_UNISWAP_FORK_SELECTOR[SELECTOR_SIZE] = {0x08,
                                                                              0x63,
                                                                              0xb7,
                                                                              0xac};
static const uint8_t PARASWAP_SIMPLE_SWAP_SELECTOR[SELECTOR_SIZE] = {0xcf, 0xc0, 0xaf, 0xeb};
static const uint8_t PARASWAP_MULTI_SWAP_SELECTOR[SELECTOR_SIZE] = {0x8f, 0x00, 0xec, 0xcb};
static const uint8_t PARASWAP_MEGA_SWAP_SELECTOR[SELECTOR_SIZE] = {0xec, 0x1d, 0x21, 0xdd};
static const uint8_t PARASWAP_BUY_ON_UNISWAP_SELECTOR[SELECTOR_SIZE] = {0xf9, 0x35, 0x5f, 0x72};
static const uint8_t PARASWAP_BUY_ON_UNISWAP_FORK_SELECTOR[SELECTOR_SIZE] = {0x33,
                                                                             0x63,
                                                                             0x52,
                                                                             0x26};
static const uint8_t PARASWAP_SIMPLE_BUY_SELECTOR[SELECTOR_SIZE] = {0xa2, 0x7e, 0x8b, 0x6b};
static const uint8_t PARASWAP_BUY_SELECTOR[SELECTOR_SIZE] = {0xf9, 0x5a, 0x49, 0xeb};

// Array of all the different paraswap selectors.
const uint8_t *const PARASWAP_SELECTORS[NUM_PARASWAP_SELECTORS] = {
    PARASWAP_SWAP_ON_UNISWAP_SELECTOR,
    PARASWAP_BUY_ON_UNISWAP_SELECTOR,
    PARASWAP_SWAP_ON_UNISWAP_FORK_SELECTOR,
    PARASWAP_BUY_ON_UNISWAP_FORK_SELECTOR,
    PARASWAP_SIMPLE_SWAP_SELECTOR,
    PARASWAP_SIMPLE_BUY_SELECTOR,
    PARASWAP_MULTI_SWAP_SELECTOR,
    PARASWAP_BUY_SELECTOR,
    PARASWAP_MEGA_SWAP_SELECTOR,
};

// Paraswap uses `0xeeeee` as a dummy address to represent ETH.
const uint8_t PARASWAP_ETH_ADDRESS[ADDRESS_LENGTH] = {0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
                                                      0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
                                                      0xee, 0xee, 0xee, 0xee, 0xee, 0xee};

// Used to indicate that the beneficiary should be the sender.
const uint8_t NULL_ETH_ADDRESS[ADDRESS_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// Prepend `dest` with `ticker`.
// Dest must be big enough to hold `ticker` + `dest` + `\0`.
static void prepend_ticker(char *dest, uint8_t destsize, char *ticker) {
    if (dest == NULL || ticker == NULL) {
        THROW(0x6503);
    }
    uint8_t ticker_len = strlen(ticker);
    uint8_t dest_len = strlen(dest);

    if (dest_len + ticker_len >= destsize) {
        THROW(0x6503);
    }

    // Right shift the string by `ticker_len` bytes.
    while (dest_len != 0) {
        dest[dest_len + ticker_len] = dest[dest_len];  // First iteration will copy the \0
        dest_len--;
    }
    // Don't forget to null terminate the string.
    dest[ticker_len] = dest[0];

    // Copy the ticker to the beginning of the string.
    memcpy(dest, ticker, ticker_len);
}

// Called once to init.
static void handle_init_contract(void *parameters) {
    ethPluginInitContract_t *msg = (ethPluginInitContract_t *) parameters;
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;
    memset(context, 0, sizeof(*context));
    context->valid = 1;

    size_t i;
    for (i = 0; i < NUM_PARASWAP_SELECTORS; i++) {
        if (memcmp((uint8_t *) PIC(PARASWAP_SELECTORS[i]), msg->selector, SELECTOR_SIZE) == 0) {
            context->selectorIndex = i;
            break;
        }
    }

    // Set `next_param` to be the first field we expect to parse.
    switch (context->selectorIndex) {
        case BUY_ON_UNI_FORK:
        case SWAP_ON_UNI_FORK:
        case BUY_ON_UNI:
        case SWAP_ON_UNI:
            if (context->selectorIndex == SWAP_ON_UNI_FORK ||
                context->selectorIndex == BUY_ON_UNI_FORK) {
                context->skip =
                    2;  // Skip the first two parameters (factory and initCode) for uni forks.
            }
            context->next_param = AMOUNT_SENT;
            break;
        case SIMPLE_BUY:
        case SIMPLE_SWAP:
            context->next_param = TOKEN_SENT;
            break;
        case MEGA_SWAP:
        case BUY:
        case MULTI_SWAP:
            context->next_param = TOKEN_SENT;
            context->skip = 1;  // Skipping 0x20 (offset of structure)
            break;
        default:
            PRINTF("Missing selectorIndex\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}

static void handle_finalize(void *parameters) {
    ethPluginFinalize_t *msg = (ethPluginFinalize_t *) parameters;
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;
    PRINTF("eth2 plugin finalize\n");
    if (context->valid) {
        msg->numScreens = 2;
        if (context->selectorIndex == SIMPLE_SWAP || context->selectorIndex == SIMPLE_BUY)
            if (strncmp(context->beneficiary, (char *) NULL_ETH_ADDRESS, ADDRESS_LENGTH) != 0) {
                // An addiitonal screen is required to display the `beneficiary` field.
                msg->numScreens += 1;
            }
        if (ADDRESS_IS_ETH(context->contract_address_sent) == 0) {
            // Address is not ETH so we will need to look up the token in the CAL.
            msg->tokenLookup1 = context->contract_address_sent;
            PRINTF("Setting address sent to: %.*H\n",
                   ADDRESS_LENGTH,
                   context->contract_address_sent);
        } else {
            msg->tokenLookup1 = NULL;
        }
        if (ADDRESS_IS_ETH(context->contract_address_received) == 0) {
            // Address is not ETH so we will need to look up the token in the CAL.
            PRINTF("Setting address receiving to: %.*H\n",
                   ADDRESS_LENGTH,
                   context->contract_address_received);
            msg->tokenLookup2 = context->contract_address_received;
        } else {
            msg->tokenLookup2 = NULL;
        }

        msg->uiType = ETH_UI_TYPE_GENERIC;
        msg->result = ETH_PLUGIN_RESULT_OK;
    } else {
        PRINTF("Context not valid\n");
        msg->result = ETH_PLUGIN_RESULT_FALLBACK;
    }
}

static void handle_provide_token(void *parameters) {
    ethPluginProvideToken_t *msg = (ethPluginProvideToken_t *) parameters;
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;
    PRINTF("PARASWAP plugin provide token: 0x%p, 0x%p\n", msg->token1, msg->token2);
    if (msg->token1 != NULL) {
        context->decimals_sent = msg->token1->decimals;
        strncpy(context->ticker_sent, (char *) msg->token1->ticker, sizeof(context->ticker_sent));
    } else {
        context->decimals_sent = WEI_TO_ETHER;
        strncpy(context->ticker_sent, "ETH", sizeof(context->ticker_sent));
    }
    if (msg->token2 != NULL) {
        context->decimals_received = msg->token2->decimals;
        strncpy(context->ticker_received,
                (char *) msg->token2->ticker,
                sizeof(context->ticker_received));
    } else {
        context->decimals_received = WEI_TO_ETHER;
        strncpy(context->ticker_received, "ETH", sizeof(context->ticker_received));
    }
    msg->result = ETH_PLUGIN_RESULT_OK;
}

static void handle_query_contract_id(void *parameters) {
    ethQueryContractID_t *msg = (ethQueryContractID_t *) parameters;
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;

    strncpy(msg->name, PLUGIN_NAME, SHARED_CTX_FIELD_1_SIZE);
    msg->nameLength = sizeof(PLUGIN_NAME);

    switch (context->selectorIndex) {
        case MEGA_SWAP:
        case MULTI_SWAP:
        case SIMPLE_SWAP:
        case SWAP_ON_UNI_FORK:
        case SWAP_ON_UNI:
            strncpy(msg->version, "Swap", SHARED_CTX_FIELD_2_SIZE);
            msg->versionLength = sizeof("Swap");
            break;
        case SIMPLE_BUY:
        case BUY_ON_UNI_FORK:
        case BUY_ON_UNI:
        case BUY:
            strncpy(msg->version, "Buy", SHARED_CTX_FIELD_2_SIZE);
            msg->versionLength = sizeof("Buy");
            break;
        default:
            PRINTF("Selector Index :%d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    msg->versionLength = strnlen(msg->version, SHARED_CTX_FIELD_2_SIZE) + 1;
    msg->result = ETH_PLUGIN_RESULT_OK;
}

// Set UI for the "Send" screen.
static void set_send_ui(ethQueryContractUI_t *msg, paraswap_parameters_t *context) {
    switch (context->selectorIndex) {
        case SWAP_ON_UNI_FORK:
        case SWAP_ON_UNI:
        case SIMPLE_SWAP:
        case MEGA_SWAP:
        case MULTI_SWAP:
            strncpy(msg->title, "Send", SHARED_CTX_FIELD_1_SIZE);
            break;
        case BUY_ON_UNI_FORK:
        case BUY_ON_UNI:
        case BUY:
        case SIMPLE_BUY:
            strncpy(msg->title, "Send max", SHARED_CTX_FIELD_1_SIZE);
            break;
        default:
            PRINTF("Unhandled selector Index: %d\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    adjustDecimals((char *) context->amount_sent,
                   strnlen((char *) context->amount_sent, sizeof(context->amount_sent)),
                   msg->msg,
                   SHARED_CTX_FIELD_2_SIZE,
                   context->decimals_sent);

    prepend_ticker(msg->msg, SHARED_CTX_FIELD_2_SIZE, context->ticker_sent);

    msg->titleLength = strnlen(msg->msg, SHARED_CTX_FIELD_1_SIZE);
    msg->msgLength = strnlen(msg->msg, SHARED_CTX_FIELD_2_SIZE);
}

// Set UI for "Receive" screen.
static void set_receive_ui(ethQueryContractUI_t *msg, paraswap_parameters_t *context) {
    switch (context->selectorIndex) {
        case SWAP_ON_UNI_FORK:
        case SWAP_ON_UNI:
        case SIMPLE_SWAP:
        case MEGA_SWAP:
        case MULTI_SWAP:
            strncpy(msg->title, "Receive min", SHARED_CTX_FIELD_1_SIZE);
            break;
        case BUY_ON_UNI_FORK:
        case BUY_ON_UNI:
        case BUY:
        case SIMPLE_BUY:
            strncpy(msg->title, "Receive", SHARED_CTX_FIELD_1_SIZE);
            break;
        default:
            PRINTF("Unhandled selector Index: %d\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    adjustDecimals((char *) context->amount_received,
                   strnlen((char *) context->amount_received, sizeof(context->amount_received)),
                   msg->msg,
                   SHARED_CTX_FIELD_2_SIZE,
                   context->decimals_received);

    prepend_ticker(msg->msg, SHARED_CTX_FIELD_2_SIZE, context->ticker_received);

    msg->titleLength = strnlen(msg->msg, SHARED_CTX_FIELD_1_SIZE);
    msg->msgLength = strnlen(msg->msg, SHARED_CTX_FIELD_2_SIZE);
}

// Set UI for "Beneficiary" screen.
static void set_beneficiary_ui(ethQueryContractUI_t *msg, paraswap_parameters_t *context) {
    strncpy(msg->title, "Beneficiary", SHARED_CTX_FIELD_1_SIZE);

    msg->msg[0] = '0';
    msg->msg[1] = 'x';

    chain_config_t chainConfig = {0};

    getEthAddressStringFromBinary((uint8_t *) context->beneficiary,
                                  (uint8_t *) msg->msg + 2,
                                  msg->pluginSharedRW->sha3,
                                  &chainConfig);

    msg->titleLength = strnlen(msg->msg, SHARED_CTX_FIELD_1_SIZE);
    msg->msgLength = strnlen(msg->msg, SHARED_CTX_FIELD_2_SIZE);
}

static void handle_query_contract_ui(void *parameters) {
    ethQueryContractUI_t *msg = (ethQueryContractUI_t *) parameters;
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;

    memset(msg->title, 0, SHARED_CTX_FIELD_1_SIZE);
    memset(msg->msg, 0, SHARED_CTX_FIELD_2_SIZE);

    msg->result = ETH_PLUGIN_RESULT_OK;

    switch (msg->screenIndex) {
        case 0:
            set_send_ui(msg, context);
            break;
        case 1:
            set_receive_ui(msg, context);
            break;
        case 2:
            set_beneficiary_ui(msg, context);
            break;
        default:
            PRINTF("Received an invalid screenIndex\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }
}

void paraswap_plugin_call(int message, void *parameters) {
    switch (message) {
        case ETH_PLUGIN_INIT_CONTRACT:
            handle_init_contract(parameters);
            break;
        case ETH_PLUGIN_PROVIDE_PARAMETER:
            handle_provide_parameter(parameters);
            break;
        case ETH_PLUGIN_FINALIZE:
            handle_finalize(parameters);
            break;
        case ETH_PLUGIN_PROVIDE_TOKEN:
            handle_provide_token(parameters);
            break;
        case ETH_PLUGIN_QUERY_CONTRACT_ID:
            handle_query_contract_id(parameters);
            break;
        case ETH_PLUGIN_QUERY_CONTRACT_UI:
            handle_query_contract_ui(parameters);
            break;
        default:
            PRINTF("Unhandled message %d\n", message);
            break;
    }
}
