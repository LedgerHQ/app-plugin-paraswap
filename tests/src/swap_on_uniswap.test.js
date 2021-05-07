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

test("Test Swap on Uniswap", async () => {
  jest.setTimeout(100000);
  const sim = new Zemu(APP_PATH, PARASWAP_LIB);
  try {
    await sim.start(sim_options);

    let transport = await sim.getTransport();
    const eth = new Eth(transport);

    // Send a special APDU to prepare CAL for our call.
    await eth.setExternalPlugin("Paraswap", "0x1bd435f3c054b6e901b7b108a0ab7617c808677b", "0x58b9d179");

    // Original TX: https://etherscan.io/tx/0x6b6aeef79fc7e295109fd6dd6f614753e30996a8ac1d5c15269c440c4c768df5
    await expect(
      eth.signTransaction(
        "44'/60'/0'/0/0",
        "f9014a30850bdfd63e0083029fb2941bd435f3c054b6e901b7b108a0ab7617c808677b80b8e458b9d179000000000000000000000000000000000000000000000007dc477bc1cfa4000000000000000000000000000000000000000000000000000003ee127868deef220000000000000000000000000000000000000000000000000000000000000080000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000020000000000000000000000000b38210ea11411557c13457d4da7dc6ea731b88a000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee26a022380972064ddd3aef0178dec7f1a2542df6ad8c5d95ccd6e29d6f0e0a3f938ca04343dcc66bf041170803672fd96d6d7b2030c0bb3ecd3069e247475f30edc9ec"
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
