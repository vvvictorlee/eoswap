
#pragma once
#include <common/defines.hpp>




struct DODOLpTokenStore {
   string  symbol = "DLP";
   address originToken;

   uint256 totalSupply;
   mapping(address = > uint256) balances;
   mapping(address = > mapping(address = > uint256)) allowed;


  std::string names;
  std::string symbol ;
  uint8 decimals ;
  std::map<name, uint> balance;
  std::map<name, Account2Amt> allowance;
  uint totalSupply;
  EOSLIB_SERIALIZE(DODOLpTokenStore, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};


struct [[eosio::table("tokenstore"), eosio::contract("eoswap")]] BTokenStorage {
  std::map<namesym, BTokenStore> tokens;
  EOSLIB_SERIALIZE(BTokenStorage, (tokens))
};

typedef eosio::singleton<"tokenstore"_n, BTokenStorage> BTokenStorageSingleton;



struct Store {

    // ============ Variables for Control ============

    bool  _INITIALIZED_;
    bool _CLOSED_;
    bool _DEPOSIT_QUOTE_ALLOWED_;
    bool _DEPOSIT_BASE_ALLOWED_;
    bool _TRADE_ALLOWED_;
   uint256 _GAS_PRICE_LIMIT_;

    // ============ Advanced Controls ============
    bool _BUYING_ALLOWED_;
    bool _SELLING_ALLOWED_;
   uint256 _BASE_BALANCE_LIMIT_;
   uint256 _QUOTE_BALANCE_LIMIT_;

    // ============ Core Address ============

   address _SUPERVISOR_; // could freeze system in emergency
   address _MAINTAINER_; // collect maintainer fee to buy food for DODO

   address _BASE_TOKEN_;
   address _QUOTE_TOKEN_;
   address _ORACLE_;

    // ============ Variables for PMM Algorithm ============

   uint256 _LP_FEE_RATE_;
   uint256 _MT_FEE_RATE_;
   uint256 _K_;

    Types.RStatus public _R_STATUS_;
   uint256 _TARGET_BASE_TOKEN_AMOUNT_;
   uint256 _TARGET_QUOTE_TOKEN_AMOUNT_;
   uint256 _BASE_BALANCE_;
   uint256 _QUOTE_BALANCE_;

   address _BASE_CAPITAL_TOKEN_;
   address _QUOTE_CAPITAL_TOKEN_;

    // ============ Variables for Final Settlement ============

   uint256 _BASE_CAPITAL_RECEIVE_QUOTE_;
   uint256 _QUOTE_CAPITAL_RECEIVE_BASE_;
    mapping(address => bool) _CLAIMED_;
  EOSLIB_SERIALIZE(Store, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};


struct [[eosio::table("tokenstore"), eosio::contract("eoswap")]] BTokenStorage {
  std::map<namesym, BTokenStore> tokens;
  EOSLIB_SERIALIZE(BTokenStorage, (tokens))
};

typedef eosio::singleton<"tokenstore"_n, BTokenStorage> BTokenStorageSingleton;

