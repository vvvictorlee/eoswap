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
class Trader : virtual public Storage, virtual public Pricing, virtual public Settlement {
 private:
   DODOStore& stores;

 public:
   Trader(DODOStore& _stores, IFactory& _factory)
       : stores(_stores)
       , Storage(_stores, _factory)
       , Pricing(_stores, _factory)
       , Settlement(_stores, _factory) {}
   // ============ Events ============

   void tradeAllowed() { require(stores._TRADE_ALLOWED_, "TRADE_NOT_ALLOWED"); }

   void buyingAllowed() { require(stores._BUYING_ALLOWED_, "BUYING_NOT_ALLOWED"); }

   void sellingAllowed() { require(stores._SELLING_ALLOWED_, "SELLING_NOT_ALLOWED"); }

   void gasPriceLimit() { require(tx_gasprice <= stores._GAS_PRICE_LIMIT_, "GAS_PRICE_EXCEED"); }

// #define TEST_DATA
   // ============ Trade Functions ============
   uint64_t sellBaseToken(uint64_t amount, uint64_t minReceiveQuote, bytes data) {


//   _R_STATUS_: 2,
//   _TARGET_BASE_TOKEN_AMOUNT_: '994358158970',
//   _TARGET_QUOTE_TOKEN_AMOUNT_: '739478001769',
//   _BASE_BALANCE_: '1550541851660',
//   _QUOTE_BALANCE_: '327954293302'

      tradeAllowed();
      sellingAllowed();
      gasPriceLimit();
      //   //preventReentrant();
      // query price
      uint256 receiveQuote;
      uint256 lpFeeQuote;
      uint256 mtFeeQuote;
      uint8_t newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(receiveQuote, lpFeeQuote, mtFeeQuote, newRStatus, newQuoteTarget, newBaseTarget) =
          _querySellBaseToken(amount);

      require(
          receiveQuote >= minReceiveQuote,
          "SELL_BASE_RECEIVE_NOT_ENOUGH" + std::to_string(static_cast<uint64_t>(receiveQuote)));

      // settle assets
      _quoteTokenTransferOut(getMsgSender(), extended_asset(static_cast<uint64_t>(receiveQuote), stores._QUOTE_TOKEN_));
      if (data.size() > 0) {
         //  IDODOCallee(getMsgSender()).dodoCall(false, amount, receiveQuote, data);
      }

      my_print_f(
          ">>>>> =====SsellBaseToken stores._BASE_BALANCE_=%,stores._TARGET_BASE_TOKEN_AMOUNT_=%,newBaseTarget=%, "
          "amount=%=; ",
          stores._BASE_BALANCE_, stores._TARGET_BASE_TOKEN_AMOUNT_, newBaseTarget, amount);

      _baseTokenTransferIn(getMsgSender(), extended_asset(amount, stores._BASE_TOKEN_));
      if (mtFeeQuote != 0) {
         _quoteTokenTransferOut(
             stores._MAINTAINER_, extended_asset(static_cast<uint64_t>(mtFeeQuote), stores._QUOTE_TOKEN_));
      }

      // update TARGET
      if (stores._TARGET_QUOTE_TOKEN_AMOUNT_ != newQuoteTarget) {
         stores._TARGET_QUOTE_TOKEN_AMOUNT_ = newQuoteTarget;
      }
      if (stores._TARGET_BASE_TOKEN_AMOUNT_ != newBaseTarget) {
         my_print_f(
             ">>>>> $$$$$SsellBaseToken stores._BASE_BALANCE_=%,stores._TARGET_BASE_TOKEN_AMOUNT_=%,newBaseTarget=%, "
             "newQuoteTarget=%=; ",
             stores._BASE_BALANCE_, stores._TARGET_BASE_TOKEN_AMOUNT_, newBaseTarget, newQuoteTarget);

         stores._TARGET_BASE_TOKEN_AMOUNT_ = newBaseTarget;
      }
      if (stores._R_STATUS_ != newRStatus) {
         stores._R_STATUS_ = newRStatus;
      }

      _donateQuoteToken(lpFeeQuote);

      return static_cast<uint64_t>(receiveQuote);
   }

