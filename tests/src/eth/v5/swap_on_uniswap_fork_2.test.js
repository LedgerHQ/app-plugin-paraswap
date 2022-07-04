import { processTest } from "../../test.fixture";

const contractName = "Paraswap V5";
// From : https://etherscan.io/tx/0x82d79ba7c509772f07fc8ae13c0777b1ea025d227561027cca8321cf00d227e5
const rawTxHex =
  "0x02f90158016a8459682f008509ad2b460083027d5994def171fe48cf0115b1d80b88dc8eab59176fee5787470de4df820000b8e40b86a4c1000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee00000000000000000000000000000000000000000000000000470de4df82000000000000000000000000000000000000000000000000000000000000016c8048000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc200000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001000000000000000000004de520e95253e54490d8d30ea41574b24f741ee70201c001a0c6776980d7b73a9383fe9c92effdc3a146e448012ea4a3134c963d1e6bee04b1a0522efffeb53cbc4c99fbcf049fbba20da9584660a897122626f2d08b7951b9d5";
const testLabel = "Swap On Uniswap Fork"; // <= Name of the test
const testDirSuffix = "swap_on_uniswap_fork_2"; // <= directory to compare device snapshots to
const testNetwork = "ethereum";
const signedPlugin = false;

const devices = [
  {
    name: "nanos",
    label: "Nano S",
    steps: 7 // <= Define the number of steps for this test case and this device
  },
  {
    name: "nanox",
    label: "Nano X",
    steps: 6 // <= Define the number of steps for this test case and this device
  }
];

devices.forEach((device) =>
  processTest(device, contractName, testLabel, testDirSuffix, rawTxHex, signedPlugin,"",testNetwork)
);
