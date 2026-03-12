import { processTest, populateTransaction } from "../../test.fixture";

const contractName = "Augustus V6.2";

const testLabel = "Swap Exact Amount Out Pro"; // <= Name of the test
const testDirSuffix = "swap_exact_amount_out_pro"; // <= directory to compare device snapshots to
const testNetwork = "ethereum";
const signedPlugin = false;

const contractAddr = "0x6a000f20005980200259b80c5102003040001068";
const chainID = 1;

// Manually crafted calldata for swapExactAmountOutPro
// swapExactAmountOutPro(address executor, GenericData swapData, uint256 partnerAndFee, bytes permit, bytes executorData)
// GenericData: (address srcToken, address destToken, uint256 fromAmount, uint256 toAmount, uint256 quotedAmount, bytes32 metadata, address beneficiary)
// For swapExactAmountOut: fromAmount = maxAmountIn, toAmount = exact amountOut
const inputData =
  "0x44224add" + // selector
  "000000000000000000000000000000000022d473030f116ddee9f6b43ac78ba3" + // executor
  "000000000000000000000000a0b86991c6218b36c1d19d4a2e9eb0ce3606eb48" + // srcToken (USDC)
  "000000000000000000000000eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee" + // destToken (ETH)
  "0000000000000000000000000000000000000000000000000000000041314cf0" + // fromAmount (1100 USDC = 1100000000 = 0x41314cf0)
  "0000000000000000000000000000000000000000000000000de0b6b3a7640000" + // toAmount (1 ETH)
  "000000000000000000000000000000000000000000000000000000003e95ba80" + // quotedAmount (1050 USDC = 1050000000 = 0x3e95ba80)
  "0000000000000000000000000000000000000000000000000000000000000000" + // metadata
  "000000000000000000000000d8da6bf26964af9d7eed9e03e53415d37aa96045" + // beneficiary
  "0000000000000000000000000000000000000000000000000000000000000000" + // partnerAndFee
  "0000000000000000000000000000000000000000000000000000000000000160" + // offset of permit (11*32=352=0x160)
  "0000000000000000000000000000000000000000000000000000000000000180" + // offset of executorData (352+32=384=0x180)
  "0000000000000000000000000000000000000000000000000000000000000000" + // permit length (0)
  "0000000000000000000000000000000000000000000000000000000000000000"; // executorData length (0)

// Create serializedTx and remove the "0x" prefix
const serializedTx = populateTransaction(contractAddr, inputData, chainID);

const devices = [
  {
    name: "nanosp",
    label: "Nano S+",
    steps: 7 // <= Define the number of steps for this test case and this device
  },
  {
    name: "nanox",
    label: "Nano X",
    steps: 7 // <= Define the number of steps for this test case and this device
  },
  {
    name: "stax",
    label: "Stax",
  },
  {
    name: "flex",
    label: "Flex",
  }
];

devices.forEach((device) =>
  processTest(device, contractName, testLabel, testDirSuffix, "", signedPlugin, serializedTx, testNetwork)
);
