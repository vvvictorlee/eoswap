/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>


#include <eodos/Storage.hpp>


/**
 * @title Admin
 * @author DODO Breeder
 *
 * @notice Functions for admin operations
 */
class Admin : public  Storage {
    // ============ Params Setting Functions ============

    void  setOracle(address newOracle) {
        _ORACLE_ = newOracle;
    }

    void  setSupervisor(address newSupervisor) {
        _SUPERVISOR_ = newSupervisor;
    }

    void  setMaintainer(address newMaintainer) {
        _MAINTAINER_ = newMaintainer;
    }

    void  setLiquidityProviderFeeRate(uint256 newLiquidityPorviderFeeRate) {
        
        _LP_FEE_RATE_ = newLiquidityPorviderFeeRate;
        _checkDODOParameters();
    }

    void  setMaintainerFeeRate(uint256 newMaintainerFeeRate) {
        
        _MT_FEE_RATE_ = newMaintainerFeeRate;
        _checkDODOParameters();
    }

    void  setK(uint256 newK) {
        
        _K_ = newK;
        _checkDODOParameters();
    }

    void  setGasPriceLimit(uint256 newGasPriceLimit) {
        
        _GAS_PRICE_LIMIT_ = newGasPriceLimit;
    }

    // ============ System Control Functions ============

    void  disableTrading() {
        _TRADE_ALLOWED_ = false;
    }

    void  enableTrading() {
        _TRADE_ALLOWED_ = true;
    }

    void  disableQuoteDeposit() {
        _DEPOSIT_QUOTE_ALLOWED_ = false;
    }

    void  enableQuoteDeposit() {
        _DEPOSIT_QUOTE_ALLOWED_ = true;
    }

    void  disableBaseDeposit() {
        _DEPOSIT_BASE_ALLOWED_ = false;
    }

    void  enableBaseDeposit() {
        _DEPOSIT_BASE_ALLOWED_ = true;
    }

    // ============ Advanced Control Functions ============

    void  disableBuying() {
        _BUYING_ALLOWED_ = false;
    }

    void  enableBuying() {
        _BUYING_ALLOWED_ = true;
    }

    void  disableSelling() {
        _SELLING_ALLOWED_ = false;
    }

    void  enableSelling() {
        _SELLING_ALLOWED_ = true;
    }

    void  setBaseBalanceLimit(uint256 newBaseBalanceLimit) {
        _BASE_BALANCE_LIMIT_ = newBaseBalanceLimit;
    }

    void  setQuoteBalanceLimit(uint256 newQuoteBalanceLimit) {
        _QUOTE_BALANCE_LIMIT_ = newQuoteBalanceLimit;
    }
}
