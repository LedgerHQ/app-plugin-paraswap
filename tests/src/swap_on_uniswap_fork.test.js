import "core-js/stable";
import "regenerator-runtime/runtime";
import Eth from "@ledgerhq/hw-app-eth";
import { byContractAddress } from "@ledgerhq/hw-app-eth/erc20";
import Zemu from "@zondax/zemu";
import { TransportStatusError } from "@ledgerhq/errors";

const {NANOS_ETH_ELF_PATH, NANOX_ETH_ELF_PATH, NANOS_PARASWAP_LIB, NANOX_PARASWAP_LIB, sim_options_nanos, sim_options_nanox, TIMEOUT} = require("generic.js");

test("Test SimpleSwap", async () => {
  jest.setTimeout(100000);
  const sim = new Zemu(NANOS_ETH_ELF_PATH, NANOS_PARASWAP_LIB);
  try {
    await sim.start(sim_options_nanos);

    let transport = await sim.getTransport();
    const eth = new Eth(transport);

    // Original TX: https://etherscan.io/tx/0x30f033eae4ef9f426b934b7b494de6ca839cd45f6fc3390690ff894fa6519ec4
    // Send ALCX 0.795106864969129095
    // Receive USDC 1212.688661 (user got 1523.52118)
    // Max Fees: 0.011521299
      let tx = eth.signTransaction(
        "44'/60'/0'/0/0",
        "f9016d8202ce850c570bd20083035127941bd435f3c054b6e901b7b108a0ab7617c808677b80b901440863b7ac000000000000000000000000c0aee478e3658e2610c5f7a4a2e1777ce9e4f2ace18a34eb0e04b04f7a0ac29a6e80748dca96319b42c54d679cb821dca90c63030000000000000000000000000000000000000000000000000b08c97b24f59c87000000000000000000000000000000000000000000000000000000004848291500000000000000000000000000000000000000000000000000000000000000c000000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000003000000000000000000000000dbdb4d16eda451d0503b854cf79d55697f90c8df000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc2000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb48018080"
      );

      await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await sim.clickRight();
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
