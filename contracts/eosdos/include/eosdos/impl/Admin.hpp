/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/impl/Storage.hpp>
#include <set>
/**
 * @title Admin
 * @author DODO Breeder
 *
 * @notice Functions for admin operations
 */
class Admin : virtual public Storage {
 private:
   DODOStore& stores;

 public:
   Admin(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , Storage(_stores, _factory) {}
   // ============ Params Setting Functions ============
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
   // ============ System Control Functions ============
   void enableTrading(uint64_t flag) { stores._TRADE_ALLOWED_ = (flag != 0); }

   void enableQuoteDeposit(uint64_t flag) { stores._DEPOSIT_QUOTE_ALLOWED_ = (flag != 0); }

   void enableBaseDeposit(uint64_t flag) { stores._DEPOSIT_BASE_ALLOWED_ = (flag != 0); }

   // ============ Advanced Control Functions ============
   void enableBuying(uint64_t flag) { stores._BUYING_ALLOWED_ = (flag != 0); }

   void enableSelling(uint64_t flag) { stores._SELLING_ALLOWED_ = (flag != 0); }

   void setBaseBalanceLimit(uint64_t newBaseBalanceLimit) { stores._BASE_BALANCE_LIMIT_ = newBaseBalanceLimit; }

   void setQuoteBalanceLimit(uint64_t newQuoteBalanceLimit) { stores._QUOTE_BALANCE_LIMIT_ = newQuoteBalanceLimit; }
   void setParameter(name para_name, uint64_t para_value) {
    //   std::set<name>            boolparas = {"trading"_n, "quotedeposit"_n, "basedeposit"_n, "buying"_n, "selling"_n};
      std::map<name, uint64_t&> paras     = {std::pair<name, uint64_t&>{"k"_n, stores._K_},
                                         std::pair<name, uint64_t&>{"lpfeerate"_n, stores._LP_FEE_RATE_},
                                         std::pair<name, uint64_t&>{"mtfeerate"_n, stores._MT_FEE_RATE_},
                                         std::pair<name, uint64_t&>{"basebalimit"_n, stores._BASE_BALANCE_LIMIT_},
                                         std::pair<name, uint64_t&>{"quotebalimit"_n, stores._QUOTE_BALANCE_LIMIT_}};
      std::map<name, bool&> boolparas = {
          std::pair<name, bool&>{"trading"_n, stores._TRADE_ALLOWED_},
          std::pair<name, bool&>{"quotedeposit"_n, stores._DEPOSIT_QUOTE_ALLOWED_},
          std::pair<name, bool&>{"basedeposit"_n, stores._DEPOSIT_BASE_ALLOWED_},
          std::pair<name, bool&>{"buying"_n, stores._BUYING_ALLOWED_},
          std::pair<name, bool&>{"selling"_n, stores._SELLING_ALLOWED_}};
      //   std::map<name, uint64_t&> paras = {std::pair<name, uint64_t&>{"k"_n, stores._K_}};

      auto it  = paras.find(para_name);
      auto itb = boolparas.find(para_name);
      check(it != paras.end() || itb != boolparas.end(), "no  parameter");
      if (it != paras.end()) {
         it->second = para_value;
      } else if (itb != boolparas.end()) {
         itb->second = !!para_value;
      } else {
         check(false, "no  parameter");
      }
   }
};
