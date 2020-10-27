/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eodos/intf/IDODOLpToken.hpp>
#include <eodos/intf/IOracle.hpp>
#include <eodos/intf/IStorage.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/InitializableOwnable.hpp>
#include <eodos/lib/ReentrancyGuard.hpp>
#include <eodos/lib/SafeMath.hpp>
#include <eodos/lib/Types.hpp>

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
   IStorage&  storage;

 public:
   Storage(DODOStore& _stores, IStorage& _storage)
       : stores(_stores)
       , storage(_storage)
       , InitializableOwnable(_stores.initownable)
       , ReentrancyGuard(_stores.guard) {}

   // ============ Modifiers ============
   void onlySupervisorOrOwner() {
      require(
          getMsgSender() == stores.store._SUPERVISOR_ || getMsgSender() == stores.initownable._OWNER_,
          "NOT_SUPERVISOR_OR_OWNER");
   }

   void notClosed() { require(!stores.store._CLOSED_, "DODO_CLOSED"); }

   // ============ Helper Functions ============

   void _checkDODOParameters() {
      require(stores.store._K_ < DecimalMath::ONE, "K>=1");
      require(stores.store._K_ > 0, "K=0");
      require(add(stores.store._LP_FEE_RATE_, stores.store._MT_FEE_RATE_) < DecimalMath::ONE, "FEE_RATE>=1");
   }

   uint256 getOraclePrice() {
      uint256 price = 0;
      storage.get_oracle(stores.store._ORACLE_, [&](auto& oracle) { price = oracle.getPrice(); });
      return price;
      // return IOracle(_ORACLE_).getPrice();
   }

   uint256 getBaseCapitalBalanceOf(address _lp) {
      uint256 balance = 0;
      storage.get_lptoken(stores.store._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { balance = lptoken.balanceOf(_lp); });
      return balance;
      // return IDODOLpToken(stores.store._BASE_CAPITAL_TOKEN_).balanceOf(lp);
   }

   uint256 getTotalBaseCapital() {
      uint256 totalSupply = 0;
      storage.get_lptoken(
          stores.store._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { totalSupply = lptoken.totalSupply(); });
      return totalSupply;

      // return IDODOLpToken(stores.store._BASE_CAPITAL_TOKEN_).totalSupply();
   }

   uint256 getQuoteCapitalBalanceOf(address _lp) {
      uint256 balance = 0;
      storage.get_lptoken(stores.store._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { balance = lptoken.balanceOf(_lp); });
      return balance;
      //   return IDODOLpToken(stores.store._QUOTE_CAPITAL_TOKEN_).balanceOf(lp);
   }

   uint256 getTotalQuoteCapital() {
      uint256 totalSupply = 0;
      storage.get_lptoken(
          stores.store._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { totalSupply = lptoken.totalSupply(); });
      return totalSupply;
      // return IDODOLpToken(stores.store._QUOTE_CAPITAL_TOKEN_).totalSupply();
   }

   // ============ Version Control ============
   uint256 version() {
      return 101; // 1.0.1
   }
};
