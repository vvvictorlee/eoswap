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
#include <storage/BFactoryTable.hpp>
#include <eoswap/BPool.hpp>

class BFactory : public BBronze {
private:
  name self;
  BFactoryStorageSingleton factory_storage_singleton;
  BFactoryStorage _factory_storage;


public:
  BFactory(name _self)
      : self(_self),factory_storage_singleton(_self, _self.value)

  {
    _factory_storage = factory_storage_singleton.exists()
                        ? factory_storage_singleton.get()
                        : BFactoryStorage{};
  }
  ~BFactory() { factory_storage_singleton.set(_factory_storage, self); }


    bool isBPool(name owner)
    {
        return _factory_storage.isBPool[owner];
    }

    name newBPool(name msg_sender)
    {
        BPool bpool(self);
        _factory_storage.isBPool[msg_sender] = true;
        bpool.setController(msg_sender,msg_sender);
        return msg_sender;
    }

    void initBFactory(name msg_sender)  {
        _factory_storage.blabs = msg_sender;
    }

    name getBLabs()
    {
        return _factory_storage.blabs;
    }

    void setBLabs(name msg_sender,name blabs)
        
    {
        require(msg_sender== _factory_storage.blabs, "ERR_NOT_factory_storage.blabs");
        _factory_storage.blabs = blabs;
    }

    void collect(name msg_sender,BPool pool)
    {
        require(msg_sender== _factory_storage.blabs, "ERR_NOT_factory_storage.blabs");
        uint collected = pool.balanceOf(self);
        bool xfer = pool.transfer(msg_sender,_factory_storage.blabs, collected);
        require(xfer, "ERR_ERC20_FAILED");
    }
};
