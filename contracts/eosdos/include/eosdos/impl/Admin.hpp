/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/impl/Storage.hpp>

/**
 * @title Admin
 * @author DODO Breeder
 *
 * @notice Functions for admin operations
 */
class Admin : virtual  public Storage {
 private:
   DODOStore& stores;
 public:
   Admin(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , Storage(_stores,_factory){}
       // ============ Params Setting Functions ============

       void setOracle(const extended_symbol&  newOracle) {
      stores._ORACLE_ = newOracle;
   }

   void setSupervisor(address newSupervisor) { stores._SUPERVISOR_ = newSupervisor; }

   void setMaintainer(address newMaintainer) { stores._MAINTAINER_ = newMaintainer; }

   void setLiquidityProviderFeeRate(uint64_t newLiquidityPorviderFeeRate) {
      stores._LP_FEE_RATE_ = newLiquidityPorviderFeeRate;
      _checkDODOParameters();
   }

   void setMaintainerFeeRate(uint64_t newMaintainerFeeRate) {
      stores._MT_FEE_RATE_ = newMaintainerFeeRate;
      _checkDODOParameters();
   }

   void setK(uint64_t newK) {

      stores._K_ = newK;
      _checkDODOParameters();
   }

   void setGasPriceLimit(uint64_t newGasPriceLimit) { stores._GAS_PRICE_LIMIT_ = newGasPriceLimit; }

   // ============ System Control Functions ============

   void disableTrading() { stores._TRADE_ALLOWED_ = false; }

   void enableTrading() { stores._TRADE_ALLOWED_ = true; }

   void disableQuoteDeposit() { stores._DEPOSIT_QUOTE_ALLOWED_ = false; }

   void enableQuoteDeposit() { stores._DEPOSIT_QUOTE_ALLOWED_ = true; }

   void disableBaseDeposit() { stores._DEPOSIT_BASE_ALLOWED_ = false; }

   void enableBaseDeposit() { stores._DEPOSIT_BASE_ALLOWED_ = true; }

   // ============ Advanced Control Functions ============

   void disableBuying() { stores._BUYING_ALLOWED_ = false; }

   void enableBuying() { stores._BUYING_ALLOWED_ = true; }

   void disableSelling() { stores._SELLING_ALLOWED_ = false; }

   void enableSelling() { stores._SELLING_ALLOWED_ = true; }

   void setBaseBalanceLimit(uint64_t newBaseBalanceLimit) { stores._BASE_BALANCE_LIMIT_ = newBaseBalanceLimit; }

   void setQuoteBalanceLimit(uint64_t newQuoteBalanceLimit) {
      stores._QUOTE_BALANCE_LIMIT_ = newQuoteBalanceLimit;
   }
};
