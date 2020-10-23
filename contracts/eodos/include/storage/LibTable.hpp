
#pragma once
//////
#include <common/defines.hpp>

struct [
    [eosio::table("factorystore"), eosio::contract("eoswap")]] InitializableOwnableStorage {
   address _OWNER_;
   address _NEW_OWNER_;

  EOSLIB_SERIALIZE(InitializableOwnableStorage, (blabs)(isBPool))
};

struct [
    [eosio::table("factorystore"), eosio::contract("eoswap")]] OwnableStorage {
   address _OWNER_;
   address _NEW_OWNER_;

  EOSLIB_SERIALIZE(OwnableStorage, (blabs)(isBPool))
};


struct [
    [eosio::table("factorystore"), eosio::contract("eoswap")]] ReentrancyGuardStorage {
 bool private _ENTERED_;

  EOSLIB_SERIALIZE(ReentrancyGuardStorage, (blabs)(isBPool))
};

typedef eosio::singleton<"factorystore"_n, BFactoryStorage>
    BFactoryStorageSingleton;