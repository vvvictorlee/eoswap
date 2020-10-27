/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once
#include <common/defines.hpp>

#include <common/storage_mgmt.hpp>
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
 private:
   DODOStore& stores;
   DODOZoo&   zoo;

 public:
   DODO(DODOStore& _stores, DODOZoo& _zoo)
       : stores(_stores)
       , zoo(_zoo)
       , Admin(_stores)
       , Trader(_stores)
       , LiquidityProvider(_stores) {}
   void init(
       address owner, address supervisor, address maintainer, const extended_symbol& baseToken,
       const extended_symbol& quoteToken, const extended_symbol&  oracle, uint256 lpFeeRate, uint256 mtFeeRate, uint256 k,
       uint256 gasPriceLimit) {
      require(!_INITIALIZED_, "DODO_INITIALIZED");
      stores.store._INITIALIZED_ = true;

      stores.store._OWNER_ = owner;

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
      auto        ob             = zoo.get_storage_mgmt().get_token_store(_BASE_TOKEN_);
      auto        oq             = zoo.get_storage_mgmt().get_token_store(_QUOTE_TOKEN_);
      auto        oblp           = zoo.get_storage_mgmt().newLpTokenStore(_BASE_TOKEN_);
      auto        oqlp           = zoo.get_storage_mgmt().newLpTokenStore(_QUOTE_TOKEN_);
      DODOLpToken b(oblp, ob);
      DODOLpToken q(oq, oqlp);
      stores.store._BASE_CAPITAL_TOKEN_  = to_namesym(b.get_esymbol()); // address(new DODOLpToken(_BASE_TOKEN_));
      stores.store._QUOTE_CAPITAL_TOKEN_ = to_namesym(q.get_esymbol()); // address(new DODOLpToken(_QUOTE_TOKEN_));

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

   address _QUOTE_CAPITAL_TOKEN_() { return stores.store._QUOTE_CAPITAL_TOKEN_; }

   address _QUOTE_TOKEN_() { return stores.store._QUOTE_TOKEN_; }
};
