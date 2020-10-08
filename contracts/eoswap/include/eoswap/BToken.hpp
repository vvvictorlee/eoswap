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
#include <eoswap/BNum.hpp>
#include <storage/BTokenTable.hpp>

// Highly opinionated token implementation

class IERC20 {
public:
  virtual uint totalSupply() = 0;
  virtual uint balanceOf(name whom) = 0;
  virtual uint allowance(name src, name dst) = 0;

  virtual bool approve(name dst, uint amt) = 0;
  virtual bool transfer(name dst, uint amt) = 0;
  virtual bool transferFrom(name src, name dst, uint amt) = 0;
};

template <typename TokenStoreType> class BTokenBase : public BNum {
private:
  name self;
  name msg_sender;
  BTokenStore &token_store;

public:
  BTokenBase(name _self, TokenStoreType &_token_store)
      : self(_self), token_store(_token_store) {}
  ~BTokenBase() {}

  void auth(name _msg_sender) {
    require_auth(_msg_sender);
    msg_sender = _msg_sender;
  }

  name get_self() { return self; }
  name get_msg_sender() { return msg_sender; }
  BTokenStore &get_token_store() { return token_store; }
  void _mint(uint amt) {
    token_store.balance[msg_sender] =
        badd(token_store.balance[msg_sender], amt);
    token_store.totalSupply = badd(token_store.totalSupply, amt);
  }

  void _burn(uint amt) {
    require(token_store.balance[msg_sender] >= amt, "ERR_INSUFFICIENT_BAL");
    token_store.balance[msg_sender] =
        bsub(token_store.balance[msg_sender], amt);
    token_store.totalSupply = bsub(token_store.totalSupply, amt);
  }

  void _move(name src, name dst, uint amt) {
    require(token_store.balance[src] >= amt, "ERR_INSUFFICIENT_BAL");
    token_store.balance[src] = bsub(token_store.balance[src], amt);
    token_store.balance[dst] = badd(token_store.balance[dst], amt);
  }

  void _push(name to, uint amt) { _move(msg_sender, to, amt); }

  void _pull(name from, uint amt) { _move(from, msg_sender, amt); }
};

template <typename TokenStoreType>
class BToken : public BTokenBase<TokenStoreType>, public IERC20 {
public:
  BToken(name _self, TokenStoreType &token_store)
      : BTokenBase<TokenStoreType>(_self, token_store) {}

  std::string _name = "Balancer Pool Token";
  std::string _symbol = "BPT";
  uint8 _decimals = 18;

  std::string namestring() { return _name; }

  std::string symbol() { return _symbol; }

  uint8 decimals() { return _decimals; }

  virtual uint allowance(name src, name dst) override {
    return BTokenBase<TokenStoreType>::get_token_store()
        .allowance[src]
        .dst2amt[dst];
  }

  virtual uint balanceOf(name whom) override {
    return BTokenBase<TokenStoreType>::get_token_store().balance[whom];
  }

  virtual uint totalSupply() override {
    return BTokenBase<TokenStoreType>::get_token_store().totalSupply;
  }

  virtual bool approve(name dst, uint amt) override {
    BTokenBase<TokenStoreType>::get_token_store()
        .allowance[BTokenBase<TokenStoreType>::get_msg_sender()]
        .dst2amt[dst] = amt;
    return true;
  }

  bool increaseApproval(name dst, uint amt) {
    BTokenBase<TokenStoreType>::get_token_store()
        .allowance[BTokenBase<TokenStoreType>::get_msg_sender()]
        .dst2amt[dst] = BTokenBase<TokenStoreType>::badd(
        BTokenBase<TokenStoreType>::get_token_store()
            .allowance[BTokenBase<TokenStoreType>::get_msg_sender()]
            .dst2amt[dst],
        amt);
    return true;
  }

  bool decreaseApproval(name dst, uint amt) {
    uint oldValue = BTokenBase<TokenStoreType>::get_token_store()
                        .allowance[BTokenBase<TokenStoreType>::get_msg_sender()]
                        .dst2amt[dst];
    if (amt > oldValue) {
      BTokenBase<TokenStoreType>::get_token_store()
          .allowance[BTokenBase<TokenStoreType>::get_msg_sender()]
          .dst2amt[dst] = 0;
    } else {
      BTokenBase<TokenStoreType>::get_token_store()
          .allowance[BTokenBase<TokenStoreType>::get_msg_sender()]
          .dst2amt[dst] = BTokenBase<TokenStoreType>::bsub(oldValue, amt);
    }
    return true;
  }

  virtual bool transfer(name dst, uint amt) override {
    BTokenBase<TokenStoreType>::_move(
        BTokenBase<TokenStoreType>::get_msg_sender(), dst, amt);
    return true;
  }

  virtual bool transferFrom(name src, name dst, uint amt) override {
    require(BTokenBase<TokenStoreType>::get_msg_sender() == src ||
                amt <=
                    BTokenBase<TokenStoreType>::get_token_store()
                        .allowance[src]
                        .dst2amt[BTokenBase<TokenStoreType>::get_msg_sender()],
            "ERR_BTOKEN_BAD_CALLER");
    BTokenBase<TokenStoreType>::_move(src, dst, amt);
    if (BTokenBase<TokenStoreType>::get_msg_sender() != src &&
        BTokenBase<TokenStoreType>::get_token_store()
                .allowance[src]
                .dst2amt[BTokenBase<TokenStoreType>::get_msg_sender()] !=
            uint(-1)) {
      BTokenBase<TokenStoreType>::get_token_store()
          .allowance[src]
          .dst2amt[BTokenBase<TokenStoreType>::get_msg_sender()] =
          BTokenBase<TokenStoreType>::bsub(
              BTokenBase<TokenStoreType>::get_token_store()
                  .allowance[src]
                  .dst2amt[BTokenBase<TokenStoreType>::get_msg_sender()],
              amt);
    }
    return true;
  }
};
