/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/


import {SafeMath} from "../lib/SafeMath";
import {DecimalMath} from "../lib/DecimalMath";
import {DODOMath} from "../lib/DODOMath";
import {Types_RStatus} from "../lib/Types";
import {Storage} from "./Storage";


/**
 * @title Pricing
 * @author DODO Breeder
 *
 * @notice DODO Pricing model
 */
export class Pricing extends Storage {

    // ============ R = 1 cases ============

     _ROneSellBaseToken(amount:number, targetQuoteTokenAmount:number)
    {
        let i:number = this.getOraclePrice();
        let Q2:number = DODOMath._SolveQuadraticForTrade(
            targetQuoteTokenAmount,
            targetQuoteTokenAmount,
            DecimalMath.mul(i, amount),
            false,
            this._K_
        );
        // in theory Q2 <= targetQuoteTokenAmount
        // however when amount is close to 0, precision problems may cause Q2 > targetQuoteTokenAmount
        return targetQuoteTokenAmount.sub(Q2);
    }

     _ROneBuyBaseToken(amount:number, targetBaseTokenAmount:number)
    {
        //require(amount < targetBaseTokenAmount, "DODO_BASE_BALANCE_NOT_ENOUGH");
        let B2:number = targetBaseTokenAmount.sub(amount);
        let payQuoteToken = this._RAboveIntegrate(targetBaseTokenAmount, targetBaseTokenAmount, B2);
        return payQuoteToken;
    }

    // ============ R < 1 cases ============

     _RBelowSellBaseToken(
        amount:number,
        quoteBalance:number,
        targetQuoteAmount:number
    )  {
       let i:number = this.getOraclePrice();
        let Q2:number = DODOMath._SolveQuadraticForTrade(
            targetQuoteAmount,
            quoteBalance,
            DecimalMath.mul(i, amount),
            false,
            this._K_
        );
        return quoteBalance.sub(Q2);
    }

     _RBelowBuyBaseToken(
        amount:number,
        quoteBalance:number,
        targetQuoteAmount:number
    )  {
        // Here we don't //require amount less than some value
        // Because it is limited at upper 
        // See Trader.queryBuyBaseToken
       let i:number = this.getOraclePrice();
        let Q2:number = DODOMath._SolveQuadraticForTrade(
            targetQuoteAmount,
            quoteBalance,
            DecimalMath.mulCeil(i, amount),
            true,
            this._K_
        );
        return Q2.sub(quoteBalance);
    }

     _RBelowBackToOne()  {
        // important: carefully design the system to make sure spareBase always greater than or equal to 0
        let spareBase:number = this._BASE_BALANCE_.sub(this._TARGET_BASE_TOKEN_AMOUNT_);
        let price:number = this.getOraclePrice();
        let fairAmount:number = DecimalMath.mul(spareBase, price);
        let newTargetQuote:number = DODOMath._SolveQuadraticForTarget(
            this._QUOTE_BALANCE_,
            this._K_,
            fairAmount
        );
        return newTargetQuote.sub(this._QUOTE_BALANCE_);
    }

    // ============ R > 1 cases ============

     _RAboveBuyBaseToken(
        amount:number,
        baseBalance:number,
        targetBaseAmount:number
    )  {
        //require(amount < baseBalance, "DODO_BASE_BALANCE_NOT_ENOUGH");
        let B2:number = baseBalance.sub(amount);
        return this._RAboveIntegrate(targetBaseAmount, baseBalance, B2);
    }

     _RAboveSellBaseToken(
        amount:number,
        baseBalance:number,
        targetBaseAmount:number
    )  {
        // here we don't //require B1 <= targetBaseAmount
        // Because it is limited at upper 
        // See Trader.querySellBaseToken
        let B1:number = baseBalance.add(amount);
        return this._RAboveIntegrate(targetBaseAmount, B1, baseBalance);
    }

     _RAboveBackToOne() {
        // important: carefully design the system to make sure spareBase always greater than or equal to 0
       let  spareQuote:number = this._QUOTE_BALANCE_.sub(this._TARGET_QUOTE_TOKEN_AMOUNT_);
       let price:number = this.getOraclePrice();
       let fairAmount:number = DecimalMath.divFloor(spareQuote, price);
       let  newTargetBase:number = DODOMath._SolveQuadraticForTarget(
            this._BASE_BALANCE_,
            this._K_,
            fairAmount
        );
        return newTargetBase.sub(this._BASE_BALANCE_);
    }

    // ============ Helper s ============

     getExpectedTarget() {
       let Q:number = this._QUOTE_BALANCE_;
       let B:number = this._BASE_BALANCE_;
        if (this._R_STATUS_ == Types_RStatus.ONE) {
            return [this._TARGET_BASE_TOKEN_AMOUNT_, this._TARGET_QUOTE_TOKEN_AMOUNT_];
        } else if (this._R_STATUS_ == Types_RStatus.BELOW_ONE) {
           let  payQuoteToken:number = this._RBelowBackToOne();
            return [this._TARGET_BASE_TOKEN_AMOUNT_, Q.add(payQuoteToken)];
        } else if (this._R_STATUS_ == Types_RStatus.ABOVE_ONE) {
           let payBaseToken:number = this._RAboveBackToOne();
            return [B.add(payBaseToken), this._TARGET_QUOTE_TOKEN_AMOUNT_];
        }
        return [0,0];
    }

     getMidPrice()  {
// baseTarget:number, quoteTarget:number
        let [baseTarget, quoteTarget] = this.getExpectedTarget();
        if (this._R_STATUS_ == Types_RStatus.BELOW_ONE) {
            let R:number = DecimalMath.divFloor(
                quoteTarget.mul(quoteTarget).div(this._QUOTE_BALANCE_),
                this._QUOTE_BALANCE_
            );
           R = DecimalMath.ONE.sub(this._K_).add(DecimalMath.mul(this._K_, R));
            return DecimalMath.divFloor(this.getOraclePrice(), R);
        } else {
           let R:number = DecimalMath.divFloor(
                baseTarget.mul(baseTarget).div(this._BASE_BALANCE_),
                this._BASE_BALANCE_
            );
            R = DecimalMath.ONE.sub(this._K_).add(DecimalMath.mul(this._K_, R));
            return DecimalMath.mul(this.getOraclePrice(), R);
        }
    }

     _RAboveIntegrate(
        B0:number,
        B1:number,
        B2:number
    ) {
        let i:number = this.getOraclePrice();
        return DODOMath._GeneralIntegrate(B0, B1, B2, i,this._K_);
    }

    //  _RBelowIntegrate(
    //     Q0:number,
    //     Q1:number,
    //     Q2:number
    // ) internal view returns (uint256) {
    //     i:number = getOraclePrice();
    //     i = DecimalMath.divFloor(DecimalMath.ONE, i); // 1/i
    //     return DODOMath._GeneralIntegrate(Q0, Q1, Q2, i, this._K_);
    // }
}
