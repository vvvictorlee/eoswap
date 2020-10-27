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

class DODO : public Admin, public Trader, public LiquidityProvider {
 private:
   DODOStore& stores;
   IStorage& storage;

 public:
   DODO(DODOStore& _stores, IStorage& _storage)
       : stores(_stores)
       , storage(_storage)
       , Admin(_stores,_storage)
       , Trader(_stores,_storage)
       , LiquidityProvider(_stores,_storage),Storage(_stores, _storage) {}
   void init(
       address owner, address supervisor, address maintainer, const extended_symbol& baseToken,
       const extended_symbol& quoteToken, const extended_symbol&  oracle, uint256 lpFeeRate, uint256 mtFeeRate, uint256 k,
       uint256 gasPriceLimit) {
      require(!stores.store._INITIALIZED_, "DODO_INITIALIZED");
      stores.store._INITIALIZED_ = true;

      stores.initownable._OWNER_ = owner;

      stores.store._SUPERVISOR_  = supervisor;
      stores.store._MAINTAINER_  = maintainer;
      stores.store._BASE_TOKEN_  = baseToken;
      stores.store._QUOTE_TOKEN_ = quoteToken;
      stores.store._ORACLE_      = oracle;

      stores.store._DEPOSIT_BASE_ALLOWED_  = false;
      stores.store._DEPOSIT_QUOTE_ALLOWED_ = false;
      stores.store._TRADE_ALLOWED_         = false;
      stores.store._GAS_PRICE_LIMIT_       = gasPriceLimit;

      // Advanced controls are disabled by default
      stores.store._BUYING_ALLOWED_  = true;
      stores.store._SELLING_ALLOWED_ = true;
      uint256 MAX_INT                = 0xffffffffffffffff;
      //   ffffffffffffffffffffffffffffffffffffffffffffffff;
      stores.store._BASE_BALANCE_LIMIT_  = MAX_INT;
      stores.store._QUOTE_BALANCE_LIMIT_ = MAX_INT;

      stores.store._LP_FEE_RATE_ = lpFeeRate;
      stores.store._MT_FEE_RATE_ = mtFeeRate;
      stores.store._K_           = k;
      stores.store._R_STATUS_    = Types::RStatus::ONE;
      auto        ob             = storage.get_storage_mgmt().get_token_store(stores.store._BASE_TOKEN_);
      auto        oq             = storage.get_storage_mgmt().get_token_store(stores.store._QUOTE_TOKEN_);
      auto        oblp           = storage.get_storage_mgmt().newLpTokenStore(stores.store._BASE_TOKEN_);
      auto        oqlp           = storage.get_storage_mgmt().newLpTokenStore(stores.store._QUOTE_TOKEN_);
      DODOLpToken b(oblp, ob);
      DODOLpToken q(oq, oqlp);
      stores.store._BASE_CAPITAL_TOKEN_  = b.get_esymbol(); // address(new DODOLpToken(_BASE_TOKEN_));
      stores.store._QUOTE_CAPITAL_TOKEN_ = q.get_esymbol(); // address(new DODOLpToken(_QUOTE_TOKEN_));

      _checkDODOParameters();
   }

   //      void  transferOwnership(address newOwner){}

   //      void  claimOwnership() {}

   //      uint256 sellBaseToken(
   //         uint256 amount,
   //         uint256 minReceiveQuote,
   //         bytes  data
   //     ){}

   //      uint256 buyBaseToken(
   //         uint256 amount,
   //         uint256 maxPayQuote,
   //         bytes  data
   //     ) {
   // }

   //      uint256  querySellBaseToken(uint256 amount) {}

   //      uint256  getExpectedTarget() {}

   //      uint256  withdrawBase(uint256 amount) {}

   //      uint256  depositQuoteTo(address to, uint256 amount) {}

   //      address  withdrawAllQuote() {}

   namesym _BASE_TOKEN_() { return to_namesym(stores.store._BASE_TOKEN_); }

   namesym _QUOTE_TOKEN_() { return to_namesym(stores.store._QUOTE_TOKEN_); }

   const extended_symbol& _BASE_CAPITAL_TOKEN_() { return stores.store._BASE_CAPITAL_TOKEN_; }
  const extended_symbol& _QUOTE_CAPITAL_TOKEN_() { return stores.store._QUOTE_CAPITAL_TOKEN_; }

};
