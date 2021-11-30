import { processTest } from "../test.fixture";

const contractName = "Paraswap V5";
// From : https://etherscan.io/tx/0xe592df06975a1161a4582ab1663382457b0ea6619d5640f410096afd92643bf2
const rawTxHex =
  "0x02f9065d018230868503b9aca0008521bd3cdabe83054b7494def171fe48cf0115b1d80b88dc8eab59176fee57884b6eb9c3a9b52b63b905e42298207a0000000000000000000000000000000000000000000000000000000000000020000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee000000000000000000000000467bccd9d29f223bce8043b84e8c8b282827790f0000000000000000000000000000000000000000000000004b6eb9c3a9b52b630000000000000000000000000000000000000000000000000000000005c718d90000000000000000000000000000000000000000000000004aaf87c442a42af600000000000000000000000000000000000000000000000000000000000001e000000000000000000000000000000000000000000000000000000000000002600000000000000000000000000000000000000000000000000000000000000480000000000000000000000000000000000000000000000000000000000000052000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000005a000000000000000000000000000000000000000000000000000000000616b8659447c5ba02ebd11ec9c2e59c773c2c810000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000003000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc2000000000000000000000000f9234cb08edb93c0d4a4d4c70cc3ffd070e78e07000000000000000000000000e592427a0aece92de3edee1f18e0157c0586156400000000000000000000000000000000000000000000000000000000000001ecd0e30db03f1c8c53000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee00000000000000000000000000000000000000000000000015135a9399c818d300000000000000000000000000000000000000000000000000000000019e25ad000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc200000000000000000000000000000000000000000000000000000000000000a00000000000000000000000000000000000000000000000000000000000000001000000000000000000004de5291c69fdaebd3cbe953843da243f8605a766a268db3e2198000000000000000000000000c02aaa39b223fe8d0a0e5c4f27ead9083c756cc2000000000000000000000000467bccd9d29f223bce8043b84e8c8b282827790f0000000000000000000000000000000000000000000000000000000000000bb8000000000000000000000000def171fe48cf0115b1d80b88dc8eab59176fee5700000000000000000000000000000000000000000000000000000000616b4009000000000000000000000000000000000000000000000000000000000428f32c000000000000000000000000000000000000000000000000365b5f300fed128d0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000040000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000400000000000000000000000000000000000000000000000000000000000000e800000000000000000000000000000000000000000000000000000000000001ec000000000000000000000000000000000000000000000000000000000000000300000000000000000000000000000000000000000000000035d1988ad578aa7300000000000000000000000000000000000000000000000015135a9399c818d300000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000c080a0f34b1bc1a7131bdf5c562e784e906e0f705a049811dbaf9418352e737b3da421a0063c085bb909fb95f795fbf5f81723e80b0e846740157a793e14ab80e03500dd";
const testLabel = "Simple Buy"; // <= Name of the test
const testDirSuffix = "simple_buy"; // <= directory to compare device snapshots to
const signedPlugin = true;

const devices = [
  {
    name: "nanos",
    label: "Nano S",
    steps: 8, // <= Define the number of steps for this test case and this device
  },
  // {
  //   name: "nanox",
  //   label: "Nano X",
  //   steps: 5, // <= Define the number of steps for this test case and this device
  // },
];

devices.forEach((device) =>
  processTest(device, contractName, testLabel, testDirSuffix, rawTxHex, signedPlugin)
);
