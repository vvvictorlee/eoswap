
#pragma once
#include <common/defines.hpp>
#include <storage/LibTable.hpp>

struct DODOStore {
   OwnableStore         initownable;
   ReentrancyGuardStore guard;
   // ============ Variables for Control ============

   bool    _INITIALIZED_;
   bool    _CLOSED_;
   bool    _DEPOSIT_QUOTE_ALLOWED_;
   bool    _DEPOSIT_BASE_ALLOWED_;
   bool    _TRADE_ALLOWED_;
   uint256 _GAS_PRICE_LIMIT_;

   // ============ Advanced Controls ============
   bool    _BUYING_ALLOWED_;
   bool    _SELLING_ALLOWED_;
   uint256 _BASE_BALANCE_LIMIT_;
   uint256 _QUOTE_BALANCE_LIMIT_;

   // ============ Core Address ============

   address _SUPERVISOR_; // could freeze system in emergency
   address _MAINTAINER_; // collect maintainer fee to buy food for DODO

   extended_symbol _BASE_TOKEN_;
   extended_symbol _QUOTE_TOKEN_;
   extended_symbol _ORACLE_;

   // ============ Variables for PMM Algorithm ============

   uint256 _LP_FEE_RATE_;
   uint256 _MT_FEE_RATE_;
   uint256 _K_;

   uint8 _R_STATUS_;
   uint256       _TARGET_BASE_TOKEN_AMOUNT_;
   uint256       _TARGET_QUOTE_TOKEN_AMOUNT_;
   uint256       _BASE_BALANCE_;
   uint256       _QUOTE_BALANCE_;


   extended_symbol _BASE_CAPITAL_TOKEN_;
   extended_symbol _QUOTE_CAPITAL_TOKEN_;

   // ============ Variables for Final Settlement ============

   uint256                 _BASE_CAPITAL_RECEIVE_QUOTE_;
   uint256                 _QUOTE_CAPITAL_RECEIVE_BASE_;
   std::map<address, bool> _CLAIMED_;
   EOSLIB_SERIALIZE(
       DODOStore, (initownable)(guard)(_INITIALIZED_)(_CLOSED_)(_DEPOSIT_QUOTE_ALLOWED_)(_DEPOSIT_BASE_ALLOWED_)(_TRADE_ALLOWED_)(
                     _GAS_PRICE_LIMIT_)(_BUYING_ALLOWED_)(_SELLING_ALLOWED_)(_BASE_BALANCE_LIMIT_)(
                    _QUOTE_BALANCE_LIMIT_)(_SUPERVISOR_)(_MAINTAINER_)(_BASE_TOKEN_)(_QUOTE_TOKEN_)(_ORACLE_)(
                    _LP_FEE_RATE_)(_MT_FEE_RATE_)(_K_)(_R_STATUS_)(_TARGET_BASE_TOKEN_AMOUNT_)(
                    _TARGET_QUOTE_TOKEN_AMOUNT_)(_BASE_BALANCE_)(_QUOTE_BALANCE_)(_BASE_CAPITAL_TOKEN_)(
                    _QUOTE_CAPITAL_TOKEN_)(_BASE_CAPITAL_RECEIVE_QUOTE_)(_QUOTE_CAPITAL_RECEIVE_BASE_)(_CLAIMED_))
};

