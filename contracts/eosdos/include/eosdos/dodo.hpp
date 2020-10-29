/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <common/storage_mgmt.hpp>
#include <eosdos/impl/Admin.hpp>
#include <eosdos/impl/DODOLpToken.hpp>
#include <eosdos/impl/LiquidityProvider.hpp>
#include <eosdos/impl/Storage.hpp>
#include <eosdos/impl/Trader.hpp>
#include <eosdos/intf/IERC20.hpp>
#include <eosdos/lib/Types.hpp>

/**
 * @title DODO
 * @author DODO Breeder
 *
 * @notice Entrance for users
 */
static const uint256 MAX_INT = 0xffffffffffffffff;
//   ffffffffffffffffffffffffffffffffffffffffffffffff;

class DODO : public Admin, public Trader, virtual public LiquidityProvider {
 private:
   DODOStore& stores;
   IFactory&  factory;

 public:
   DODO(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , factory(_factory)
       , Admin(_stores, _factory)
       , Trader(_stores, _factory)
       , LiquidityProvider(_stores, _factory)
       , Storage(_stores, _factory)
       , Pricing(_stores, _factory)
       , Settlement(_stores, _factory) {
}
   void init(
       address owner, address supervisor, address maintainer, const extended_symbol& baseToken,
       const extended_symbol& quoteToken, const extended_symbol& oracle, uint256 lpFeeRate, uint256 mtFeeRate,
       uint256 k, uint256 gasPriceLimit) {
      require(!stores._INITIALIZED_, "DODO_INITIALIZED");
      stores._INITIALIZED_ = true;

      stores.initownable._OWNER_ = owner;

      stores._SUPERVISOR_            = supervisor;
      stores._MAINTAINER_            = maintainer;
      stores._BASE_TOKEN_            = baseToken;
      stores._QUOTE_TOKEN_           = quoteToken;
      stores._ORACLE_                = oracle;
      stores._DEPOSIT_BASE_ALLOWED_  = false;
      stores._DEPOSIT_QUOTE_ALLOWED_ = false;
      stores._TRADE_ALLOWED_         = false;
      stores._GAS_PRICE_LIMIT_       = gasPriceLimit;

      // Advanced controls are disabled by default
      stores._BUYING_ALLOWED_  = true;
      stores._SELLING_ALLOWED_ = true;

      stores._BASE_BALANCE_LIMIT_  = MAX_INT;
      stores._QUOTE_BALANCE_LIMIT_ = MAX_INT;

      stores._LP_FEE_RATE_         = lpFeeRate;
      stores._MT_FEE_RATE_         = mtFeeRate;
      stores._K_                   = k;
      stores._R_STATUS_            = Types::RStatus::ONE;
      stores._BASE_CAPITAL_TOKEN_  = factory.newLpToken(stores._BASE_TOKEN_);
      stores._QUOTE_CAPITAL_TOKEN_ = factory.newLpToken(stores._QUOTE_TOKEN_);

      _checkDODOParameters();
   }

   namesym _BASE_TOKEN_() { return to_namesym(stores._BASE_TOKEN_); }

   namesym _QUOTE_TOKEN_() { return to_namesym(stores._QUOTE_TOKEN_); }

   const extended_symbol& _BASE_CAPITAL_TOKEN_() { return stores._BASE_CAPITAL_TOKEN_; }
   const extended_symbol& _QUOTE_CAPITAL_TOKEN_() { return stores._QUOTE_CAPITAL_TOKEN_; }
};
