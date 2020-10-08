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
class BFactory : public BBronze {
private:
  name self;
  name msg_sender;
  storage_mgmt _storage_mgmt;
  BFactoryStorage &_factory_storage;

public:
  BFactory(name _self)
      : self(_self), _storage_mgmt(_self),
        _factory_storage(_storage_mgmt.get_factory_store()) {
    _factory_storage.blabs = self;
  }
  ~BFactory() {}

  void setMsgSender(name _msg_sender) {
    msg_sender = _msg_sender;
  }

  bool isBPool(name pool) { return _factory_storage.isBPool[pool]; }

  void newBPool(name pool_name) {
    BPoolStore &poolStore = _storage_mgmt.newPoolStore(pool_name);
    BTokenStore &tokenStore = _storage_mgmt.newTokenStore(pool_name);
    BPool pool(self, *this, poolStore, tokenStore);
    pool.init();
    pool.auth(msg_sender);
    _factory_storage.isBPool[pool_name] = true;
    pool.setController(msg_sender);
  }

  void newToken(name token) {
    BTokenStore &tokenStore = _storage_mgmt.newTokenStore(token);
    BToken otoken(self, tokenStore);
    otoken.auth(msg_sender);
  }

  template <typename T> void pool(name pool_name, T func) {
    BPoolStore &poolStore = _storage_mgmt.get_pool_store(pool_name);
    BTokenStore &tokenStore = _storage_mgmt.get_token_store(pool_name);
    BPool pool(self, *this, poolStore, tokenStore);
    pool.auth(msg_sender);
    func(pool);
  }

  template <typename T> void token(name token, T func) {
    BTokenStore &tokenStore = _storage_mgmt.get_token_store(token);
    BToken otoken(self, tokenStore);
    otoken.auth(msg_sender);
    func(otoken);
  }

  storage_mgmt &get_storage_mgmt() { return _storage_mgmt; }

  name getBLabs() { return _factory_storage.blabs; }

  void setBLabs(name blabs) {
    require_auth(msg_sender);
    require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
    _factory_storage.blabs = blabs;
  }

  void collect(name pool_name) {
    require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
    BPoolStore &poolStore = _storage_mgmt.get_pool_store(pool_name);
    BTokenStore &tokenStore = _storage_mgmt.get_token_store(pool_name);
    BPool pool(self, *this, poolStore, tokenStore);
    pool.auth(msg_sender);
    uint collected = pool.balanceOf(self);
    bool xfer = pool.transfer(_factory_storage.blabs, collected);
    require(xfer, "ERR_ERC20_FAILED");
  }
};
