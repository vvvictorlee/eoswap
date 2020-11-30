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
#include <common/transfer_mgmt.hpp>
#include <eoswap/BPool.hpp>
#include <storage/BFactoryTable.hpp>
class BFactory : public BBronze {
 private:
   name             self;
   name             msg_sender;
   storage_mgmt     _storage_mgmt;
   transfer_mgmt    _transfer_mgmt;
   BFactoryStorage& _factory_storage;

 public:
   BFactory(name _self)
       : self(_self)
       , _storage_mgmt(_self)
       , _transfer_mgmt(_self)
       , _factory_storage(_storage_mgmt.get_factory_store()) {
      _factory_storage.blabs = self;
   }
   ~BFactory() {}
   storage_mgmt&  get_storage_mgmt() { return _storage_mgmt; }
   transfer_mgmt& get_transfer_mgmt() { return _transfer_mgmt; }
   void           setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }

   bool isBPool(name pool) { return _factory_storage.isBPool[pool]; }

   void newBPool(name pool_name) {
      const BPoolStore&  poolStore  = _storage_mgmt.newPool(msg_sender,pool_name);
      BTokenStore& tokenStore = _storage_mgmt.newTokenStore(to_namesym(extended_symbol(symbol("BPT", 4), pool_name)));
      BPool        pool(self, *this, pool_name,poolStore, tokenStore);
      pool.init();
      pool.auth(msg_sender);
      _factory_storage.isBPool[pool_name] = true;
      pool.setController(msg_sender);
   }

   void newToken(const extended_asset& tokenx) {
      const extended_symbol& exsym      = tokenx.get_extended_symbol();
      const symbol&          sym        = exsym.get_symbol();
      namesym                token      = to_namesym(exsym);
      BTokenStore&           tokenStore = _storage_mgmt.newTokenStore(token);
      BToken otoken(self, tokenStore, exsym.get_contract().to_string(), sym.code().to_string(), sym.precision());
      otoken.auth(msg_sender);
   }

   template <typename T>
   void pool(name pool_name, T func) {
      const BPoolStore&  poolStore  = _storage_mgmt.get_pool(pool_name);
      BTokenStore& tokenStore = _storage_mgmt.get_token_store(to_namesym(extended_symbol(symbol("BPT", 4), pool_name)));
      BPool        pool(self, *this, pool_name,poolStore, tokenStore);
      pool.auth(msg_sender);
      func(pool);
   }

   template <typename T>
   void token(namesym token, T func) {
      BTokenStore& tokenStore = _storage_mgmt.get_token_store(token);
      BToken       otoken(self, tokenStore);
      otoken.auth(msg_sender);
      func(otoken);
   }

   name getBLabs() { return _factory_storage.blabs; }

   void setBLabs(name blabs) {
      require_auth(msg_sender);
      require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
      _factory_storage.blabs = blabs;
   }

   void collect(name pool_name) {
      require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
      const BPoolStore&  poolStore  = _storage_mgmt.get_pool(pool_name);
      BTokenStore& tokenStore = _storage_mgmt.get_token_store(to_namesym(extended_symbol(symbol("BPT", 4), pool_name)));
      BPool        pool(self, *this,pool_name, poolStore, tokenStore);
      pool.auth(msg_sender);
      uint collected = pool.balanceOf(self);
      bool xfer      = pool.transfer(_factory_storage.blabs, collected);
      require(xfer, "ERR_ERC20_FAILED");
   }
};
