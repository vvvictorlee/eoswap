/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/



import {SafeMath} from "./SafeMath";
import {DecimalMath} from "./DecimalMath";

/**
 * @title DODOMath
 * @author DODO Breeder
 *
 * @notice s for complex calculating. Including ONE Integration and TWO Quadratic solutions
 */
export class DODOMath {

    /*
        Integrate dodo curve fron V1 to V2
        require V0>=V1>=V2>0
        res = (1-k)i(V1-V2)+ikV0*V0(1/V2-1/V1)
        let V1-V2=delta
        res = i*delta*(1-k+k(V0^2/V1/V2))
    */
    static  _GeneralIntegrate(
        V0:number,
        V1:number,
        V2:number,
        i:number,
        k:number
    )  {
        let fairAmount:number = DecimalMath.mul(i, V1.sub(V2)); // i*delta
        let V0V0V1V2:number = DecimalMath.divCeil(V0.mul(V0).div(V1), V2);
        let penalty:number = DecimalMath.mul(k, V0V0V1V2); // k(V0^2/V1/V2)
        return DecimalMath.mul(fairAmount, DecimalMath.ONE.sub(k).add(penalty));
    }

    /*
        The same with integration expression above, we have:
        i*deltaB = (Q2-Q1)*(1-k+kQ0^2/Q1/Q2)
        Given Q1 and deltaB, solve Q2
        This is a quadratic  and the standard version is
        aQ2^2 + bQ2 + c = 0, where
        a=1-k
        -b=(1-k)Q1-kQ0^2/Q1+i*deltaB
        c=-kQ0^2
        and Q2=(-b+sqrt(b^2+4(1-k)kQ0^2))/2(1-k)
        note: another root is negative, abondan
        if deltaBSig=true, then Q2>Q1
        if deltaBSig=false, then Q2<Q1
    */
    static  _SolveQuadraticForTrade(
        Q0:number,
        Q1:number,
        ideltaB:number,
        deltaBSig:boolean ,
        k:number
    )  {
        // calculate -b value and sig
        // -b = (1-k)Q1-kQ0^2/Q1+i*deltaB
        let kQ02Q1:number = DecimalMath.mul(k, Q0).mul(Q0).div(Q1); // kQ0^2/Q1
        let b:number = DecimalMath.mul(DecimalMath.ONE.sub(k), Q1); // (1-k)Q1
        let minusbSig:boolean = true;
        if (deltaBSig) {
            b = b.add(ideltaB); // (1-k)Q1+i*deltaB
        } else {
            kQ02Q1 = kQ02Q1.add(ideltaB); // i*deltaB+kQ0^2/Q1
        }
        if (b >= kQ02Q1) {
            b = b.sub(kQ02Q1);
            minusbSig = true;
        } else {
            b = kQ02Q1.sub(b);
            minusbSig = false;
        }

        // calculate sqrt
        let squareRoot:number = DecimalMath.mul(
            DecimalMath.ONE.sub(k).mul(4),
            DecimalMath.mul(k, Q0).mul(Q0)
        ); // 4(1-k)kQ0^2
        squareRoot = b.mul(b).add(squareRoot).sqrt(); // sqrt(b*b+4(1-k)kQ0*Q0)

        // final res
        let denominator:number = DecimalMath.ONE.sub(k).mul(2); // 2(1-k)
        let numerator:number;
        if (minusbSig) {
            numerator = b.add(squareRoot);
        } else {
            numerator = squareRoot.sub(b);
        }

        if (deltaBSig) {
            return DecimalMath.divFloor(numerator, denominator);
        } else {
            return DecimalMath.divCeil(numerator, denominator);
        }
    }

    /*
        Start from the integration 
        i*deltaB = (Q2-Q1)*(1-k+kQ0^2/Q1/Q2)
        Assume Q2=Q0, Given Q1 and deltaB, solve Q0
        let fairAmount = i*deltaB
    */
    static  _SolveQuadraticForTarget(
        V1:number,
        k:number,
        fairAmount:number
    )  {
        // V0 = V1+V1*(sqrt-1)/2k
        let sqrt:number = DecimalMath.divCeil(DecimalMath.mul(k, fairAmount).mul(4), V1);
        sqrt = sqrt.add(DecimalMath.ONE).mul(DecimalMath.ONE).sqrt();
        let premium:number = DecimalMath.divCeil(sqrt.sub(DecimalMath.ONE), k.mul(2));
        // V0 is greater than or equal to V1 according to the solution
        return DecimalMath.mul(V1, DecimalMath.ONE.add(premium));
    }
}
