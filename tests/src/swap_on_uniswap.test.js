import "core-js/stable";
import "regenerator-runtime/runtime";
import Eth from "@ledgerhq/hw-app-eth";
import { byContractAddress } from "@ledgerhq/hw-app-eth/erc20";
import Zemu from "@zondax/zemu";
import { TransportStatusError } from "@ledgerhq/errors";

const {NANOS_ETH_ELF_PATH, NANOX_ETH_ELF_PATH, NANOS_PARASWAP_LIB, NANOX_PARASWAP_LIB, sim_options_nanos, sim_options_nanox, TIMEOUT} = require("generic.js");

test("Test Swap on Uniswap", async () => {
  jest.setTimeout(100000);
  const sim = new Zemu(NANOS_ETH_ELF_PATH, NANOS_PARASWAP_LIB);
  try {
    await sim.start(sim_options_nanos);

    let transport = await sim.getTransport();
    const eth = new Eth(transport);

    // Original TX: https://etherscan.io/tx/0x6b6aeef79fc7e295109fd6dd6f614753e30996a8ac1d5c15269c440c4c768df5
    // Send 145 API3
    // Receive 0.283184134935736098
    // Fees 0.008769654
    let tx = eth.signTransaction(
        "44'/60'/0'/0/0",
        "f9010a30850bdfd63e0083029fb2941bd435f3c054b6e901b7b108a0ab7617c808677b80b8e458b9d179000000000000000000000000000000000000000000000007dc477bc1cfa4000000000000000000000000000000000000000000000000000003ee127868deef220000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000020000000000000000000000000b38210ea11411557c13457d4da7dc6ea731b88a000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee018080"
      );

    await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot(), 200000);
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickRight();
    await sim.clickBoth();

    await tx;
  } finally {
    await sim.close();
  }
});
