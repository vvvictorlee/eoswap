/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/


import { SafeMath } from "./SafeMath";
import "../utils/number.extensions";
/**
 * @title DecimalMath
 * @author DODO Breeder
 *
 * @notice s for fixed point number with 18 decimals
 */
export class DecimalMath {

    static ONE: number = Math.pow(10, 4);

    static mul(target: number, d: number) {
        return target.mul(d) / DecimalMath.ONE;
    }

    static mulCeil(target: number, d: number) {
        return target.mul(d).divCeil(DecimalMath.ONE);
    }

    static divFloor(target: number, d: number) {
        return target.mul(DecimalMath.ONE).div(d);
    }

    static divCeil(target: number, d: number) {
        // return target.mul(DecimalMath.ONE).divCeil(d);
return target.mul(DecimalMath.ONE).div(d);
    }
}

// export default DecimalMath;