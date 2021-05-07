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

    // These lines  are guessed and applied automatically by custom ledger.js (uncomment when using regular ledger.js)
    // await eth.setExternalPlugin("Paraswap", "0xf650c3d88d12db855b8bf7d11be6c55a4e07dcc9", "0xa0712d68");
    // const erc20Info = byContractAddress("0xf650c3d88d12db855b8bf7d11be6c55a4e07dcc9");
    // await eth.provideERC20TokenInformation(erc20Info);

    await expect(
      eth.signTransaction(
        "44'/60'/0'/0/0",
        "f9019316850af16b160083034f87941bd435f3c054b6e901b7b108a0ab7617c808677b880de0b6b3a7640000b901240863b7ac000000000000000000000000c0aee478e3658e2610c5f7a4a2e1777ce9e4f2ace18a34eb0e04b04f7a0ac29a6e80748dca96319b42c54d679cb821dca90c63030000000000000000000000000000000000000000000000000de0b6b3a764000000000000000000000000000000000000000000000000000000000000cc1cc13a00000000000000000000000000000000000000000000000000000000000000c000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000002000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb4826a08fa697095214dc755211da7a2414ef6960ebc86dfeebe490e20b42fd1f71ee61a05451b911a6b7dd20b1ad7e450b5e3978c2aec5cb044cc8a1ffa7917feb2597df"
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
