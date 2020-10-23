/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>


#include <eodos/lib/InitializableOwnable.hpp>
#include <eodos/lib/SafeMath.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/ReentrancyGuard.hpp>
#include <eodos/intf/IOracle.hpp>
#include <eodos/intf/IDODOLpToken.hpp>
#include <eodos/lib/Types.hpp>


/**
 * @title Storage
 * @author DODO Breeder
 *
 * @notice Local Variables
 */
class Storage : public  InitializableOwnable,public  ReentrancyGuard{
    


    // ============ Modifiers ============

    void onlySupervisorOrOwner() {
        require(msg.sender == _SUPERVISOR_ || msg.sender == _OWNER_, "NOT_SUPERVISOR_OR_OWNER");
        
    }

    void notClosed() {
        require(!_CLOSED_, "DODO_CLOSED");
        
    }

    // ============ Helper Functions ============

    uint256  _checkDODOParameters() {
        require(_K_ < DecimalMath.ONE, "K>=1");
        require(_K_ > 0, "K=0");
        require(_LP_FEE_RATE_.add(_MT_FEE_RATE_) < DecimalMath.ONE, "FEE_RATE>=1");
    }

    uint256  getOraclePrice() {
        return IOracle(_ORACLE_).getPrice();
    }

    uint256  getBaseCapitalBalanceOf(address lp) {
        return IDODOLpToken(_BASE_CAPITAL_TOKEN_).balanceOf(lp);
    }

    uint256  getTotalBaseCapital() {
        return IDODOLpToken(_BASE_CAPITAL_TOKEN_).totalSupply();
    }

    uint256  getQuoteCapitalBalanceOf(address lp) {
        return IDODOLpToken(_QUOTE_CAPITAL_TOKEN_).balanceOf(lp);
    }

    uint256  getTotalQuoteCapital() {
        return IDODOLpToken(_QUOTE_CAPITAL_TOKEN_).totalSupply();
    }

    // ============ Version Control ============
    uint256  version() {
        return 101; // 1.0.1
    }
}
