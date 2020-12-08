// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the dodoied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
#pragma once
#include <common/BType.hpp>
#include <common/IFactory.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
#include <eoswap/BPool.hpp>
class instance_mgmt : public IFactory {
 private:
   name          self;
   name          msg_sender;
   storage_mgmt  _storage_mgmt;
   transfer_mgmt _transfer_mgmt;

 public:
   static constexpr symbol BPT_symbol = symbol(symbol_code("BPT"), 9);
   instance_mgmt(name _self)
       : self(_self)
       , _storage_mgmt(_self)
       , _transfer_mgmt(_self) {}
   ~instance_mgmt() {}
   name get_self() override { return self; }
   void setMsgSender(name _msg_sender) override { msg_sender = _msg_sender; }

   storage_mgmt&  get_storage_mgmt() override { return _storage_mgmt; }
   transfer_mgmt& get_transfer_mgmt() override { return _transfer_mgmt; }
   instance_mgmt& get_instance_mgmt() override { return *this; }

   void newBPool(name pool_name) override {
      const BPoolStore& poolStore = _storage_mgmt.newPool(msg_sender, pool_name);
      extended_symbol   bpt       = extended_symbol(BPT_symbol, pool_name);
      BPool             pool(self, bpt, *this, pool_name, poolStore);
      pool.auth(msg_sender);
      pool.init();
      //   _factory_storage.isBPool[pool_name] = true;
      pool.setController(msg_sender);
   }

   template <typename T>
   void pool(name pool_name, T func) {
      const BPoolStore& poolStore = _storage_mgmt.get_pool(pool_name);
      extended_symbol   bpt       = extended_symbol(BPT_symbol, pool_name);
      BPool             pool(self, bpt, *this, pool_name, poolStore);
      pool.auth(msg_sender);
      func(pool);
   }
};

template <typename T>
void IFactory::pool(name pool_name, T func) {
   get_instance_mgmt().pool(pool_name, func);
}
