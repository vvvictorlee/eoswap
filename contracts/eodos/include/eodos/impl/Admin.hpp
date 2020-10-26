/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once
#include <common/defines.hpp>

#include <eodos/Storage.hpp>

/**
 * @title Admin
 * @author DODO Breeder
 *
 * @notice Functions for admin operations
 */
class Admin : public Storage {
 private:
   DODOStore& stores;

 public:
   Admin(DODOStore& _stores)
       : stores(_stores)
       , Storage(_stores)
       // ============ Params Setting Functions ============

       void setOracle(address newOracle) {
      _ORACLE_ = newOracle;
   }

   void setSupervisor(address newSupervisor) { stores.store._SUPERVISOR_ = newSupervisor; }

   void setMaintainer(address newMaintainer) { stores.store._MAINTAINER_ = newMaintainer; }

   void setLiquidityProviderFeeRate(uint256 newLiquidityPorviderFeeRate) {
      stores.store._LP_FEE_RATE_ = newLiquidityPorviderFeeRate;
      _checkDODOParameters();
   }

   void setMaintainerFeeRate(uint256 newMaintainerFeeRate) {
      stores.store._MT_FEE_RATE_ = newMaintainerFeeRate;
      _checkDODOParameters();
   }

   void setK(uint256 newK) {

      stores.store._K_ = newK;
      _checkDODOParameters();
   }

   void setGasPriceLimit(uint256 newGasPriceLimit) { stores.store._GAS_PRICE_LIMIT_ = newGasPriceLimit; }

   // ============ System Control Functions ============

   void disableTrading() { stores.store._TRADE_ALLOWED_ = false; }

   void enableTrading() { stores.store._TRADE_ALLOWED_ = true; }

   void disableQuoteDeposit() { stores.store._DEPOSIT_QUOTE_ALLOWED_ = false; }

   void enableQuoteDeposit() { stores.store._DEPOSIT_QUOTE_ALLOWED_ = true; }

   void disableBaseDeposit() { stores.store._DEPOSIT_BASE_ALLOWED_ = false; }

   void enableBaseDeposit() { stores.store._DEPOSIT_BASE_ALLOWED_ = true; }

   // ============ Advanced Control Functions ============

   void disableBuying() { stores.store._BUYING_ALLOWED_ = false; }

   void enableBuying() { stores.store._BUYING_ALLOWED_ = true; }

   void disableSelling() { stores.store._SELLING_ALLOWED_ = false; }

   void enableSelling() { stores.store._SELLING_ALLOWED_ = true; }

   void setBaseBalanceLimit(uint256 newBaseBalanceLimit) { stores.store._BASE_BALANCE_LIMIT_ = newBaseBalanceLimit; }

   void setQuoteBalanceLimit(uint256 newQuoteBalanceLimit) {
      stores.store._QUOTE_BALANCE_LIMIT_ = newQuoteBalanceLimit;
   }
};
