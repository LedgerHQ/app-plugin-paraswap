#include "paraswap_plugin.h"

// Need more information about the interface for plugins? Please read the README.md!

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

// Called once to init.
static void handle_init_contract(void *parameters) {
    ethPluginInitContract_t *msg = (ethPluginInitContract_t *) parameters;

    if (msg->interfaceVersion != ETH_PLUGIN_INTERFACE_VERSION_LATEST) {
        msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
        return;
    }

    if (msg->pluginContextLength < sizeof(paraswap_parameters_t)) {
        msg->result = ETH_PLUGIN_RESULT_ERROR;
        return;
    }

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
    if (context->valid) {
        msg->numScreens = 2;
        if (context->selectorIndex == SIMPLE_SWAP || context->selectorIndex == SIMPLE_BUY)
            if (strncmp(context->beneficiary, (char *) NULL_ETH_ADDRESS, ADDRESS_LENGTH) != 0) {
                // An addiitonal screen is required to display the `beneficiary` field.
                msg->numScreens += 1;
            }
        if (!ADDRESS_IS_ETH(context->contract_address_sent)) {
            // Address is not ETH so we will need to look up the token in the CAL.
            msg->tokenLookup1 = context->contract_address_sent;
            PRINTF("Setting address sent to: %.*H\n",
                   ADDRESS_LENGTH,
                   context->contract_address_sent);

            // The user is not swapping ETH, so make sure there's no ETH being sent in this tx.
            if (!allzeroes(msg->pluginSharedRO->txContent->value.value,
                           msg->pluginSharedRO->txContent->value.length)) {
                PRINTF("ETH attached to tx when token being swapped is %.*H\n",
                       sizeof(context->contract_address_sent),
                       context->contract_address_sent);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
            }
        } else {
            msg->tokenLookup1 = NULL;
        }
        if (!ADDRESS_IS_ETH(context->contract_address_received)) {
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

    if (ADDRESS_IS_ETH(context->contract_address_sent)) {
        context->decimals_sent = WEI_TO_ETHER;
        strlcpy(context->ticker_sent, "ETH", sizeof(context->ticker_sent));
        context->tokens_found |= TOKEN_SENT_FOUND;
    } else if (msg->token1 != NULL) {
        context->decimals_sent = msg->token1->decimals;
        strlcpy(context->ticker_sent, (char *) msg->token1->ticker, sizeof(context->ticker_sent));
        context->tokens_found |= TOKEN_SENT_FOUND;
    } else {
        // CAL did not find the token and token is not ETH.
        context->decimals_sent = DEFAULT_DECIMAL;
        strlcpy(context->ticker_sent, DEFAULT_TICKER, sizeof(context->ticker_sent));
        // We will need an additional screen to display a warning message.
        msg->additionalScreens++;
    }

    if (ADDRESS_IS_ETH(context->contract_address_received)) {
        context->decimals_received = WEI_TO_ETHER;
        strlcpy(context->ticker_received, "ETH", sizeof(context->ticker_received));
        context->tokens_found |= TOKEN_RECEIVED_FOUND;
    } else if (msg->token2 != NULL) {
        context->decimals_received = msg->token2->decimals;
        strlcpy(context->ticker_received,
                (char *) msg->token2->ticker,
                sizeof(context->ticker_received));
        context->tokens_found |= TOKEN_RECEIVED_FOUND;
    } else {
        // CAL did not find the token and token is not ETH.
        context->decimals_received = DEFAULT_DECIMAL;
        strlcpy(context->ticker_received, DEFAULT_TICKER, sizeof(context->ticker_sent));
        // We will need an additional screen to display a warning message.
        msg->additionalScreens++;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}

static void handle_query_contract_id(void *parameters) {
    ethQueryContractID_t *msg = (ethQueryContractID_t *) parameters;
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;

    strlcpy(msg->name, PLUGIN_NAME, msg->nameLength);

    switch (context->selectorIndex) {
        case MEGA_SWAP:
        case MULTI_SWAP:
        case SIMPLE_SWAP:
        case SWAP_ON_UNI_FORK:
        case SWAP_ON_UNI:
            strlcpy(msg->version, "Swap", msg->versionLength);
            break;
        case SIMPLE_BUY:
        case BUY_ON_UNI_FORK:
        case BUY_ON_UNI:
        case BUY:
            strlcpy(msg->version, "Buy", msg->versionLength);
            break;
        default:
            PRINTF("Selector Index :%d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
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
