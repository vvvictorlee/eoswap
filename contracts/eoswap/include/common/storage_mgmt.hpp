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
#include <storage/BFactoryTable.hpp>
#include <storage/BPoolTable.hpp>
#include <storage/BTokenTable.hpp>

class storage_mgmt {

private:
  name self;
  BFactoryStorageSingleton factory_storage_singleton;
  BFactoryStorage _factory_storage;
  BPoolStorageSingleton pool_storage_singleton;
  BPoolStorage _pool_storage;
  BTokenStorageSingleton token_storage_singleton;
  BTokenStorage _token_storage;

public:
  storage_mgmt(name _self)
      : self(_self), factory_storage_singleton(_self, _self.value),
        pool_storage_singleton(_self, _self.value),
        token_storage_singleton(_self, _self.value) {
    _factory_storage = factory_storage_singleton.exists()
                           ? factory_storage_singleton.get()
                           : BFactoryStorage{};
    _factory_storage.blabs = _self;
    _pool_storage = pool_storage_singleton.exists()
                        ? pool_storage_singleton.get()
                        : BPoolStorage{};
    _token_storage = token_storage_singleton.exists()
                         ? token_storage_singleton.get()
                         : BTokenStorage{};
  }
  ~storage_mgmt() {
    factory_storage_singleton.set(_factory_storage, self);
    pool_storage_singleton.set(_pool_storage, self);
    token_storage_singleton.set(_token_storage, self);
  }

  BFactoryStorage &get_factory_store() { return _factory_storage; }

  BPoolStore &get_pool_store(name pool_name) {
    auto p = _pool_storage.pools.find(pool_name);
    bool f = p != _pool_storage.pools.end();
    require(f, "NO_POOL");
    return p->second;
  }

  BTokenStore &get_token_store(name token) {
    auto t = _token_storage.tokens.find(token);
    bool f = (t != _token_storage.tokens.end());

    require(f, "NO_TOKEN");
    return t->second;
  }

  BPoolStore &newPoolStore(name pool_name) {
    auto p = _pool_storage.pools.find(pool_name);
    bool f = (p == _pool_storage.pools.end());
    require(f, "ALREADY_EXIST_POOL");

    auto pb = _pool_storage.pools.insert(
        std::map<name, BPoolStore>::value_type(pool_name, BPoolStore()));
    require(pb.second, "INSERT_POOL_FAIL");

    return pb.first->second;
  }

  BTokenStore &newTokenStore(name token) {
    auto t = _token_storage.tokens.find(token);

    bool f = (t == _token_storage.tokens.end());
    require(f, "ALREADY_EXIST_TOKEN");

    auto pb = _token_storage.tokens.insert(
        std::map<name, BTokenStore>::value_type(token, BTokenStore()));

    return pb.first->second;
  }
};