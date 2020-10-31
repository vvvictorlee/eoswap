
#pragma once
#include <common/defines.hpp>
#include <storage/ImplTable.hpp>
#include <storage/LibTable.hpp>

struct DODOEthProxyStore {
   address         _DODO_ZOO_;
   extended_symbol _WETH_;
   extended_symbol core_symbol;
   EOSLIB_SERIALIZE(DODOEthProxyStore, (_DODO_ZOO_)(_WETH_)(core_symbol))
};

struct QuoteToken2Dodo {
   std::map<namesym, name> q2d;
   EOSLIB_SERIALIZE(QuoteToken2Dodo, (q2d))
};

struct [[eosio::table("dodo"), eosio::contract("eosdos")]] DODOStorage {
   std::map<name, DODOStore> dodos;
   EOSLIB_SERIALIZE(DODOStorage, (dodos))
};

typedef eosio::singleton<"dodo"_n, DODOStorage> DODOStorageSingleton;

struct [[eosio::table("proxy"), eosio::contract("eosdos")]] ProxyStorage {
   DODOEthProxyStore    proxy;
   ReentrancyGuardStore guard;

   EOSLIB_SERIALIZE(ProxyStorage, (proxy)(guard))
};

typedef eosio::singleton<"proxy"_n, ProxyStorage> ProxyStorageSingleton;

struct [[eosio::table("zoo"), eosio::contract("eosdos")]] ZooStorage {
   OwnableStore ownable;
   address      _DODO_LOGIC_;
   address      _CLONE_FACTORY_;

   address _DEFAULT_SUPERVISOR_;
   //_DODO_REGISTER_[baseToken][quoteToken] = dodo;
   std::map<namesym, QuoteToken2Dodo> _DODO_REGISTER_;
   std::vector<address>               _DODOs;

   EOSLIB_SERIALIZE(ZooStorage, (ownable)(_DODO_LOGIC_)(_CLONE_FACTORY_)(_DEFAULT_SUPERVISOR_)(_DODO_REGISTER_)(_DODOs))
};

typedef eosio::singleton<"zoo"_n, ZooStorage> ZooStorageSingleton;
