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
   oracle_storage_table   oracle_table;

 public:
   storage_mgmt(name _self)
       : self(_self)
       , zoo_storage_singleton(_self, _self.value)
       , dodo_storage_singleton(_self, _self.value)
       , proxy_storage_singleton(_self, _self.value)
       , helper_storage_singleton(_self, _self.value)
       , token_storage_singleton(_self, _self.value)
       , oracle_table(_self, _self.value) {
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
      auto d = dodo_storage.dodos.find(dodo_name);
      bool f = d != dodo_storage.dodos.end();
      require(f, "NO_DODO");
      return d->second;
   }

   TokenStore& get_token_store(const extended_symbol& token) {
      namesym token_name = to_namesym(token);
      auto    t          = token_storage.tokens.find(token_name);
      bool    f          = (t != token_storage.tokens.end());

      require(f, "NO_TOKEN");
      return t->second;
   }

   TokenStore& newTokenStore(const extended_symbol& token) {
      namesym token_name = to_namesym(token);
      auto    t          = token_storage.tokens.find(token_name);
      bool    f          = (t == token_storage.tokens.end());
      require(f, "ALREADY_EXIST_TOKEN");

      auto pb = token_storage.tokens.insert(std::map<namesym, TokenStore>::value_type(token_name, TokenStore()));
      require(pb.second, "INSERT_TOKEN_FAIL");
      return pb.first->second;
   }

   DODOStore& newDodoStore(name dodo_name) {
      auto t = dodo_storage.dodos.find(dodo_name);
      bool f = (t == dodo_storage.dodos.end());
      require(f, "ALREADY_EXIST_DODO");
      auto pb = dodo_storage.dodos.insert(std::map<name, DODOStore>::value_type(dodo_name, DODOStore()));
      require(pb.second, "INSERT_DODO_FAIL");
      return pb.first->second;
   }

   void save_oracle_price(name msg_sender, const extended_symbol& basetoken, const extended_asset& quotetoken) {
      uint64_t    key = get_hash_key(get_checksum256(
          basetoken.get_contract().value, basetoken.get_symbol().raw(),
          quotetoken.get_extended_symbol().get_contract().value, quotetoken.get_extended_symbol().get_symbol().raw()));
 
 auto oracle = oracle_table.find(key);
 if (oracle == oracle_table.end()) {
    oracle_table.emplace(msg_sender, [&](auto& o) {
       o.basetoken  = basetoken;
       o.quotetoken = quotetoken;
    });
 } else {
    oracle_table.modify(oracle, same_payer, [&](auto& o) {
       o.basetoken  = basetoken;
       o.quotetoken = quotetoken;
    });
 }
   }

   uint64_t get_oracle_price(const extended_symbol& basetoken, const extended_symbol& quotetoken) {
      uint64_t key = get_hash_key(get_checksum256(
          basetoken.get_contract().value, basetoken.get_symbol().raw(), quotetoken.get_contract().value,
          quotetoken.get_symbol().raw()));

      auto oracle = oracle_table.find(key);
      check(oracle != oracle_table.end(), "no oracle");
      return oracle->quotetoken.quantity.amount;
   }
};