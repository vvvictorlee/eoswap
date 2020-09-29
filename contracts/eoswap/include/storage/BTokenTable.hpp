// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#pragma once

#include <common/BType.hpp>

struct Account2Amt {
  std::map<name, uint> dst2amt; // is token bound to pool

  EOSLIB_SERIALIZE(Account2Amt, (dst2amt))
};

struct BTokenStore {
  std::map<name, uint> balance;
  std::map<name, Account2Amt> allowance;
  uint totalSupply;
  EOSLIB_SERIALIZE(BTokenStore, (balance)(allowance)(totalSupply))
};

struct [[eosio::table("tokentable"), eosio::contract("eoswap")]] BTokenStorage {
  std::map<name, BTokenStore> tokens;
  EOSLIB_SERIALIZE(BTokenStorage, (tokens))
};

typedef eosio::singleton<"tokenstore"_n, BTokenStorage> BTokenStorageSingleton;