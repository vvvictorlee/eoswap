/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once
#include <common/defines.hpp>

#include <eodos/intf/IDODOLpToken.hpp>
#include <eodos/intf/IOracle.hpp>
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
class Storage : public InitializableOwnable, public ReentrancyGuard {
 private:
   DODOStore& stores;
public:
   Storage(DODOStore& _stores)
       : stores(_stores)
       , InitializableOwnable(_stores)
       , ReentrancyGuard(_stores){}

   // ============ Modifiers ============
   void onlySupervisorOrOwner() {
      require(getMsgSender() == stores.store._SUPERVISOR_ || getMsgSender() == stores.store._OWNER_, "NOT_SUPERVISOR_OR_OWNER");
   }

   void notClosed() { require(!_CLOSED_, "DODO_CLOSED"); }

   // ============ Helper Functions ============

   uint256 _checkDODOParameters() {
      require(stores.store._K_ < DecimalMath.ONE, "K>=1");
      require(stores.store._K_ > 0, "K=0");
      require(stores.store._LP_FEE_RATE_.add(_MT_FEE_RATE_) < DecimalMath.ONE, "FEE_RATE>=1");
   }

   uint256 getOraclePrice() { return IOracle(_ORACLE_).getPrice(); }

   uint256 getBaseCapitalBalanceOf(address lp) { return IDODOLpToken(stores.store._BASE_CAPITAL_TOKEN_).balanceOf(lp); }

   uint256 getTotalBaseCapital() { return IDODOLpToken(stores.store._BASE_CAPITAL_TOKEN_).totalSupply(); }

   uint256 getQuoteCapitalBalanceOf(address lp) { return IDODOLpToken(stores.store._QUOTE_CAPITAL_TOKEN_).balanceOf(lp); }

   uint256 getTotalQuoteCapital() { return IDODOLpToken(stores.store._QUOTE_CAPITAL_TOKEN_).totalSupply(); }

   // ============ Version Control ============
   uint256 version() {
      return 101; // 1.0.1
   }
};
