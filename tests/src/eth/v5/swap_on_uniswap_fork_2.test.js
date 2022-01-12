import { processTest } from "../../test.fixture";

const contractName = "Paraswap V5";
// From : https://etherscan.io/tx/0xe7b7b2683b300ba8a096e5ebab88eae13d0e1860ea6e44a439f1a6c454a1c970
const rawTxHex =
  "0x02f90192017b847270e000853cc39ec500830330fa94def171fe48cf0115b1d80b88dc8eab59176fee5780b90124f5661034000000000000000000000000c0aee478e3658e2610c5f7a4a2e1777ce9e4f2ace18a34eb0e04b04f7a0ac29a6e80748dca96319b42c54d679cb821dca90c630300000000000000000000000000000000000000000000000029a2281e4a09a277000000000000000000000000000000000000000000000000000000011767928700000000000000000000000000000000000000000000000000000000000000a000000000000000000000000000000000000000000000000000000000000000030000000000000000000000001ceb5cb57c4d4e2b2433641b95dd330a33185a44000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc2000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb48c001a0731af59ef4168c9d4478ab7b45569cdd991b7b1c7377376f0ecfb1332f9b79e8a07c37d645210ced2bd80be3d16e97f1a40b35139916cb38e262f883d9fcd816cc";
const testLabel = "Swap On Uniswap Fork"; // <= Name of the test
const testDirSuffix = "swap_on_uniswap_fork_2"; // <= directory to compare device snapshots to
const testNetwork = "ethereum"; 
const signedPlugin = false;

const devices = [
  {
    name: "nanos",
    label: "Nano S",
    steps: 7, // <= Define the number of steps for this test case and this device
  },
  // {
  //   name: "nanox",
  //   label: "Nano X",
  //   steps: 5, // <= Define the number of steps for this test case and this device
  // },
];

devices.forEach((device) =>
  processTest(device, contractName, testLabel, testDirSuffix, rawTxHex, signedPlugin,"",testNetwork)
);
