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

#pragma once
#include <common/BType.hpp>
#include <common/storage_mgmt.hpp>
#include <eoswap/BPool.hpp>
#include <storage/BFactoryTable.hpp>
template <typename PoolStoreType, typename TokenStoreType>
class BFactory : public BBronze {
private:
  name self;
  storage_mgmt _storage_mgmt;
  BFactoryStorage &_factory_storage;

public:
  BFactory(name _self)
      : self(_self), _factory_storage(_storage_mgmt_.get_factory_store()) {
    _factory_storage.blabs = self;
  }
  ~BFactory() {}

  bool isBPool(name pool) { return _factory_storage.isBPool[pool]; }

  void newBPool(name msg_sender, name pool_name) {
    BPoolStore &poolStore = _storage_mgmt_.newPoolsSore(pool_name);
    BTokenStore &tokenStore = _storage_mgmt_.newTokenStore(pool_name);
    BPool pool(self, poolStore, tokenStore);
    _factory_storage.isBPool[pool_name] = true;
    pool.setController(msg_sender);
  }

  template <type T> void pool(name msg_sender, name pool_name, T func) {
    require_auth(msg_sender);
    BPoolStore &poolStore = _storage_mgmt_.get_pool_store(pool_name);
    BTokenStore &tokenStore = _storage_mgmt_.get_token_tore(pool_name);
    BPool pool(self, poolStore, tokenStore);
    func(pool);
  }

  template <type T> void token(name msg_sender, name token, T func) {
    require_auth(msg_sender);
    BTokenStore &tokenStore = _storage_mgmt_.get_token_tore(token);
    BToken token(self, tokenStore);
    func(token);
  }

  storage_mgmt &get_storage_mgmt() { return _storage_mgmt; }

  name getBLabs() { return _factory_storage.blabs; }

  void setBLabs(name msg_sender, name blabs)

  {
    require_auth(msg_sender);
    require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
    _factory_storage.blabs = blabs;
  }

  void collect(name msg_sender, name pool_name) {
    pool.auth(msg_sender, pool_name);
    require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
    uint collected = pool.getPoolToken().balanceOf(self);
    bool xfer = pool.getPoolToken().transfer(msg_sender, _factory_storage.blabs,
                                             collected);
    require(xfer, "ERR_ERC20_FAILED");
  }
};
