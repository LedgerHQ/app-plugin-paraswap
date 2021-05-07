/*******************************************************************************
 *   Ethereum 2 Deposit Application
 *   (c) 2020 Ledger
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
 ********************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "os.h"
#include "cx.h"

#include "glyphs.h"

#include "eth_plugin_interface.h"

#define RUN_APPLICATION 1

#define NUM_PARASWAP_SELECTORS 9
#define SELECTOR_SIZE          4

static const uint8_t const PARASWAP_SWAP_ON_UNISWAP_SELECTOR[SELECTOR_SIZE] = {0x58,
                                                                               0xb9,
                                                                               0xd1,
                                                                               0x79};
static const uint8_t const PARASWAP_SWAP_ON_UNISWAP_FORK_SELECTOR[SELECTOR_SIZE] = {0x08,
                                                                                    0x63,
                                                                                    0xb7,
                                                                                    0xac};
static const uint8_t const PARASWAP_SIMPLE_SWAP_SELECTOR[SELECTOR_SIZE] = {0xcf, 0xc0, 0xaf, 0xeb};
static const uint8_t const PARASWAP_MULTI_SWAP_SELECTOR[SELECTOR_SIZE] = {0x8f, 0x00, 0xec, 0xcb};
static const uint8_t const PARASWAP_MEGA_SWAP_SELECTOR[SELECTOR_SIZE] = {0xec, 0x1d, 0x21, 0xdd};
static const uint8_t const PARASWAP_BUY_ON_UNISWAP_SELECTOR[SELECTOR_SIZE] = {0xf9,
                                                                              0x35,
                                                                              0x5f,
                                                                              0x72};
static const uint8_t const PARASWAP_BUY_ON_UNISWAP_FORK_SELECTOR[SELECTOR_SIZE] = {0x33,
                                                                                   0x63,
                                                                                   0x52,
                                                                                   0x26};
static const uint8_t const PARASWAP_SIMPLE_BUY_SELECTOR[SELECTOR_SIZE] = {0xa2, 0x7e, 0x8b, 0x6b};
static const uint8_t const PARASWAP_BUY_SELECTOR[SELECTOR_SIZE] = {0xf9, 0x5a, 0x49, 0xeb};

const uint8_t* const PARASWAP_SELECTORS[NUM_PARASWAP_SELECTORS] = {
    PARASWAP_SWAP_ON_UNISWAP_SELECTOR,
    PARASWAP_SWAP_ON_UNISWAP_FORK_SELECTOR,
    PARASWAP_SIMPLE_SWAP_SELECTOR,
    PARASWAP_MULTI_SWAP_SELECTOR,
    PARASWAP_MEGA_SWAP_SELECTOR,
    PARASWAP_BUY_ON_UNISWAP_SELECTOR,
    PARASWAP_BUY_ON_UNISWAP_FORK_SELECTOR,
    PARASWAP_SIMPLE_BUY_SELECTOR,
    PARASWAP_BUY_SELECTOR,
};

// Paraswap uses `0xeeeee` as a dummy address to represent ETH.
const uint8_t PARASWAP_ETH_ADDRESS[ADDRESS_LENGTH] = {0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee,
                                        0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee, 0xee};

const uint8_t NULL_ETH_ADDRESS[ADDRESS_LENGTH] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void call_app_ethereum() {
    unsigned int libcall_params[3];
    libcall_params[0] = "Ethereum";
    libcall_params[1] = 0x100;
    libcall_params[2] = RUN_APPLICATION;
    os_lib_call(&libcall_params);
}

__attribute__((section(".boot"))) int main(int arg0) {
    // exit critical section
    __asm volatile("cpsie i");

    // ensure exception will work as planned
    os_boot();

    BEGIN_TRY {
        TRY {
            check_api_level(CX_COMPAT_APILEVEL);

            if (!arg0) {
                // called from dashboard, launch Ethereum app
                call_app_ethereum();
                return 0;
            } else {
                // regular call from ethereum
                unsigned int *args = (unsigned int *) arg0;

                if (args[0] != ETH_PLUGIN_CHECK_PRESENCE) {
                    paraswap_plugin_call(args[0], (void *) args[1]);
                }
                os_lib_end();
            }
        }
        FINALLY {
        }
    }
    END_TRY;

    return 0;
}
