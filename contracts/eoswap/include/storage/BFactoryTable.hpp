// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is disstributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// // Builds new BPools, logging their addresses and providing `isBPool(name)
// -> (bool)`
#pragma once
//////
#include <common/BType.hpp>

struct [
    [eosio::table("factorystore"), eosio::contract("eoswap")]] BFactoryStorage {
  name blabs;

  std::map<name, bool> isBPool;

  EOSLIB_SERIALIZE(BFactoryStorage, (blabs)(isBPool))
};

typedef eosio::singleton<"factorystore"_n, BFactoryStorage>
    BFactoryStorageSingleton;