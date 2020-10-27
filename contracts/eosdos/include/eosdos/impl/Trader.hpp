/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/impl/Pricing.hpp>
#include <eosdos/impl/Settlement.hpp>
#include <eosdos/impl/Storage.hpp>
#include <eosdos/intf/IDODOCallee.hpp>
#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/SafeMath.hpp>
#include <eosdos/lib/Types.hpp>
/**
 * @title Trader
 * @author DODO Breeder
 *
 * @notice Functions for trader operations
 */
static const uint256 tx_gasprice = 0;
class Trader : virtual public Storage, public Pricing, public Settlement {
 private:
   DODOStore& stores;

 public:
   Trader(DODOStore& _stores, IStorage& _storage)
       : stores(_stores)
       , Storage(_stores, _storage)
       , Pricing(_stores, _storage)
       , Settlement(_stores, _storage) {}
   // ============ Events ============

   void tradeAllowed() { require(stores.store._TRADE_ALLOWED_, "TRADE_NOT_ALLOWED"); }

   void buyingAllowed() { require(stores.store._BUYING_ALLOWED_, "BUYING_NOT_ALLOWED"); }

   void sellingAllowed() { require(stores.store._SELLING_ALLOWED_, "SELLING_NOT_ALLOWED"); }

   void gasPriceLimit() { require(tx_gasprice <= stores.store._GAS_PRICE_LIMIT_, "GAS_PRICE_EXCEED"); }

   // ============ Trade Functions ============

   uint256 sellBaseToken(uint256 amount, uint256 minReceiveQuote, bytes data) {
      tradeAllowed();
      sellingAllowed();
      gasPriceLimit();
      preventReentrant();
      // query price
      uint256 receiveQuote;
      uint256 lpFeeQuote;
      uint256 mtFeeQuote;
      uint8   newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(receiveQuote, lpFeeQuote, mtFeeQuote, newRStatus, newQuoteTarget, newBaseTarget) =
          _querySellBaseToken(amount);
      require(receiveQuote >= minReceiveQuote, "SELL_BASE_RECEIVE_NOT_ENOUGH");

      // settle assets
      _quoteTokenTransferOut(getMsgSender(), extended_asset(receiveQuote, stores.store._QUOTE_TOKEN_));
      if (data.size() > 0) {
         //  IDODOCallee(getMsgSender()).dodoCall(false, amount, receiveQuote, data);
      }
      _baseTokenTransferIn(getMsgSender(), extended_asset(amount, stores.store._BASE_TOKEN_));
      if (mtFeeQuote != 0) {
         _quoteTokenTransferOut(stores.store._MAINTAINER_, extended_asset(mtFeeQuote, stores.store._QUOTE_TOKEN_));
      }

      // update TARGET
      if (stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ != newQuoteTarget) {
         stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = newQuoteTarget;
      }
      if (stores.store._TARGET_BASE_TOKEN_AMOUNT_ != newBaseTarget) {
         stores.store._TARGET_BASE_TOKEN_AMOUNT_ = newBaseTarget;
      }
      if (stores.store._R_STATUS_ != newRStatus) {
         stores.store._R_STATUS_ = newRStatus;
      }

      _donateQuoteToken(lpFeeQuote);

      return receiveQuote;
   }

   uint256 buyBaseToken(uint256 amount, uint256 maxPayQuote, bytes data) {
      tradeAllowed();
      buyingAllowed();
      gasPriceLimit();
      preventReentrant();
      // query price
      uint256 payQuote;
      uint256 lpFeeBase;
      uint256 mtFeeBase;
      uint8   newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(payQuote, lpFeeBase, mtFeeBase, newRStatus, newQuoteTarget, newBaseTarget) = _queryBuyBaseToken(amount);
      require(payQuote <= maxPayQuote, "BUY_BASE_COST_TOO_MUCH");

      // settle assets
      _baseTokenTransferOut(getMsgSender(), extended_asset(amount, stores.store._BASE_TOKEN_));
      if (data.size() > 0) {
         //  IDODOCallee(getMsgSender()).dodoCall(true, amount, payQuote, data);
      }
      _quoteTokenTransferIn(getMsgSender(), extended_asset(payQuote, stores.store._QUOTE_TOKEN_));
      if (mtFeeBase != 0) {
         _baseTokenTransferOut(stores.store._MAINTAINER_, extended_asset(mtFeeBase, stores.store._BASE_TOKEN_));
      }

      // update TARGET
      if (stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ != newQuoteTarget) {
         stores.store._TARGET_QUOTE_TOKEN_AMOUNT_ = newQuoteTarget;
      }
      if (stores.store._TARGET_BASE_TOKEN_AMOUNT_ != newBaseTarget) {
         stores.store._TARGET_BASE_TOKEN_AMOUNT_ = newBaseTarget;
      }
      if (stores.store._R_STATUS_ != newRStatus) {
         stores.store._R_STATUS_ = newRStatus;
      }

      _donateBaseToken(lpFeeBase);

      return payQuote;
   }

   // ============ Query Functions ============

   uint256 querySellBaseToken(uint256 amount) {
      uint256 receiveQuote = 0;
      // tie(i, ignore, s)
      std::tie(receiveQuote, std::ignore, std::ignore, std::ignore, std::ignore, std::ignore) =
          _querySellBaseToken(amount);
      return receiveQuote;
   }

