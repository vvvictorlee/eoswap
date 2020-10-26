/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once
#include <common/defines.hpp>

#include <eodos/Pricing.hpp>
#include <eodos/Settlement.hpp>
#include <eodos/Storage.hpp>
#include <eodos/intf/IDODOCallee.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/SafeMath.hpp>
#include <eodos/lib/Types.hpp>

/**
 * @title Trader
 * @author DODO Breeder
 *
 * @notice Functions for trader operations
 */
class DODOZoo;
class Trader : public Storage, public Pricing, public Settlement {
 private:
   DODOStore& stores;
   DODOZoo&   zoo;

 public:
   LiquidityProvider(DODOStore& _stores)
       : stores(_stores)
       , Storage(_stores)
       , Pricing(_stores)
       , Settlement(_stores) {}
   // ============ Events ============

   void tradeAllowed() { require(_TRADE_ALLOWED_, "TRADE_NOT_ALLOWED"); }

   void buyingAllowed() { require(_BUYING_ALLOWED_, "BUYING_NOT_ALLOWED"); }

   void sellingAllowed() { require(_SELLING_ALLOWED_, "SELLING_NOT_ALLOWED"); }

   void gasPriceLimit() { require(tx.gasprice <= _GAS_PRICE_LIMIT_, "GAS_PRICE_EXCEED"); }

   // ============ Trade Functions ============

