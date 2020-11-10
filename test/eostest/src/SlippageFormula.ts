// import './string.extensions';

// import "./number.extensions";

// const l:Number = 1111;
// console.log(l.thousandsSeperator());

// var num: number = 7;
// var str: string = "7";
// console.log("padded number: ", num.padZero(5));
// console.log("padding string: ", str.padZero(5));
// console.log("mul number: ", num.mul(7));


function calculateSlippage(buyPercentage: number) {
  const k = 0.1
  console.log(buyPercentage, ":", ((1 / (1 - buyPercentage)) * k - k) * 100, "%")
}

function calculateLoss(priceGap: number) {
  const feeRate = 0.0025
  const k = 0.1
  let amountPartial = Math.sqrt(priceGap / k + 1) - 1
  let loss = amountPartial * (priceGap - feeRate * 2)
  console.log(priceGap, ":", loss * 100, "%")
}

calculateSlippage(0.01)
// calculateSlippage(0.05)
// calculateSlippage(0.1)
// calculateSlippage(0.2)
// calculateSlippage(0.5)
// calculateSlippage(0.7)

calculateLoss(0.006)
// calculateLoss(0.007)
// calculateLoss(0.008)
// calculateLoss(0.009)
// calculateLoss(0.01)
// calculateLoss(0.02)
// calculateLoss(0.03)

// declare global {
//     interface Number {
//           toPowerOf10: () => string;
//     }
// }

// Number.prototype.toPowerOf10 = function() : string {
//   return this.toExponential();
// }

// var n: Number = 10000;
// document.write(n.toPowerOf10());


