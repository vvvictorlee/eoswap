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
#include <common/instance_mgmt.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
#include <eoswap/BPool.hpp>
#include <storage/BFactoryTable.hpp>

class BFactory : public BBronze {
 private:
   name             self;
   name             msg_sender;
   IFactory&        factory;
   BFactoryStorage& _factory_storage;

 public:
   BFactory(name _self, IFactory& _factory)
       : self(_self)
       , factory(_factory)
       , _factory_storage(factory.get_storage_mgmt().get_factory_store()) {
      _factory_storage.blabs = self;
   }
   ~BFactory() {}
   void setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }
   void enableBPool(name pool_name) { _factory_storage.isBPool[pool_name] = true; }

   bool isBPool(name pool) { return _factory_storage.isBPool[pool]; }

   name getBLabs() { return _factory_storage.blabs; }

   void setBLabs(name blabs) {
      require_auth(msg_sender);
      require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
      _factory_storage.blabs = blabs;
   }

   void collect(name pool_name) {
      require(msg_sender == _factory_storage.blabs, "ERR_NOT_BLABS");
      const BPoolStore& poolStore = factory.get_storage_mgmt().get_pool(pool_name);
      extended_symbol   bpt       = extended_symbol(symbol("BPT", 4), pool_name);
      BPool             pool(self, bpt, factory, pool_name, poolStore);
      pool.auth(msg_sender);
      uint64_t collected = pool.balanceOf(self);
      pool.set_caller(self);
      if (collected > 0) {
         bool xfer = pool.transfer(_factory_storage.blabs, collected);
         require(xfer, "ERR_ERC20_FAILED");
      }
   }
};
