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
#include <eosdos/lib/SafeMath.hpp>
#include <eosdos/lib/Types.hpp>

/**
 * @title Storage
 * @author DODO Breeder
 *
 * @notice Local Variables
 */
using namespace SafeMath;
class Storage {
 private:
   DODOStore& stores;
   IFactory&  factory;
name msg_sender;
 public:
   Storage(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , factory(_factory)
        {}

   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender, bool flag = false) {
      if (flag) {
         require_auth(_msg_sender);
      }
      msg_sender = _msg_sender;
   }

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
      return factory.get_storage_mgmt().get_oracle_prices(stores._BASE_TOKEN_, stores._QUOTE_TOKEN_);
      // return IOracle(_ORACLE_).getPrice();
   }

   uint256 getBaseCapitalBalanceOf(address _lp) {
      uint256 balance = 0;
      //   asset lpbalance = transfer_mgmt::get_balance(getMsgSender(), stores._BASE_CAPITAL_TOKEN_);
      //   balance = lpbalance.amount;
      factory.get_lptoken(
          getMsgSender(), stores._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { balance = lptoken.balanceOf(_lp); });
      return balance;
      // return IDODOLpToken(stores._BASE_CAPITAL_TOKEN_).balanceOf(lp);
   }

   uint256 getTotalBaseCapital() {
      uint256 totalSupply = 0;
      factory.get_lptoken(
          getMsgSender(), stores._BASE_CAPITAL_TOKEN_, [&](auto& lptoken) { totalSupply = lptoken.totalSupply(); });
      return totalSupply;

      // return IDODOLpToken(stores._BASE_CAPITAL_TOKEN_).totalSupply();
   }

   uint256 getQuoteCapitalBalanceOf(address _lp) {
      uint256 balance = 0;
      factory.get_lptoken(
          getMsgSender(), stores._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { balance = lptoken.balanceOf(_lp); });
      return balance;
      //   return IDODOLpToken(stores._QUOTE_CAPITAL_TOKEN_).balanceOf(lp);
   }

   uint256 getTotalQuoteCapital() {
      uint256 totalSupply = 0;
      factory.get_lptoken(
          getMsgSender(), stores._QUOTE_CAPITAL_TOKEN_, [&](auto& lptoken) { totalSupply = lptoken.totalSupply(); });

      return totalSupply;
      // return IDODOLpToken(stores._QUOTE_CAPITAL_TOKEN_).totalSupply();
   }

   // ============ Version Control ============
   uint256 version() {
      return 101; // 1.0.1
   }
};
