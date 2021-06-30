#include "paraswap_plugin.h"

// Store the amount sent in the form of a string, without any ticker or decimals. These will be
// added when displaying.
static void handle_amount_sent(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    memset(context->amount_sent, 0, sizeof(context->amount_sent));

    // Convert to string.
    amountToString(msg->parameter,
                   PARAMETER_LENGTH,
                   0,
                   "",
                   (char *) context->amount_sent,
                   sizeof(context->amount_sent));
    PRINTF("AMOUNT SENT: %s\n", context->amount_sent);
}

// Store the amount received in the form of a string, without any ticker or decimals. These will be
// added when displaying.
static void handle_amount_received(ethPluginProvideParameter_t *msg,
                                   paraswap_parameters_t *context) {
    memset(context->amount_received, 0, sizeof(context->amount_received));

    // Convert to string.
    amountToString(msg->parameter,
                   PARAMETER_LENGTH,
                   0,   // No decimals
                   "",  // No ticker
                   (char *) context->amount_received,
                   sizeof(context->amount_received));
    PRINTF("AMOUNT RECEIVED: %s\n", context->amount_received);
}

static void handle_beneficiary(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    memset(context->beneficiary, 0, sizeof(context->beneficiary));
    memcpy(context->beneficiary,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->beneficiary));
    PRINTF("BENEFICIARY: %.*H\n", ADDRESS_LENGTH, context->beneficiary);
}

static void handle_array_len(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    context->array_len = msg->parameter[PARAMETER_LENGTH - 1];
    PRINTF("LIST LEN: %d\n", context->array_len);
}

static void handle_token_sent(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    memset(context->contract_address_sent, 0, sizeof(context->contract_address_sent));
    memcpy(context->contract_address_sent,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->contract_address_sent));
    PRINTF("TOKEN SENT: %.*H\n", ADDRESS_LENGTH, context->contract_address_sent);
}

static void handle_token_received(ethPluginProvideParameter_t *msg,
                                  paraswap_parameters_t *context) {
    memset(context->contract_address_received, 0, sizeof(context->contract_address_received));
    memcpy(context->contract_address_received,
           &msg->parameter[PARAMETER_LENGTH - ADDRESS_LENGTH],
           sizeof(context->contract_address_received));
    PRINTF("TOKEN RECIEVED: %.*H\n", ADDRESS_LENGTH, context->contract_address_received);
}