   uint256 sellBaseToken(uint256 amount, uint256 minReceiveQuote, bytes data) {
      tradeAllowed();
      sellingAllowed();
      gasPriceLimit();
      preventReentrant();
      // query price
      (uint256 receiveQuote, uint256 lpFeeQuote, uint256 mtFeeQuote, Types.RStatus newRStatus, uint256 newQuoteTarget,
       uint256 newBaseTarget) = _querySellBaseToken(amount);
      require(receiveQuote >= minReceiveQuote, "SELL_BASE_RECEIVE_NOT_ENOUGH");

      // settle assets
      _quoteTokenTransferOut(getMsgSender(), receiveQuote);
      if (data.length > 0) {
         IDODOCallee(getMsgSender()).dodoCall(false, amount, receiveQuote, data);
      }
      _baseTokenTransferIn(getMsgSender(), amount);
      if (mtFeeQuote != 0) {
         _quoteTokenTransferOut(_MAINTAINER_, mtFeeQuote);
      }

      // update TARGET
      if (_TARGET_QUOTE_TOKEN_AMOUNT_ != newQuoteTarget) {
         _TARGET_QUOTE_TOKEN_AMOUNT_ = newQuoteTarget;
      }
      if (_TARGET_BASE_TOKEN_AMOUNT_ != newBaseTarget) {
         _TARGET_BASE_TOKEN_AMOUNT_ = newBaseTarget;
      }
      if (_R_STATUS_ != newRStatus) {
         _R_STATUS_ = newRStatus;
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
      (uint256 payQuote, uint256 lpFeeBase, uint256 mtFeeBase, uint8 newRStatus, uint256 newQuoteTarget,
       uint256 newBaseTarget) = _queryBuyBaseToken(amount);
      require(payQuote <= maxPayQuote, "BUY_BASE_COST_TOO_MUCH");

      // settle assets
      _baseTokenTransferOut(getMsgSender(), amount);
      if (data.length > 0) {
         IDODOCallee(getMsgSender()).dodoCall(true, amount, payQuote, data);
      }
      _quoteTokenTransferIn(getMsgSender(), payQuote);
      if (mtFeeBase != 0) {
         _baseTokenTransferOut(_MAINTAINER_, mtFeeBase);
      }

      // update TARGET
      if (_TARGET_QUOTE_TOKEN_AMOUNT_ != newQuoteTarget) {
         _TARGET_QUOTE_TOKEN_AMOUNT_ = newQuoteTarget;
      }
      if (_TARGET_BASE_TOKEN_AMOUNT_ != newBaseTarget) {
         _TARGET_BASE_TOKEN_AMOUNT_ = newBaseTarget;
      }
      if (_R_STATUS_ != newRStatus) {
         _R_STATUS_ = newRStatus;
      }

      _donateBaseToken(lpFeeBase);

      return payQuote;
   }

   // ============ Query Functions ============

   uint256 receiveQuote querySellBaseToken(uint256 amount) {
      (receiveQuote, , , , , ) = _querySellBaseToken(amount);
      return receiveQuote;
   }

   uint256 payQuote queryBuyBaseToken(uint256 amount) {
      (payQuote, , , , , ) = _queryBuyBaseToken(amount);
      return payQuote;
   }

   std::tuple<uint256, uint256, uint256, Types.RStatus, uint256, uint256> _querySellBaseToken(uint256 amount)

   {
      uint256       receiveQuote;
      uint256       lpFeeQuote;
      uint256       mtFeeQuote;
      Types.RStatus newRStatus;
      uint256       newQuoteTarget;
      uint256       newBaseTarget;

      (newBaseTarget, newQuoteTarget) = getExpectedTarget();

      uint256 sellBaseAmount = amount;

      if (_R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         // R falls below one
         receiveQuote = _ROneSellBaseToken(sellBaseAmount, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
      } else if (_R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 backToOnePayBase      = newBaseTarget.sub(_BASE_BALANCE_);
         uint256 backToOneReceiveQuote = _QUOTE_BALANCE_.sub(newQuoteTarget);
         // case 2: R>1
         // complex case, R status depends on trading amount
         if (sellBaseAmount < backToOnePayBase) {
            // case 2.1: R status do not change
            receiveQuote = _RAboveSellBaseToken(sellBaseAmount, _BASE_BALANCE_, newBaseTarget);
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
                backToOneReceiveQuote.add(_ROneSellBaseToken(sellBaseAmount.sub(backToOnePayBase), newQuoteTarget));
            newRStatus = Types::RStatus::BELOW_ONE;
         }
      } else {
         // _R_STATUS_ == Types::RStatus::BELOW_ONE
         // case 3: R<1
         receiveQuote = _RBelowSellBaseToken(sellBaseAmount, _QUOTE_BALANCE_, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
      }

      // count fees
      lpFeeQuote   = DecimalMath.mul(receiveQuote, _LP_FEE_RATE_);
      mtFeeQuote   = DecimalMath.mul(receiveQuote, _MT_FEE_RATE_);
      receiveQuote = receiveQuote.sub(lpFeeQuote).sub(mtFeeQuote);

      return (receiveQuote, lpFeeQuote, mtFeeQuote, newRStatus, newQuoteTarget, newBaseTarget);
   }

   std::tupple<uint256, uint256, uint256, Types.RStatus, uint256, uint256> _queryBuyBaseToken(uint256 amount)

   {
      uint256       payQuote;
      uint256       lpFeeBase;
      uint256       mtFeeBase;
      Types.RStatus newRStatus;
      uint256       newQuoteTarget;
      uint256       newBaseTarget;

      (newBaseTarget, newQuoteTarget) = getExpectedTarget();

      // charge fee from user receive amount
      lpFeeBase             = DecimalMath.mul(amount, _LP_FEE_RATE_);
      mtFeeBase             = DecimalMath.mul(amount, _MT_FEE_RATE_);
      uint256 buyBaseAmount = amount.add(lpFeeBase).add(mtFeeBase);

      if (_R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         payQuote   = _ROneBuyBaseToken(buyBaseAmount, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
      } else if (_R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         // case 2: R>1
         payQuote   = _RAboveBuyBaseToken(buyBaseAmount, _BASE_BALANCE_, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
      } else if (_R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 backToOnePayQuote    = newQuoteTarget.sub(_QUOTE_BALANCE_);
         uint256 backToOneReceiveBase = _BASE_BALANCE_.sub(newBaseTarget);
         // case 3: R<1
         // complex case, R status may change
         if (buyBaseAmount < backToOneReceiveBase) {
            // case 3.1: R status do not change
            // no need to check payQuote because spare base token must be greater than zero
            payQuote   = _RBelowBuyBaseToken(buyBaseAmount, _QUOTE_BALANCE_, newQuoteTarget);
            newRStatus = Types::RStatus::BELOW_ONE;
         } else if (buyBaseAmount == backToOneReceiveBase) {
            // case 3.2: R status changes to ONE
            payQuote   = backToOnePayQuote;
            newRStatus = Types::RStatus::ONE;
         } else {
            // case 3.3: R status changes to ABOVE_ONE
            payQuote = backToOnePayQuote.add(_ROneBuyBaseToken(buyBaseAmount.sub(backToOneReceiveBase), newBaseTarget));
            newRStatus = Types::RStatus::ABOVE_ONE;
         }
      }

      return (payQuote, lpFeeBase, mtFeeBase, newRStatus, newQuoteTarget, newBaseTarget);
   }
};
