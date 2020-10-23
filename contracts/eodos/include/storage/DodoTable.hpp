
#pragma once
#include <common/defines.hpp>

struct DODOEthProxyStore {
   address _DODO_ZOO_;
   address _WETH_;
   EOSLIB_SERIALIZE(DODOEthProxyStore, (_DODO_ZOO_)(_WETH_))
};

struct QuoteToken2Dodo {
   std::map<address, address> q2d;
   EOSLIB_SERIALIZE(QuoteToken2Dodo, (q2d))
};

struct [[eosio::table("zoostore"), eosio::contract("eoswap")]] DODOZooStorage {
   DODOEthProxyStore proxy;
   address _DODO_LOGIC_;
   address _CLONE_FACTORY_;

   address _DEFAULT_SUPERVISOR_;
   //_DODO_REGISTER_[baseToken][quoteToken] = dodo;
   std::map<name, QuoteToken2Dodo> _DODO_REGISTER_;
   std::vector<address>            _DODOs;

   EOSLIB_SERIALIZE(DODOZooStorage, (proxy)(_DODO_LOGIC_)(_CLONE_FACTORY_)(_DEFAULT_SUPERVISOR_)(_DODO_REGISTER_)(_DODOs))
};

typedef eosio::singleton<"zoostore"_n, DODOZooStorage> DODOZooStorageSingleton;
