import { processTest } from "../../test.fixture";

const contractName = "Paraswap V5";
// From : https://etherscan.io/tx/0x4ae6d1fe1d298a6f84eb710cf5fb3107f5646ed3dbbe04a89d869de4e2a6888a
const rawTxHex =
  "0x02f90133018203998459682f008534b978c8808302a29e94def171fe48cf0115b1d80b88dc8eab59176fee5780b8c454840d1a0000000000000000000000000000000000000000000000022d2a0fb6d30a34c70000000000000000000000000000000000000000000000000861580ca47330ff00000000000000000000000000000000000000000000000000000000000000600000000000000000000000000000000000000000000000000000000000000002000000000000000000000000b6ca7399b4f9ca56fc27cbff44f4d2e4eef1fc81000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeec001a05f6d50a806a94e7b01b5612e300684e4ae0c4b6a3fe7e0a6e46e3155e8474e09a00a6e2bc1a444152c9da455ea5ee47ef587d522d22e6b0ae4a83ccebafe31af7d";
const testLabel = "Swap On Uniswap"; // <= Name of the test
const testDirSuffix = "swap_on_uniswap_2"; // <= directory to compare device snapshots to
const testNetwork = "ethereum";
const signedPlugin = false;

const devices = [
  {
    name: "nanos",
    label: "Nano S",
    steps: 9 // <= Define the number of steps for this test case and this device
  },
  {
    name: "nanox",
    label: "Nano X",
    steps: 5 // <= Define the number of steps for this test case and this device
  }
];

devices.forEach((device) =>
  processTest(device, contractName, testLabel, testDirSuffix, rawTxHex, signedPlugin,"",testNetwork)
);
