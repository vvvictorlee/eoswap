/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

#include <eodos/impl/Admin.hpp>
#include <eodos/impl/DODOLpToken.hpp>
#include <eodos/impl/LiquidityProvider.hpp>
#include <eodos/impl/Storage.hpp>
#include <eodos/impl/Trader.hpp>
#include <eodos/intf/IERC20.hpp>
#include <eodos/lib/Types.hpp>

/**
 * @title DODO
 * @author DODO Breeder
 *
 * @notice Entrance for users
 */
class DODO : public Admin, public Trader, public LiquidityProvider {
 public:
   void init(
       address owner, address supervisor, address maintainer, address baseToken, address quoteToken, address oracle,
       uint256 lpFeeRate, uint256 mtFeeRate, uint256 k, uint256 gasPriceLimit) {
      require(!_INITIALIZED_, "DODO_INITIALIZED");
      _INITIALIZED_ = true;

      _OWNER_ = owner;

      _SUPERVISOR_  = supervisor;
      _MAINTAINER_  = maintainer;
      _BASE_TOKEN_  = baseToken;
      _QUOTE_TOKEN_ = quoteToken;
      _ORACLE_      = oracle;

      _DEPOSIT_BASE_ALLOWED_  = false;
      _DEPOSIT_QUOTE_ALLOWED_ = false;
      _TRADE_ALLOWED_         = false;
      _GAS_PRICE_LIMIT_       = gasPriceLimit;

      // Advanced controls are disabled by default
      _BUYING_ALLOWED_      = true;
      _SELLING_ALLOWED_     = true;
      uint256 MAX_INT       = 0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff;
      _BASE_BALANCE_LIMIT_  = MAX_INT;
      _QUOTE_BALANCE_LIMIT_ = MAX_INT;

      _LP_FEE_RATE_ = lpFeeRate;
      _MT_FEE_RATE_ = mtFeeRate;
      _K_           = k;
      _R_STATUS_    = Types.RStatus.ONE;

      _BASE_CAPITAL_TOKEN_  = address(new DODOLpToken(_BASE_TOKEN_));
      _QUOTE_CAPITAL_TOKEN_ = address(new DODOLpToken(_QUOTE_TOKEN_));

      _checkDODOParameters();
   }
}
