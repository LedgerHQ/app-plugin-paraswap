import "core-js/stable";
import "regenerator-runtime/runtime";
import Eth from "@ledgerhq/hw-app-eth";
import { byContractAddress } from "@ledgerhq/hw-app-eth/erc20";
import Zemu from "@zondax/zemu";
import { TransportStatusError } from "@ledgerhq/errors";

const sim_options = {
  model: "nanos",
  logging: true,
  start_delay: 2000,
  custom: '-k 2.0',
  X11: true,
};
const Resolve = require("path").resolve;
const APP_PATH = Resolve("elfs/ethereum.elf");

const PARASWAP_LIB = { Paraswap: Resolve("elfs/paraswap.elf") };

test("Test SimpleSwap", async () => {
  jest.setTimeout(100000);
  const sim = new Zemu(APP_PATH, PARASWAP_LIB);
  try {
    await sim.start(sim_options);

    let transport = await sim.getTransport();
    const eth = new Eth(transport);

    // Send a special APDU to prepare CAL for our call.
    await eth.setExternalPlugin("Paraswap", "0x1bd435f3c054b6e901b7b108a0ab7617c808677b", "0x0863b7ac");

    // Original TX: https://etherscan.io/tx/0x30f033eae4ef9f426b934b7b494de6ca839cd45f6fc3390690ff894fa6519ec4
    await expect(
      eth.signTransaction(
        "44'/60'/0'/0/0",
        "f901ad8202ce850c570bd20083035127941bd435f3c054b6e901b7b108a0ab7617c808677b80b901440863b7ac000000000000000000000000c0aee478e3658e2610c5f7a4a2e1777ce9e4f2ace18a34eb0e04b04f7a0ac29a6e80748dca96319b42c54d679cb821dca90c63030000000000000000000000000000000000000000000000000b08c97b24f59c87000000000000000000000000000000000000000000000000000000004848291500000000000000000000000000000000000000000000000000000000000000c000000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000003000000000000000000000000dbdb4d16eda451d0503b854cf79d55697f90c8df000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc2000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb4826a0649856a7c253e327433f667c4816f3eb616f57c74bd95fe2f7c70b5b51d4e1e1a023227729bf28bfaa85706e738447f3c953ea6f07a6869f616d0a898ff4ca99db"
      )
    ).resolves.toEqual({
      r: "e8c2b5b956b34386e68c5cc5bfc76aac158706430fcf1c71c9f6eb7d9dd690c8",
      s: "064005410a778b047fdd4afcb1e712217c297e5b0a2190f88b9e56ac745fae69",
      v: "26",
    });
  } finally {
    await sim.close();
  }
});