   uint256 queryBuyBaseToken(uint256 amount) {
      uint256 payQuote = 0 ;                                                        
      std::tie(payQuote, std::ignore, std::ignore, std::ignore, std::ignore, std::ignore) = _queryBuyBaseToken(amount);
      return payQuote;
   }

   std::tuple<uint256, uint256, uint256, uint8, uint256, uint256> _querySellBaseToken(uint256 amount) {
      uint256 receiveQuote;
      uint256 lpFeeQuote;
      uint256 mtFeeQuote;
      uint8   newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;

      std::tie(newBaseTarget, newQuoteTarget) = getExpectedTarget();

      uint256 sellBaseAmount = amount;

      if (stores.store._R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         // R falls below one
         receiveQuote = _ROneSellBaseToken(sellBaseAmount, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
      } else if (stores.store._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 backToOnePayBase      = sub(newBaseTarget, stores.store._BASE_BALANCE_);
         uint256 backToOneReceiveQuote = sub(stores.store._QUOTE_BALANCE_, newQuoteTarget);
         // case 2: R>1
         // complex case, R status depends on trading amount
         if (sellBaseAmount < backToOnePayBase) {
            // case 2.1: R status do not change
            receiveQuote = _RAboveSellBaseToken(sellBaseAmount, stores.store._BASE_BALANCE_, newBaseTarget);
            newRStatus   = Types::RStatus::ABOVE_ONE;
            if (receiveQuote > backToOneReceiveQuote) {
               // [Important corner case!] may enter this branch when some precision problem happens. And consequently
               // contribute to negative spare quote amount to make sure spare quote>=0, mannually set
               // receiveQuote=backToOneReceiveQuote
               receiveQuote = backToOneReceiveQuote;
            }
         } else if (sellBaseAmount == backToOnePayBase) {
            // case 2.2: R status changes to ONE
            receiveQuote = backToOneReceiveQuote;
            newRStatus   = Types::RStatus::ONE;
         } else {
            // case 2.3: R status changes to BELOW_ONE
            receiveQuote =
                add(backToOneReceiveQuote, _ROneSellBaseToken(sub(sellBaseAmount, backToOnePayBase), newQuoteTarget));
            newRStatus = Types::RStatus::BELOW_ONE;
         }
      } else {
         // _R_STATUS_ == Types::RStatus::BELOW_ONE
         // case 3: R<1
         receiveQuote = _RBelowSellBaseToken(sellBaseAmount, stores.store._QUOTE_BALANCE_, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
      }

      // count fees
      lpFeeQuote   = DecimalMath::mul(receiveQuote, stores.store._LP_FEE_RATE_);
      mtFeeQuote   = DecimalMath::mul(receiveQuote, stores.store._MT_FEE_RATE_);
      receiveQuote = sub(sub(receiveQuote, lpFeeQuote), mtFeeQuote);

      return std::make_tuple(receiveQuote, lpFeeQuote, mtFeeQuote, newRStatus, newQuoteTarget, newBaseTarget);
   }

   std::tuple<uint256, uint256, uint256, uint8, uint256, uint256> _queryBuyBaseToken(uint256 amount)

   {
      uint256 payQuote;
      uint256 lpFeeBase;
      uint256 mtFeeBase;
      uint8   newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;

      std::tie(newBaseTarget, newQuoteTarget) = getExpectedTarget();

      // charge fee from user receive amount
      lpFeeBase             = DecimalMath::mul(amount, stores.store._LP_FEE_RATE_);
      mtFeeBase             = DecimalMath::mul(amount, stores.store._MT_FEE_RATE_);
      uint256 buyBaseAmount = add(add(amount, lpFeeBase), mtFeeBase);

      if (stores.store._R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         payQuote   = _ROneBuyBaseToken(buyBaseAmount, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
      } else if (stores.store._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         // case 2: R>1
         payQuote   = _RAboveBuyBaseToken(buyBaseAmount, stores.store._BASE_BALANCE_, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
      } else if (stores.store._R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 backToOnePayQuote    = sub(newQuoteTarget, stores.store._QUOTE_BALANCE_);
         uint256 backToOneReceiveBase = sub(stores.store._BASE_BALANCE_, newBaseTarget);
         // case 3: R<1
         // complex case, R status may change
         if (buyBaseAmount < backToOneReceiveBase) {
            // case 3.1: R status do not change
            // no need to check payQuote because spare base token must be greater than zero
            payQuote   = _RBelowBuyBaseToken(buyBaseAmount, stores.store._QUOTE_BALANCE_, newQuoteTarget);
            newRStatus = Types::RStatus::BELOW_ONE;
         } else if (buyBaseAmount == backToOneReceiveBase) {
            // case 3.2: R status changes to ONE
            payQuote   = backToOnePayQuote;
            newRStatus = Types::RStatus::ONE;
         } else {
            // case 3.3: R status changes to ABOVE_ONE
            payQuote =
                add(backToOnePayQuote, _ROneBuyBaseToken(sub(buyBaseAmount, backToOneReceiveBase), newBaseTarget));
            newRStatus = Types::RStatus::ABOVE_ONE;
         }
      }

      return std::make_tuple(payQuote, lpFeeBase, mtFeeBase, newRStatus, newQuoteTarget, newBaseTarget);
   }
};
