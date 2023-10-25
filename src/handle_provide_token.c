#include "paraswap_plugin.h"

void handle_provide_token(ethPluginProvideInfo_t *msg) {
    paraswap_parameters_t *context = (paraswap_parameters_t *) msg->pluginContext;
    PRINTF("PARASWAP plugin provide token: 0x%p, 0x%p\n", msg->item1, msg->item2);

    if (ADDRESS_IS_NETWORK_TOKEN(context->contract_address_sent)) {
        context->decimals_sent = WEI_TO_ETHER;
        context->tokens_found |= TOKEN_SENT_FOUND;
    } else if (msg->item1 != NULL) {
        context->decimals_sent = msg->item1->token.decimals;
        strlcpy(context->ticker_sent,
                (char *) msg->item1->token.ticker,
                sizeof(context->ticker_sent));
        context->tokens_found |= TOKEN_SENT_FOUND;
    } else {
        // CAL did not find the token and token is not ETH.
        context->decimals_sent = DEFAULT_DECIMAL;
        // We will need an additional screen to display a warning message.
        msg->additionalScreens++;
    }

    if (ADDRESS_IS_NETWORK_TOKEN(context->contract_address_received)) {
        context->decimals_received = WEI_TO_ETHER;
        context->tokens_found |= TOKEN_RECEIVED_FOUND;
    } else if (msg->item2 != NULL) {
        context->decimals_received = msg->item2->token.decimals;
        strlcpy(context->ticker_received,
                (char *) msg->item2->token.ticker,
                sizeof(context->ticker_received));
        context->tokens_found |= TOKEN_RECEIVED_FOUND;
    } else {
        // CAL did not find the token and token is not ETH.
        context->decimals_received = DEFAULT_DECIMAL;
        // We will need an additional screen to display a warning message.
        msg->additionalScreens++;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}