   uint256 buyBaseToken(uint256 amount, uint256 maxPayQuote, bytes data) {
// #ifdef TEST_DATA
//       stores._R_STATUS_                  = 2;
//       stores._TARGET_BASE_TOKEN_AMOUNT_  = 994358158970;
//       stores._TARGET_QUOTE_TOKEN_AMOUNT_ = 739478001769;
//       stores._BASE_BALANCE_              = 1550541851660;
//       stores._QUOTE_BALANCE_ = 327954293302;
// #endif

      tradeAllowed();
      buyingAllowed();
      gasPriceLimit();
      //   //preventReentrant();
      // query price
      uint256 payQuote;
      uint256 lpFeeBase;
      uint256 mtFeeBase;
      uint8_t newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(payQuote, lpFeeBase, mtFeeBase, newRStatus, newQuoteTarget, newBaseTarget) = _queryBuyBaseToken(amount);
      require(
          payQuote <= maxPayQuote, std::to_string(static_cast<uint64_t>(payQuote)) + "BUY_BASE_COST_TOO_MUCH" +
                                       std::to_string(static_cast<uint64_t>(maxPayQuote)));

      // settle assets
      _baseTokenTransferOut(getMsgSender(), extended_asset(amount, stores._BASE_TOKEN_));
      if (data.size() > 0) {
         //  IDODOCallee(getMsgSender()).dodoCall(true, amount, payQuote, data);
      }
      _quoteTokenTransferIn(getMsgSender(), extended_asset(static_cast<uint64_t>(payQuote), stores._QUOTE_TOKEN_));
      if (mtFeeBase != 0) {
         _baseTokenTransferOut(
             stores._MAINTAINER_, extended_asset(static_cast<uint64_t>(mtFeeBase), stores._BASE_TOKEN_));
      }

      // update TARGET
      if (stores._TARGET_QUOTE_TOKEN_AMOUNT_ != newQuoteTarget) {
         stores._TARGET_QUOTE_TOKEN_AMOUNT_ = newQuoteTarget;
      }

      if (stores._TARGET_BASE_TOKEN_AMOUNT_ != newBaseTarget) {
         stores._TARGET_BASE_TOKEN_AMOUNT_ = newBaseTarget;
      }
      if (stores._R_STATUS_ != newRStatus) {
         stores._R_STATUS_ = newRStatus;
      }

      _donateBaseToken(lpFeeBase);

      return static_cast<uint64_t>(payQuote);
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
      uint256 payQuote                                                                    = 0;
      std::tie(payQuote, std::ignore, std::ignore, std::ignore, std::ignore, std::ignore) = _queryBuyBaseToken(amount);
      return payQuote;
   }