static void handle_uniswap_and_forks(ethPluginProvideParameter_t *msg,
                                     paraswap_parameters_t *context) {
    switch (context->next_param) {
        case AMOUNT_SENT:  // amountIn
            handle_amount_sent(msg, context);
            context->next_param = AMOUNT_RECEIVED;
            context->checkpoint = msg->parameterOffset;
            if (context->selectorIndex == BUY_ON_UNI_FORK ||
                context->selectorIndex == SWAP_ON_UNI_FORK) {
                // Substract two chunks because we've skipped the first two parameters.
                // No underflow possible because we've skipped the first two chunks, so
                // msg->parameterOffset >= 2 * PARAMETER_LENGTH.
                context->checkpoint -= 2 * PARAMETER_LENGTH;
            }
            break;
        case AMOUNT_RECEIVED:  // amountOut
            handle_amount_received(msg, context);
            context->next_param = PATHS_OFFSET;
            break;
        case PATHS_OFFSET:
            context->offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = PATH;
            break;
        case PATH:  // len(path)
            handle_array_len(msg, context);
            context->next_param = TOKEN_SENT;
            break;
        case TOKEN_SENT:  // path[0]
            handle_token_sent(msg, context);
            // -2 because we won't be skipping the first one and the last one.
            context->skip = context->array_len - 2;
            context->next_param = TOKEN_RECEIVED;
            break;
        case TOKEN_RECEIVED:  // path[len(path) - 1]
            handle_token_received(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Unsupported param\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_simple_calls(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    switch (context->next_param) {
        case TOKEN_SENT:  // fromToken
            handle_token_sent(msg, context);
            context->next_param = TOKEN_RECEIVED;
            break;
        case TOKEN_RECEIVED:  // toToken
            handle_token_received(msg, context);
            context->next_param = AMOUNT_SENT;
            break;
        case AMOUNT_SENT:  // fromAmount
            handle_amount_sent(msg, context);
            context->next_param = AMOUNT_RECEIVED;
            break;
        case AMOUNT_RECEIVED:  // toAmount
            handle_amount_received(msg, context);
            context->next_param = BENEFICIARY;
            context->skip = 4;  // callees, exchangeData, startIndexes, values.
            if (context->selectorIndex == SIMPLE_SWAP) {
                context->skip++;  // skip field expectedAmount for simple swap.
            }
            break;
        case BENEFICIARY:
            handle_beneficiary(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Param not supported\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_multiswap(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    switch (context->next_param) {
        case TOKEN_SENT:  // fromToken
            context->checkpoint = msg->parameterOffset;
            handle_token_sent(msg, context);
            context->next_param = AMOUNT_SENT;
            break;
        case AMOUNT_SENT:  // fromAmount
            handle_amount_sent(msg, context);
            context->next_param = AMOUNT_RECEIVED;
            break;
        case AMOUNT_RECEIVED:  // toAmount
            handle_amount_received(msg, context);
            context->next_param = BENEFICIARY;
            context->skip = 1;  // Skip expectedAmount
            break;
        case BENEFICIARY:  // beneficiary
            handle_beneficiary(msg, context);
            context->next_param = PATHS_OFFSET;
            context->skip = 2;  // Skip referrer and useReduxtoken
            break;
        case PATHS_OFFSET:
            context->offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = PATHS_LEN;
            break;
        case PATHS_LEN:
            // We want to access path[-1] so take the length and decrease by one
            context->skip = msg->parameter[PARAMETER_LENGTH - 1] - 1;
            context->next_param = OFFSET;
            context->checkpoint = msg->parameterOffset +
                                  PARAMETER_LENGTH;  // Offset checkpoint starts after the length
            break;
        case OFFSET:
            context->offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = TOKEN_RECEIVED;
            break;
        case TOKEN_RECEIVED:
            handle_token_received(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Param not supported\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_buy(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    switch (context->next_param) {
        case TOKEN_SENT:  // fromToken
            handle_token_sent(msg, context);
            context->next_param = TOKEN_RECEIVED;
            break;
        case TOKEN_RECEIVED:  // toToken
            handle_token_received(msg, context);
            context->next_param = AMOUNT_SENT;
            break;
        case AMOUNT_SENT:  // fromAmount
            handle_amount_sent(msg, context);
            context->next_param = AMOUNT_RECEIVED;
            break;
        case AMOUNT_RECEIVED:  // toAmount
            handle_amount_received(msg, context);
            context->next_param = BENEFICIARY;
            break;
        case BENEFICIARY:  // beneficiary
            handle_beneficiary(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Param not supported\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_megaswap(ethPluginProvideParameter_t *msg, paraswap_parameters_t *context) {
    switch (context->next_param) {
        case TOKEN_SENT:  // fromToken
            context->checkpoint = msg->parameterOffset;
            handle_token_sent(msg, context);
            context->next_param = AMOUNT_SENT;
            break;
        case AMOUNT_SENT:
            handle_amount_sent(msg, context);
            context->next_param = AMOUNT_RECEIVED;
            break;
        case AMOUNT_RECEIVED:
            handle_amount_received(msg, context);
            context->next_param = BENEFICIARY;
            context->skip = 1;  // Skip expectedAmount.
            break;
        case BENEFICIARY:
            handle_beneficiary(msg, context);
            context->next_param = MEGA_PATHS_OFFSET;
            context->skip = 2;  // Skip referrer and useReduxToken.
            break;
        case MEGA_PATHS_OFFSET:
            context->offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = MEGA_PATHS_LEN;
            break;
        case MEGA_PATHS_LEN:
            context->next_param = FIRST_MEGAPATH_OFFSET;
            break;
        case FIRST_MEGAPATH_OFFSET:
            context->checkpoint = msg->parameterOffset;
            context->offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = FIRST_MEGAPATH;
            break;
        case FIRST_MEGAPATH:
            context->checkpoint = msg->parameterOffset;
            context->next_param = PATHS_OFFSET;
            break;
        case PATHS_OFFSET:
            context->offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = PATHS_LEN;
            break;
        case PATHS_LEN:
            context->skip = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->skip--;  // Decrease by one because we wish to acces path[-1].
            context->next_param = PATH;
            break;
        case PATH:
            context->checkpoint = msg->parameterOffset;
            context->offset = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = TOKEN_RECEIVED;
            break;
        case TOKEN_RECEIVED:
            handle_token_received(msg, context);
            context->next_param = NONE;
            break;
        case NONE:
            break;
        default:
            PRINTF("Param not supported\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

void handle_provide_parameter(void *parameters) {
    ethPluginProvideParameter_t *msg = (ethPluginProvideParameter_t *) parameters;
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;

    PRINTF("plugin provide parameter %d %.*H\n",
           msg->parameterOffset,
           PARAMETER_LENGTH,
           msg->parameter);
    print_bytes(msg->parameter, PARAMETER_LENGTH);

    msg->result = ETH_PLUGIN_RESULT_OK;

    if (context->skip) {
        // Skip this step, and don't forget to decrease skipping counter.
        context->skip--;
    } else {
        if ((context->offset) && msg->parameterOffset != context->checkpoint + context->offset) {
            PRINTF("offset: %d, checkpoint: %d, parameterOffset: %d\n",
                   context->offset,
                   context->checkpoint,
                   msg->parameterOffset);
            return;
        }

        context->offset = 0;  // Reset offset
        switch (context->selectorIndex) {
            case BUY_ON_UNI_FORK:
            case SWAP_ON_UNI_FORK:
            case BUY_ON_UNI:
            case SWAP_ON_UNI: {
                handle_uniswap_and_forks(msg, context);
                break;
            }

            case SIMPLE_BUY:
            case SIMPLE_SWAP: {
                handle_simple_calls(msg, context);
                break;
            }

            case MULTI_SWAP: {
                handle_multiswap(msg, context);
                break;
            }

            case BUY: {
                handle_buy(msg, context);
                break;
            }

            case MEGA_SWAP: {
                handle_megaswap(msg, context);
                break;
            }

            default:
                PRINTF("Selector Index %d not supported\n", context->selectorIndex);
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                break;
        }
    }
}