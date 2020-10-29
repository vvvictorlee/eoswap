/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/intf/IDODOLpToken.hpp>
#include <eosdos/intf/IFactory.hpp>
#include <eosdos/intf/IOracle.hpp>
#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/InitializableOwnable.hpp>
#include <eosdos/lib/ReentrancyGuard.hpp>
#include <eosdos/lib/SafeMath.hpp>
#include <eosdos/lib/Types.hpp>

/**
 * @title Storage
 * @author DODO Breeder
 *
 * @notice Local Variables
 */
using namespace SafeMath;
class Storage : public InitializableOwnable, public ReentrancyGuard {
 private:
   DODOStore& stores;
   IFactory&  factory;

 public:
   Storage(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , factory(_factory)
       , InitializableOwnable(_stores.initownable)
       , ReentrancyGuard(_stores.guard) {}

   // ============ Modifiers ============
   void onlySupervisorOrOwner() {
      require(
          getMsgSender() == stores._SUPERVISOR_ || getMsgSender() == stores.initownable._OWNER_,
          "NOT_SUPERVISOR_OR_OWNER");
   }

   void notClosed() { require(!stores._CLOSED_, "DODO_CLOSED"); }

   // ============ Helper Functions ============

   void _checkDODOParameters() {
      require(stores._K_ < DecimalMath::ONE, "K>=1");
      require(stores._K_ > 0, "K=0");
      require(add(stores._LP_FEE_RATE_, stores._MT_FEE_RATE_) < DecimalMath::ONE, "FEE_RATE>=1");
   }

   uint256 getOraclePrice() {
      uint256 price = 0;
      factory.get_oracle(stores._ORACLE_, [&](auto& oracle) { price = oracle.getPrice(); });
      return price;
      // return IOracle(_ORACLE_).getPrice();
   }

   uint256 getBaseCapitalBalanceOf(address _lp) {
      uint256 balance = 0;
      factory.get_lptoken(stores._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { balance = lptoken.balanceOf(_lp); });
      return balance;
      // return IDODOLpToken(stores._BASE_CAPITAL_TOKEN_).balanceOf(lp);
   }

   uint256 getTotalBaseCapital() {
      uint256 totalSupply = 0;
      factory.get_lptoken(stores._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { totalSupply = lptoken.totalSupply(); });
      return totalSupply;

      // return IDODOLpToken(stores._BASE_CAPITAL_TOKEN_).totalSupply();
   }

   uint256 getQuoteCapitalBalanceOf(address _lp) {
      uint256 balance = 0;
      factory.get_lptoken(stores._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { balance = lptoken.balanceOf(_lp); });
      return balance;
      //   return IDODOLpToken(stores._QUOTE_CAPITAL_TOKEN_).balanceOf(lp);
   }

   uint256 getTotalQuoteCapital() {
      uint256 totalSupply = 0;
      factory.get_lptoken(stores._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { totalSupply = lptoken.totalSupply(); });

      return totalSupply;
      // return IDODOLpToken(stores._QUOTE_CAPITAL_TOKEN_).totalSupply();
   }

   // ============ Version Control ============
   uint256 version() {
      return 101; // 1.0.1
   }
};
