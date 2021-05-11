#pragma once

#include <string.h>
#include "eth_internals.h"
#include "eth_plugin_interface.h"

#define PARAMETER_LENGTH 32
#define SELECTOR_SIZE    4

#define RUN_APPLICATION 1

#define NUM_PARASWAP_SELECTORS 9
#define SELECTOR_SIZE          4

// Paraswap uses `0xeeeee` as a dummy address to represent ETH.
extern const uint8_t PARASWAP_ETH_ADDRESS[ADDRESS_LENGTH];

// Adress 0x00000... used to indicate that the beneficiary is the sender.
extern const uint8_t NULL_ETH_ADDRESS[ADDRESS_LENGTH];

// Returns 1 if corresponding address is the Paraswap address for ETH (0xeeeee...).
#define ADDRESS_IS_ETH(_addr) (!memcmp(_addr, PARASWAP_ETH_ADDRESS, ADDRESS_LENGTH))

typedef enum {
    SWAP_ON_UNI,
    BUY_ON_UNI,
    SWAP_ON_UNI_FORK,
    BUY_ON_UNI_FORK,
    SIMPLE_SWAP,
    SIMPLE_BUY,
    MULTI_SWAP,
    BUY,
    MEGA_SWAP,
} paraswapSelector_t;

typedef enum {
    AMOUNT_SENT,      // Amount sent by the user to the contract.
    AMOUNT_RECEIVED,  // Amount sent by the contract to the user.
    TOKEN_SENT,       // Address of the token the user is sending.
    TOKEN_RECEIVED,   // Address of the token sent to the user.
    PATH,  // Path of the different asseths that will get swapped during the trade. First and last
           // tokens are the ones we care about.
    BENEFICIARY,  // Address to which the contract will send the tokens.
    OFFSET,
    PATHS_OFFSET,
    PATHS_LEN,
    MEGA_PATHS_OFFSET,
    MEGA_PATHS_LEN,
    FIRST_MEGAPATH_OFFSET,
    FIRST_MEGAPATH,
    NONE,  // Placeholder variant to be set when parsing is done but data is still being sent.
} swap_params;

// Shared global memory with Ethereum app. Must be at most 5 * 32 bytes.
typedef struct paraswap_parameters_t {
    uint8_t amount_sent[INT256_LENGTH];
    uint8_t amount_received[INT256_LENGTH];
    char beneficiary[ADDRESS_LENGTH];
    uint8_t contract_address_sent[ADDRESS_LENGTH];
    uint8_t contract_address_received[ADDRESS_LENGTH];
    char ticker_sent[MAX_TICKER_LEN];
    char ticker_received[MAX_TICKER_LEN];

    // 32 * 2 + 20 * 3 + 12 * 2 == 64 + 60 + 24 == 144
    // 32 * 5 == 160 bytes so there are 160 - 144 == 16 bytes left.

    uint16_t offset;
    uint16_t checkpoint;
    swap_params next_param;
    uint8_t valid;
    uint8_t decimals_sent;
    uint8_t decimals_received;
    uint8_t selectorIndex;
    uint8_t array_len;
    uint8_t skip;
    // 2 * 2 + 7 * 1 == 4 + 7 == 13 bytes. There are 16 - 13 == 3 bytes left.
} paraswap_parameters_t;

void debug_write(char *buf);
void handle_provide_parameter(void *parameters);
void paraswap_plugin_call(int message, void *parameters);