   std::tuple<uint256, uint256, uint256, uint8_t, uint256, uint256> _querySellBaseToken(uint256 amount) {
      uint256 receiveQuote;
      uint256 lpFeeQuote;
      uint256 mtFeeQuote;
      uint8_t newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(newBaseTarget, newQuoteTarget) = getExpectedTarget();

      my_print_f(
          ">>>>>begin _querySellBaseToken stores._BASE_BALANCE_=%,stores._QUOTE_BALANCE_=%,newBaseTarget=%, "
          "newQuoteTarget=%=; ",
          stores._BASE_BALANCE_, stores._QUOTE_BALANCE_, newBaseTarget, newQuoteTarget);

      uint256 sellBaseAmount = amount;

      if (stores._R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         // R falls below one
         my_print_f(">>>>>>[[[[[[1]]]]]] _querySellBaseToken stores._R_STATUS_ == Types::RStatus::ONE");
         receiveQuote = _ROneSellBaseToken(sellBaseAmount, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
      } else if (stores._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 backToOnePayBase      = sub(newBaseTarget, stores._BASE_BALANCE_);
         uint256 backToOneReceiveQuote = sub(stores._QUOTE_BALANCE_, newQuoteTarget);
         my_print_f(
             ">>>>>>[[[[2]]]] _querySellBaseToken stores._R_STATUS_ == "
             "Types::RStatus::ABOVE_ONE:backToOnePayBase=%=,backToOneReceiveQuote=%=; ",
             backToOnePayBase, backToOneReceiveQuote);

         // case 2: R>1
         // complex case, R status depends on trading amount
         if (sellBaseAmount < backToOnePayBase) {
            // case 2.1: R status do not change
            receiveQuote = _RAboveSellBaseToken(sellBaseAmount, stores._BASE_BALANCE_, newBaseTarget);
            newRStatus   = Types::RStatus::ABOVE_ONE;
            my_print_f(">>>>>>[[[[2.1]]]] _querySellBaseToken sellBaseAmount < backToOnePayBase");

            if (receiveQuote > backToOneReceiveQuote) {
               // [Important corner case!] may enter this branch when some precision problem happens. And consequently
               // contribute to negative spare quote amount to make sure spare quote>=0, mannually set
               // receiveQuote=backToOneReceiveQuote
               receiveQuote = backToOneReceiveQuote;
               my_print_f(">>>>>>[[[[2.1.1]]]] _querySellBaseToken receiveQuote > backToOneReceiveQuote");
            }

         } else if (sellBaseAmount == backToOnePayBase) {
            // case 2.2: R status changes to ONE
            receiveQuote = backToOneReceiveQuote;
            newRStatus   = Types::RStatus::ONE;
            my_print_f(">>>>>>[[[[2.2]]]] _querySellBaseToken sellBaseAmount == backToOnePayBase");
         } else {
            // case 2.3: R status changes to BELOW_ONE
            receiveQuote =
                add(backToOneReceiveQuote, _ROneSellBaseToken(sub(sellBaseAmount, backToOnePayBase), newQuoteTarget));
            newRStatus = Types::RStatus::BELOW_ONE;
            my_print_f(">>>>>>[[[[[2.3]]]]] _querySellBaseToken sellBaseAmount > backToOnePayBase");
            my_print_f(
                ">>>>>>_querySellBaseToken sellBaseAmount > "
                "backToOnePayBase:receiveQuote=%,backToOneReceiveQuote=%,sub(sellBaseAmount, "
                "backToOnePayBase)=%,newQuoteTarget=%;||",
                receiveQuote, backToOneReceiveQuote, sub(sellBaseAmount, backToOnePayBase), newQuoteTarget);
         }
      } else {
         // _R_STATUS_ == Types::RStatus::BELOW_ONE
         // case 3: R<1
         receiveQuote = _RBelowSellBaseToken(sellBaseAmount, stores._QUOTE_BALANCE_, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
         my_print_f(">>>>>>[[[[3]]]] _querySellBaseToken _R_STATUS_ == Types::RStatus::BELOW_ONE");
      }

      // count fees
      lpFeeQuote   = DecimalMath::mul(receiveQuote, stores._LP_FEE_RATE_);
      mtFeeQuote   = DecimalMath::mul(receiveQuote, stores._MT_FEE_RATE_);

      receiveQuote = sub(sub(receiveQuote, lpFeeQuote), mtFeeQuote);

      //transferfee
      extended_asset amountx = extended_asset(static_cast<uint64_t>(receiveQuote), stores._QUOTE_TOKEN_);
      int64_t        transfer_fee = transfer_mgmt::get_transfer_fee(amountx,false);
      receiveQuote -= transfer_fee;

      return std::make_tuple(receiveQuote, lpFeeQuote, mtFeeQuote, newRStatus, newQuoteTarget, newBaseTarget);
   }

   std::tuple<uint256, uint256, uint256, uint8_t, uint256, uint256> _queryBuyBaseToken(uint256 amount) {
      uint256 payQuote;
      uint256 lpFeeBase;
      uint256 mtFeeBase;
      uint8_t newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(newBaseTarget, newQuoteTarget) = getExpectedTarget();
      my_print_f(
          ">>>>>begin _queryBuyBaseToken stores._BASE_BALANCE_=%,stores._QUOTE_BALANCE_=%,newBaseTarget=%, "
          "newQuoteTarget=%=; ",
          stores._BASE_BALANCE_, stores._QUOTE_BALANCE_, newBaseTarget, newQuoteTarget);

      // charge fee from user receive amount
      lpFeeBase             = DecimalMath::mul(amount, stores._LP_FEE_RATE_);
      mtFeeBase             = DecimalMath::mul(amount, stores._MT_FEE_RATE_);
      uint256 buyBaseAmount = add(add(amount, lpFeeBase), mtFeeBase);

      //transferfee
      extended_asset amountx = extended_asset(static_cast<uint64_t>(amount), stores._BASE_TOKEN_);
      int64_t        transfer_fee = transfer_mgmt::get_transfer_fee(amountx);
      buyBaseAmount += transfer_fee;

      if (stores._R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         payQuote   = _ROneBuyBaseToken(buyBaseAmount, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
         my_print_f(">>>>>>[[[[1]]]] _queryBuyBaseToken _R_STATUS_ == Types::RStatus::ONE");
      } else if (stores._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         // case 2: R>1
         payQuote   = _RAboveBuyBaseToken(buyBaseAmount, stores._BASE_BALANCE_, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
         my_print_f(">>>>>>[[[[2]]]] _queryBuyBaseToken _R_STATUS_ == Types::RStatus::ABOVE_ONE");
      } else if (stores._R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 backToOnePayQuote    = sub(newQuoteTarget, stores._QUOTE_BALANCE_);
         uint256 backToOneReceiveBase = sub(stores._BASE_BALANCE_, newBaseTarget);
         my_print_f(">>>>>> [[[[3]]]] _queryBuyBaseToken _R_STATUS_ == Types::RStatus::BELOW_ONE==buyBaseAmount=%,backToOnePayQuote=%,backToOneReceiveBase=%,==",buyBaseAmount,backToOnePayQuote,backToOneReceiveBase);
         // case 3: R<1
         // complex case, R status may change
         if (buyBaseAmount < backToOneReceiveBase) {
            // case 3.1: R status do not change
            // no need to check payQuote because spare base token must be greater than zero
            payQuote   = _RBelowBuyBaseToken(buyBaseAmount, stores._QUOTE_BALANCE_, newQuoteTarget);
            newRStatus = Types::RStatus::BELOW_ONE;
            my_print_f(">>>>>>[[[[3.1]]]] _queryBuyBaseToken _R_STATUS_ == buyBaseAmount < backToOneReceiveBase");
         } else if (buyBaseAmount == backToOneReceiveBase) {
            // case 3.2: R status changes to ONE
            payQuote   = backToOnePayQuote;
            newRStatus = Types::RStatus::ONE;
            my_print_f(">>>>>>[[[[[3.2]]]]] _queryBuyBaseToken buyBaseAmount == backToOneReceiveBase");
         } else {
            // case 3.3: R status changes to ABOVE_ONE
            payQuote =
                add(backToOnePayQuote, _ROneBuyBaseToken(sub(buyBaseAmount, backToOneReceiveBase), newBaseTarget));
            newRStatus = Types::RStatus::ABOVE_ONE;
            my_print_f(">>>>>>*****[[[[3.3]]]]**** _queryBuyBaseToken buyBaseAmount > backToOneReceiveBase");
         }
      }

      return std::make_tuple(payQuote, lpFeeBase, mtFeeBase, newRStatus, newQuoteTarget, newBaseTarget);
   }
};
