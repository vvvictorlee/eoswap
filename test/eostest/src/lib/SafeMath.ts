const Decimal = require('decimal.js');
// const num=111111;
// console.log(num.thousandsSeperator() )



/**
 * @title SafeMath
 * @author DODO Breeder
 *
 * @notice Math operations with safety checks that revert on error
 */
export class SafeMath {
    // value: number;
    // constructor(a: number) { this.value = a; }
    // mul(b: number): number {
    //     return this.imul(this.value, b);
    // }
    // div(b: number): number {
    //     return this.idiv(this.value, b);
    // }
    // divCeil(b: number): number {
    //     return this.idivCeil(this.value, b);
    // }
    // sub(b: number): number {
    //     return this.isub(this.value, b);
    // }
    // add(b: number): number {
    //     return this.iadd(this.value, b);
    // }
    // sqrt(): number {
    //     return this.isqrt(this.value);
    // }

    static mul(a: number, b: number) {
        if (a == 0) {
            return 0;
        }

        let c: number = a * b;
        // //require(c / a == b, "MUL_ERROR");

        return c;
    }

    static div(a: number, b: number) {
        //require(b > 0, "DIVIDING_ERROR");
        // return a / b;
        return Decimal(a).div(b);
    }

    static divCeil(a: number, b: number) {
        let quotient: number = SafeMath.div(a, b);
        let remainder: number = a - quotient * b;
        if (remainder > 0) {
            return quotient + 1;
        } else {
            return quotient;
        }
    }

    static sub(a: number, b: number) {
        //require(b <= a, "SUB_ERROR");
        return a - b;
    }

    static add(a: number, b: number) {
        let c: number = a + b;
        //require(c >= a, "ADD_ERROR");
        return c;
    }

    static sqrt(x: number) {
        let z: number = x / 2 + 1;
        let y: number = x;
        while (z < y) {
            y = z;
            z = (x / z + z) / 2;
        }

        return y;
    }
}
