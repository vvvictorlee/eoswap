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
   virtual uint64_t totalSupply()        = 0;
   virtual uint64_t balanceOf(name whom) = 0;

   virtual bool transfer(name dst, uint64_t amt)               = 0;
   virtual bool transferFrom(name src, name dst, uint64_t amt) = 0;
};

class BTokenBase : public BNum {
 private:
   name            self;
   name            caller;
   name            msg_sender;
   extended_symbol ext_symbol;
   extended_token  _extended_token;

 public:
   BTokenBase(name _self, const extended_symbol& _ext_symbol, bool _auth_mode = true)
       : self(_self)
       , ext_symbol(_ext_symbol)
       , _extended_token(_self, _auth_mode) {}
   ~BTokenBase() {}

   void auth(name _msg_sender) {
      require_auth(_msg_sender);
      setMsgSender(_msg_sender);
   }

   void setMsgSender(name _msg_sender) {
      msg_sender = _msg_sender;
      caller     = _msg_sender;
   }
   void            set_caller(name _caller) { caller = _caller; }
   name            get_caller() { return caller; }
   name            get_self() { return self; }
   name            get_msg_sender() { return msg_sender; }
   extended_symbol get_ext_symbol() { return ext_symbol; }
   extended_token  get_ext_token() { return _extended_token; }
   void            createtoken() {
      static const uint64_t MAX_TOTAL_SUPPLY = 1000000000000000;
      _extended_token.create(ext_symbol.get_contract(), extended_asset{MAX_TOTAL_SUPPLY, ext_symbol});
   }
   void create(name issuer, const extended_asset& max_supply) { _extended_token.create(issuer, max_supply); }
   void _mint(uint64_t amt) {
      //   token_store.balance[msg_sender] = badd(token_store.balance[msg_sender], amt);
      //   token_store.totalSupply         = badd(token_store.totalSupply, amt);
      _extended_token.issue(caller, extended_asset(amt, ext_symbol), "");
   }

   void _burn(uint64_t amt) {
      //   require(token_store.balance[msg_sender] >= amt, "ERR_INSUFFICIENT_BAL");
      //   token_store.balance[msg_sender] = bsub(token_store.balance[msg_sender], amt);
      //   token_store.totalSupply         = bsub(token_store.totalSupply, amt);
      _extended_token.retire(extended_asset(amt, ext_symbol), "");
   }

   void _move(name src, name dst, uint64_t amt) {
      //   require(token_store.balance[src] >= amt, "ERR_INSUFFICIENT_BAL");
      //   token_store.balance[src] = bsub(token_store.balance[src], amt);
      //   token_store.balance[dst] = badd(token_store.balance[dst], amt);
      _extended_token.transfer(src, dst, extended_asset(amt, ext_symbol), "");
   }

   void _push(name to, uint64_t amt) { _move(caller, to, amt); }

   void _pull(name from, uint64_t amt) { _move(from, caller, amt); }
};

class BToken : public BTokenBase, public IERC20 {
 public:
   BToken(name _self, const extended_symbol& _ext_symbol, bool _auth_mode = true)
       : BTokenBase(_self, _ext_symbol, _auth_mode) {}

   std::string namestring() { return get_ext_symbol().get_contract().to_string(); }

   std::string symbol() { return get_ext_symbol().get_symbol().code().to_string(); }

   uint8_t decimals() { return get_ext_symbol().get_symbol().precision(); }

   virtual uint64_t balanceOf(name whom) override {
      return get_ext_token().get_balance(get_self(), whom, get_ext_symbol()).amount;
   }

   virtual uint64_t totalSupply() override { return get_ext_token().get_supply(get_self(), get_ext_symbol()).amount; }

   virtual bool transfer(name dst, uint64_t amt) override {
      _move(get_caller(), dst, amt);
      return true;
   }

   virtual bool transferFrom(name src, name dst, uint64_t amt) override {
      _move(src, dst, amt);

      return true;
   }
};
