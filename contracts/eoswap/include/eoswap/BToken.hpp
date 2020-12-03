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
#include <common/extended_token.hpp>
#include <eoswap/BNum.hpp>
#include <storage/BTokenTable.hpp>
// Highly opinionated token implementation

class IERC20 {
 public:
   virtual uint totalSupply()        = 0;
   virtual uint balanceOf(name whom) = 0;

   virtual bool transfer(name dst, uint amt)               = 0;
   virtual bool transferFrom(name src, name dst, uint amt) = 0;
};

class BTokenBase : public BNum {
 private:
   name            self;
   name            msg_sender;
   extended_symbol ext_symbol;
   BTokenStore&    token_store;
   extended_token  _extended_token;

 public:
   BTokenBase(name _self, const extended_symbol& _ext_symbol, BTokenStore& _token_store)
       : self(_self)
       , ext_symbol(_ext_symbol)
       , _extended_token(_self)
       , token_store(_token_store) {}
   ~BTokenBase() {}

   void auth(name _msg_sender) {
      require_auth(_msg_sender);
      msg_sender = _msg_sender;
   }

   name            get_self() { return self; }
   name            get_msg_sender() { return msg_sender; }
   extended_symbol get_ext_symbol() { return ext_symbol; }
   extended_token  get_ext_token() { return _extended_token; }
   BTokenStore&    get_token_store() { return token_store; }
   void            init(extended_asset max_supply) { _extended_token.create(msg_sender, max_supply); }
   void            _mint(uint amt) {
      //   token_store.balance[msg_sender] = badd(token_store.balance[msg_sender], amt);
      //   token_store.totalSupply         = badd(token_store.totalSupply, amt);
      _extended_token.issue(msg_sender, extended_asset(amt, ext_symbol), "");
   }

   void _burn(uint amt) {
      //   require(token_store.balance[msg_sender] >= amt, "ERR_INSUFFICIENT_BAL");
      //   token_store.balance[msg_sender] = bsub(token_store.balance[msg_sender], amt);
      //   token_store.totalSupply         = bsub(token_store.totalSupply, amt);
      _extended_token.retire(extended_asset(amt, ext_symbol), "");
   }

   void _move(name src, name dst, uint amt) {
      //   require(token_store.balance[src] >= amt, "ERR_INSUFFICIENT_BAL");
      //   token_store.balance[src] = bsub(token_store.balance[src], amt);
      //   token_store.balance[dst] = badd(token_store.balance[dst], amt);
      _extended_token.transfer(src, dst, extended_asset(amt, ext_symbol), "");
   }

   void _push(name to, uint amt) { _move(msg_sender, to, amt); }

   void _pull(name from, uint amt) { _move(from, msg_sender, amt); }
};

class BToken : public BTokenBase, public IERC20 {
 public:
   BToken(name _self, const extended_symbol& _ext_symbol, BTokenStore& token_store)
       : BTokenBase(_self, _ext_symbol, token_store) {
      //   get_token_store().names    = _ext_symbol.get_contract().to_string();
      //   get_token_store().symbol   = _ext_symbol.get_symbol().code().to_string();
      //   get_token_store().decimals = _ext_symbol.get_symbol().precision();
   }

   std::string namestring() { return get_ext_symbol().get_contract().to_string(); }

   std::string symbol() { return get_ext_symbol().get_symbol().code().to_string(); }

   uint8 decimals() { return get_ext_symbol().get_symbol().precision(); }

   virtual uint balanceOf(name whom) override {
      return get_ext_token().get_balance(get_self(), whom, get_ext_symbol()).amount;
   }

   virtual uint totalSupply() override { return get_ext_token().get_supply(get_self(), get_ext_symbol()).amount; }

   virtual bool transfer(name dst, uint amt) override {
      _move(get_msg_sender(), dst, amt);
      return true;
   }

   virtual bool transferFrom(name src, name dst, uint amt) override {
      //   require(
      //       get_msg_sender() == src || amt <= get_token_store().allowance[src].dst2amt[get_msg_sender()],
      //       "ERR_BTOKEN_BAD_CALLER");
      //   _move(src, dst, amt);
      //   if (get_msg_sender() != src && get_token_store().allowance[src].dst2amt[get_msg_sender()] != uint(-1)) {
      //      get_token_store().allowance[src].dst2amt[get_msg_sender()] =
      //          bsub(get_token_store().allowance[src].dst2amt[get_msg_sender()], amt);
      //   }
      _move(src, dst, amt);

      return true;
   }
};
