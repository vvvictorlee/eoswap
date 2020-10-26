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

#include <storage/HelperTable.hpp>
#include <storage/ImplTable.hpp>
#include <storage/LibTable.hpp>
#include <storage/TokenTable.hpp>
#include <storage/ZooTable.hpp>
const name LP_TOKEN_CONTRACT = "eodoslptoken"_n;
class storage_mgmt {
 private:
   name                   self;
   ZooStorageSingleton    zoo_storage_singleton;
   ZooStorage             zoo_storage;
   DODOStorageSingleton   dodo_storage_singleton;
   DODOStorage            dodo_storage;
   ProxyStorageSingleton  proxy_storage_singleton;
   ProxyStorage           proxy_storage;
   HelperStorageSingleton helper_storage_singleton;
   HelperStorage          helper_storage;
   TokenStorageSingleton  token_storage_singleton;
   TokenStorage           token_storage;

 public:
   storage_mgmt(name _self)
       : self(_self)
       , zoo_storage_singleton(_self, _self.value)
       , dodo_storage_singleton(_self, _self.value)
       , proxy_storage_singleton(_self, _self.value)
       , helper_storage_singleton(_self, _self.value)
       , token_storage_singleton(_self, _self.value) {
      zoo_storage    = zoo_storage_singleton.exists() ? zoo_storage_singleton.get() : ZooStorage{};
      dodo_storage   = dodo_storage_singleton.exists() ? dodo_storage_singleton.get() : DODOStorage{};
      proxy_storage  = proxy_storage_singleton.exists() ? proxy_storage_singleton.get() : ProxyStorage{};
      helper_storage = helper_storage_singleton.exists() ? helper_storage_singleton.get() : HelperStorage{};
      token_storage  = token_storage_singleton.exists() ? token_storage_singleton.get() : TokenStorage{};
   }
   ~storage_mgmt() {
      zoo_storage_singleton.set(zoo_storage, self);
      dodo_storage_singleton.set(dodo_storage, self);
      proxy_storage_singleton.set(proxy_storage, self);
      helper_storage_singleton.set(helper_storage, self);
      token_storage_singleton.set(token_storage, self);
   }

   ZooStorage&    get_zoo_store() { return zoo_storage; }
   DODOStorage&   get_dodo_store() { return dodo_storage; }
   ProxyStorage&  get_proxy_store() { return proxy_storage; }
   HelperStorage& get_helper_store() { return helper_storage; }
   TokenStorage&  get_token_store() { return token_storage; }

   DODOStore& get_dodo_store(name dodo_name) {
      auto d = dodo_storage.dodos.find(pool_name);
      bool f = p != dodo_storage.dodos.end();
      require(f, "NO_DODO");
      return p->second;
   }

   DODOTokenStore& get_token_store(namesym token) {
      auto t = token_storage.tokens.find(token);
      bool f = (t != token_storage.tokens.end());

      require(f, "NO_TOKEN");
      return t->second;
   }

   DODOTokenStore& get_lptoken_store(namesym token) {
      auto t = token_storage.lptokens.find(token);
      bool f = (t != token_storage.lptokens.end());

      require(f, "NO_TOKEN");
      return t->second;
   }

   DODOTokenStore& newLpTokenStore(const extended_symbol& token) {
      extended_symbol esym       = extended_symbol(token.get_symbol(), LP_TOKEN_CONTRACT);
      namesym         token_name = to_namesym(esym);
      auto            p          = _pool_storage.lptokens.find(token_name);
      bool            f          = (p == _pool_storage.lptokens.end());
      require(f, "ALREADY_EXIST_LPTOKEN");
      DODOTokenStore t;
      t.esymbol     = esym;
      t.originToken = token;
      auto pb       = _pool_storage.lptokens.insert(std::map<namesym, DODOTokenStore>::value_type(token_name, t));
      require(pb.second, "INSERT_LPTOKEN_FAIL");

      return pb.first->second;
   }

   DODOTokenStore& newTokenStore(const extended_symbol& token) {
      auto t = _token_storage.tokens.find(token);

      bool f = (t == _token_storage.tokens.end());
      require(f, "ALREADY_EXIST_TOKEN");

      auto pb = _token_storage.tokens.insert(std::map<namesym, DODOTokenStore>::value_type(token, DODOTokenStore()));
      require(pb.second, "INSERT_TOKEN_FAIL");
      return pb.first->second;
   }
};