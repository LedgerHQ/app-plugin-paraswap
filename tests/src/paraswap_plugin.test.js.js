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
        "F849208506FC23AC008303DC3194F650C3D88D12DB855B8BF7D11BE6C55A4E07DCC980A4A0712D6800000000000000000000000000000000000000000000000000000000000ACBC7018080"
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
