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

   void setTestParameters(const std::vector<int64_t>& params) {
      stores._R_STATUS_                  = params[0];
      stores._TARGET_BASE_TOKEN_AMOUNT_  = params[1];
      stores._TARGET_QUOTE_TOKEN_AMOUNT_ = params[2];
      stores._BASE_BALANCE_              = params[3];
      stores._QUOTE_BALANCE_             = params[4];

      // sell
      //   _R_STATUS_: 2,
      //   _TARGET_BASE_TOKEN_AMOUNT_: '994358158970',
      //   _TARGET_QUOTE_TOKEN_AMOUNT_: '739478001769',
      //   _BASE_BALANCE_: '1550541851660',
      //   _QUOTE_BALANCE_: '327954293302'

      //   _R_STATUS_: 1,
      //   _TARGET_BASE_TOKEN_AMOUNT_: '994984853854',
      //   _TARGET_QUOTE_TOKEN_AMOUNT_: '739566362885',
      //   _BASE_BALANCE_: '697161038448',
      //   _QUOTE_BALANCE_: '959965242649'

      // "_R_STATUS_": 1,
      // "_TARGET_BASE_TOKEN_AMOUNT_": "994984160697",
      // "_TARGET_QUOTE_TOKEN_AMOUNT_": "739567242253",
      // "_BASE_BALANCE_": "699158038448",
      // "_QUOTE_BALANCE_": "958488190546"

      // #ifdef TEST_DATA
      //       stores._R_STATUS_                  = 1;
      //       stores._TARGET_BASE_TOKEN_AMOUNT_  = 994984160697;
      //       stores._TARGET_QUOTE_TOKEN_AMOUNT_ = 739567242253;
      //       stores._BASE_BALANCE_              = 699158038448;
      //       stores._QUOTE_BALANCE_             = 958488190546;
      // #endif

      /// buy
      //   _R_STATUS_: 2,
      //   _TARGET_BASE_TOKEN_AMOUNT_: '994358158970',
      //   _TARGET_QUOTE_TOKEN_AMOUNT_: '739566542199',
      //   _BASE_BALANCE_: '1750540351660',
      //   _QUOTE_BALANCE_: '180165525862'
      //   _R_STATUS_: 2,
      //   _TARGET_BASE_TOKEN_AMOUNT_: '994358158970',
      //   _TARGET_QUOTE_TOKEN_AMOUNT_: '739566542199',
      //   _BASE_BALANCE_: '1750540351660',
      //   _QUOTE_BALANCE_: '180165525862'
      // #ifdef TEST_DATA
      //       stores._R_STATUS_                  = 2;
      //       stores._TARGET_BASE_TOKEN_AMOUNT_  = 994358158970;
      //       stores._TARGET_QUOTE_TOKEN_AMOUNT_ = 739566542199;
      //       stores._BASE_BALANCE_              = 1750540351660;
      //       stores._QUOTE_BALANCE_             = 180165525862;
      // #endif
   }
   // ============ Trade Functions ============
   uint64_t sellBaseToken(uint64_t amount, uint64_t minReceiveQuote, bytes data) {

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
      _quoteTokenTransferOut(
          getMsgSender(), extended_asset(static_cast<uint64_t>(receiveQuote), stores._QUOTE_TOKEN_), true);
      if (data.size() > 0) {
         //  IDODOCallee(getMsgSender()).dodoCall(false, amount, receiveQuote, data);
      }

      DODO_DEBUG(
          ">>>>> =====SsellBaseToken stores._BASE_BALANCE_=%,stores._TARGET_BASE_TOKEN_AMOUNT_=%,newBaseTarget=%, "
          "amount=%=; ",
          stores._BASE_BALANCE_, stores._TARGET_BASE_TOKEN_AMOUNT_, newBaseTarget, amount);

      _baseTokenTransferIn(getMsgSender(), extended_asset(amount, stores._BASE_TOKEN_));
      if (mtFeeQuote != 0) {
         _quoteTokenTransferOut(
             stores._MAINTAINER_, extended_asset(static_cast<uint64_t>(mtFeeQuote), stores._QUOTE_TOKEN_), true);
      }

      // update TARGET
      if (stores._TARGET_QUOTE_TOKEN_AMOUNT_ != newQuoteTarget) {
         stores._TARGET_QUOTE_TOKEN_AMOUNT_ = newQuoteTarget;
      }
      if (stores._TARGET_BASE_TOKEN_AMOUNT_ != newBaseTarget) {
         DODO_DEBUG(
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

      tradeAllowed();
      buyingAllowed();
      gasPriceLimit();
      //   //preventReentrant();
      // query price
      uint256 payQuote;
      uint256 lpFeeBase;
      uint256 mtFeeBase;
      uint256 transferFeeBase;
      uint8_t newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(payQuote, lpFeeBase, mtFeeBase, transferFeeBase, newRStatus, newQuoteTarget, newBaseTarget) =
          _queryBuyBaseToken(amount);
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
             stores._MAINTAINER_, extended_asset(static_cast<uint64_t>(mtFeeBase), stores._BASE_TOKEN_), true);
      }

      if (transferFeeBase != 0) {
         _baseTokenTransferFee(transferFeeBase);
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

   uint256 sellQuote(uint256 minReceiveBase, uint256 amountQuote, bytes data) {
      uint256 amount      = DecimalMath::divFloor(amountQuote, getOraclePrice());
      uint256 maxPayQuote = amountQuote;
      my_print_f("===sellQuote==amount=%,getOraclePrice()=%=",amount,getOraclePrice());

      tradeAllowed();
      buyingAllowed();
      gasPriceLimit();
      //   //preventReentrant();
      // query price
      uint256 payQuote;
      uint256 lpFeeBase;
      uint256 mtFeeBase;
      uint256 transferFeeBase;
      uint8_t newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;

      const uint64_t times            = 10; // tries
      const uint64_t actual_diff      = 1;  //
      uint256        quote_price      = 1;
      uint256        previousamount   = amount;
      uint256        previouspayQuote = amountQuote;
      for (int i = 0; i < times; ++i) {
         std::tie(payQuote, lpFeeBase, mtFeeBase, transferFeeBase, newRStatus, newQuoteTarget, newBaseTarget) =
             _queryBuyBaseToken(amount);
         DODO_DEBUG("====payQuote=%,amount=%=====", payQuote, amount);

         if (payQuote == 0 || payQuote == previouspayQuote || payQuote == amountQuote) {
            break;
         }

         quote_price = DecimalMath::divFloor(amount, payQuote);
         DODO_DEBUG("====payQuote=%,amount=%===quote_price=%=", payQuote, amount, quote_price);

         previousamount = amount;

         if (payQuote > amountQuote) {
            uint256 high = payQuote - amountQuote;
            // uint256 newamount = amount- high*(amount/payQuote);
            DODO_DEBUG("====payQuote=%,hi=%===diff=%=", payQuote, high, DecimalMath::mul(high, quote_price));
            amount = amount - DecimalMath::mul(high, quote_price);
         } else if (amountQuote - payQuote > actual_diff) {
            uint256 low = amountQuote - payQuote;
            DODO_DEBUG("====payQuote=%,low=%===diff=%=", payQuote, low, DecimalMath::mul(low, quote_price));
            amount = amount + DecimalMath::mul(low, quote_price);
         }

         previouspayQuote = payQuote;
      }

      if (amountQuote != payQuote) {
         _tokenTransferDiff(amountQuote - payQuote);
         payQuote = amountQuote;
      }

      require(
          amount >= minReceiveBase, std::to_string(static_cast<uint64_t>(amount)) + "BUY_BASE_COST_TOO_MUCH min" +
                                        std::to_string(static_cast<uint64_t>(minReceiveBase)));
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
             stores._MAINTAINER_, extended_asset(static_cast<uint64_t>(mtFeeBase), stores._BASE_TOKEN_), true);
      }

      if (transferFeeBase != 0) {
         _baseTokenTransferFee(transferFeeBase);
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
      uint256 payQuote = 0;
      std::tie(payQuote, std::ignore, std::ignore, std::ignore, std::ignore, std::ignore, std::ignore) =
          _queryBuyBaseToken(amount);
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

      DODO_DEBUG(
          ">>>>>begin _querySellBaseToken stores._BASE_BALANCE_=%,stores._QUOTE_BALANCE_=%,newBaseTarget=%, "
          "newQuoteTarget=%=; ",
          stores._BASE_BALANCE_, stores._QUOTE_BALANCE_, newBaseTarget, newQuoteTarget);

      uint256 sellBaseAmount = amount;

      if (stores._R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         // R falls below one
         DODO_DEBUG("[[[[[[1]]]]]] _querySellBaseToken stores._R_STATUS_ == Types::RStatus::ONE");
         receiveQuote = _ROneSellBaseToken(sellBaseAmount, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
      } else if (stores._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         uint256 backToOnePayBase      = sub(newBaseTarget, stores._BASE_BALANCE_);
         uint256 backToOneReceiveQuote = sub(stores._QUOTE_BALANCE_, newQuoteTarget);
         DODO_DEBUG(
             "[[[[2]]]] _querySellBaseToken stores._R_STATUS_ == "
             "Types::RStatus::ABOVE_ONE:backToOnePayBase=%=,backToOneReceiveQuote=%=; ",
             backToOnePayBase, backToOneReceiveQuote);

         // case 2: R>1
         // complex case, R status depends on trading amount
         if (sellBaseAmount < backToOnePayBase) {
            // case 2.1: R status do not change
            receiveQuote = _RAboveSellBaseToken(sellBaseAmount, stores._BASE_BALANCE_, newBaseTarget);
            newRStatus   = Types::RStatus::ABOVE_ONE;
            DODO_DEBUG("[[[[2.1]]]] _querySellBaseToken sellBaseAmount < backToOnePayBase");

            if (receiveQuote > backToOneReceiveQuote) {
               // [Important corner case!] may enter this branch when some precision problem happens. And consequently
               // contribute to negative spare quote amount to make sure spare quote>=0, mannually set
               // receiveQuote=backToOneReceiveQuote
               receiveQuote = backToOneReceiveQuote;
               DODO_DEBUG("[[[[2.1.1]]]] _querySellBaseToken receiveQuote > backToOneReceiveQuote");
            }

         } else if (sellBaseAmount == backToOnePayBase) {
            // case 2.2: R status changes to ONE
            receiveQuote = backToOneReceiveQuote;
            newRStatus   = Types::RStatus::ONE;
            DODO_DEBUG("[[[[2.2]]]] _querySellBaseToken sellBaseAmount == backToOnePayBase");
         } else {
            // case 2.3: R status changes to BELOW_ONE
            receiveQuote =
                add(backToOneReceiveQuote, _ROneSellBaseToken(sub(sellBaseAmount, backToOnePayBase), newQuoteTarget));
            newRStatus = Types::RStatus::BELOW_ONE;
            DODO_DEBUG("[[[[[2.3]]]]] _querySellBaseToken sellBaseAmount > backToOnePayBase");
            DODO_DEBUG(
                "_querySellBaseToken sellBaseAmount > "
                "backToOnePayBase:receiveQuote=%,backToOneReceiveQuote=%,sub(sellBaseAmount, "
                "backToOnePayBase)=%,newQuoteTarget=%;||",
                receiveQuote, backToOneReceiveQuote, sub(sellBaseAmount, backToOnePayBase), newQuoteTarget);
         }
      } else {
         // _R_STATUS_ == Types::RStatus::BELOW_ONE
         // case 3: R<1
         receiveQuote = _RBelowSellBaseToken(sellBaseAmount, stores._QUOTE_BALANCE_, newQuoteTarget);
         newRStatus   = Types::RStatus::BELOW_ONE;
         DODO_DEBUG("[[[[3]]]] _querySellBaseToken _R_STATUS_ == Types::RStatus::BELOW_ONE");
      }

      // count fees
      lpFeeQuote = DecimalMath::mul(receiveQuote, stores._LP_FEE_RATE_);
      mtFeeQuote = DecimalMath::mul(receiveQuote, stores._MT_FEE_RATE_);

      receiveQuote = sub(sub(receiveQuote, lpFeeQuote), mtFeeQuote);

      return std::make_tuple(receiveQuote, lpFeeQuote, mtFeeQuote, newRStatus, newQuoteTarget, newBaseTarget);
   }

   std::tuple<uint256, uint256, uint256, uint256, uint8_t, uint256, uint256> _queryBuyBaseToken(uint256 amount) {
      uint256 payQuote;
      uint256 lpFeeBase;
      uint256 mtFeeBase;
      uint256 transferFeeBase;
      uint8_t newRStatus;
      uint256 newQuoteTarget;
      uint256 newBaseTarget;
      std::tie(newBaseTarget, newQuoteTarget) = getExpectedTarget();
      DODO_DEBUG(
          ">>>>>beginstores._BASE_BALANCE_=%,stores._QUOTE_BALANCE_=%,newBaseTarget=%, "
          "newQuoteTarget=%=; ",
          stores._BASE_BALANCE_, stores._QUOTE_BALANCE_, newBaseTarget, newQuoteTarget);

      // charge fee from user receive amount
      lpFeeBase = DecimalMath::mul(amount, stores._LP_FEE_RATE_);
      mtFeeBase = DecimalMath::mul(amount, stores._MT_FEE_RATE_);
      // transferfee
      extended_asset amountx = extended_asset(static_cast<uint64_t>(amount), stores._BASE_TOKEN_);
      transferFeeBase        = transfer_mgmt::get_transfer_fee(amountx);
      DODO_DEBUG(
          ">>>>>======transferFeeBase=%========= ",
          transferFeeBase);
      uint256 buyBaseAmount = add(add(add(amount, lpFeeBase), mtFeeBase), transferFeeBase);

      if (stores._R_STATUS_ == Types::RStatus::ONE) {
         // case 1: R=1
         payQuote   = _ROneBuyBaseToken(buyBaseAmount, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
         DODO_DEBUG("[[[[1]]]]_R_STATUS_ == Types::RStatus::ONE");
      } else if (stores._R_STATUS_ == Types::RStatus::ABOVE_ONE) {
         // case 2: R>1
         payQuote   = _RAboveBuyBaseToken(buyBaseAmount, stores._BASE_BALANCE_, newBaseTarget);
         newRStatus = Types::RStatus::ABOVE_ONE;
         DODO_DEBUG("[[[[2]]]]_R_STATUS_ == Types::RStatus::ABOVE_ONE");
      } else if (stores._R_STATUS_ == Types::RStatus::BELOW_ONE) {
         uint256 backToOnePayQuote    = sub(newQuoteTarget, stores._QUOTE_BALANCE_);
         uint256 backToOneReceiveBase = sub(stores._BASE_BALANCE_, newBaseTarget);
         DODO_DEBUG(
             " [[[[3]]]]_R_STATUS_ == "
             "Types::RStatus::BELOW_ONE==buyBaseAmount=%,backToOnePayQuote=%,backToOneReceiveBase=%,==",
             buyBaseAmount, backToOnePayQuote, backToOneReceiveBase);
         // case 3: R<1
         // complex case, R status may change
         if (buyBaseAmount < backToOneReceiveBase) {
            // case 3.1: R status do not change
            // no need to check payQuote because spare base token must be greater than zero
            payQuote   = _RBelowBuyBaseToken(buyBaseAmount, stores._QUOTE_BALANCE_, newQuoteTarget);
            newRStatus = Types::RStatus::BELOW_ONE;
            DODO_DEBUG(" [[[[3.1]]]]  _R_STATUS_ == buyBaseAmount < backToOneReceiveBase=%", buyBaseAmount);
         } else if (buyBaseAmount == backToOneReceiveBase) {
            // case 3.2: R status changes to ONE
            payQuote   = backToOnePayQuote;
            newRStatus = Types::RStatus::ONE;
            DODO_DEBUG("[[[[[3.2]]]]]  buyBaseAmount == backToOneReceiveBase");
         } else {
            // case 3.3: R status changes to ABOVE_ONE
            payQuote =
                add(backToOnePayQuote, _ROneBuyBaseToken(sub(buyBaseAmount, backToOneReceiveBase), newBaseTarget));
            newRStatus = Types::RStatus::ABOVE_ONE;
            DODO_DEBUG("*****[[[[3.3]]]]****  buyBaseAmount > backToOneReceiveBase");
         }
      }

      return std::make_tuple(
          payQuote, lpFeeBase, mtFeeBase, transferFeeBase, newRStatus, newQuoteTarget, newBaseTarget);
   }
};
