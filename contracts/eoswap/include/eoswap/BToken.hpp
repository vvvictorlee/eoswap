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

  virtual bool approve(name msg_sender, name dst, uint amt) = 0;
  virtual bool transfer(name msg_sender, name dst, uint amt) = 0;
  virtual bool transferFrom(name msg_sender, name src, name dst, uint amt) = 0;
};

class BTokenBase : public BNum {
private:
  name self;
  name msg_sender;

protected:
  name token;
  BTokenStorageSingleton token_storage_singleton;
  BTokenStorage _token_storage;

public:
  BTokenBase(name _self)
      : self(_self), token_storage_singleton(_self, _self.value)

  {
    _token_storage = token_storage_singleton.exists()
                         ? token_storage_singleton.get()
                         : BTokenStorage{};
  }
  ~BTokenBase() { token_storage_singleton.set(_token_storage, self); }
  void initToken(name _msg_sender, name _token) {
    require_auth(_msg_sender);
    msg_sender = _msg_sender;
    setToken(_token);
  }

  void setToken(name _token) {
    token = _token;
    auto t = _token_storage.tokens.find(token);
    if (t == _token_storage.tokens.end()) {
      _token_storage.tokens.insert(
          std::map<name, BTokenStore>::value_type(token, BTokenStore()));
    }
  }

  void _mint(uint amt) {
    _token_storage.tokens[token].balance[msg_sender] =
        badd(_token_storage.tokens[token].balance[msg_sender], amt);
    _token_storage.tokens[token].totalSupply =
        badd(_token_storage.tokens[token].totalSupply, amt);
    print("***mint****",msg_sender,"==",_token_storage.tokens[token].balance[msg_sender], "--", amt,
          "==token==", token);
  }

  void _burn(uint amt) {
    require(_token_storage.tokens[token].balance[msg_sender] >= amt,
            "ERR_INSUFFICIENT_BAL");
    _token_storage.tokens[token].balance[msg_sender] =
        bsub(_token_storage.tokens[token].balance[msg_sender], amt);
    _token_storage.tokens[token].totalSupply =
        bsub(_token_storage.tokens[token].totalSupply, amt);
  }

  void _move(name src, name dst, uint amt) {
    print("****move*****",src,"->",dst,"===",_token_storage.tokens[token].balance[src], "--", amt,
          "token==", token);
    require(_token_storage.tokens[token].balance[src] >= amt,
            "ERR_INSUFFICIENT_BAL");
    _token_storage.tokens[token].balance[src] =
        bsub(_token_storage.tokens[token].balance[src], amt);
    _token_storage.tokens[token].balance[dst] =
        badd(_token_storage.tokens[token].balance[dst], amt);
  }

  void _push(name to, uint amt) { _move(msg_sender, to, amt); }

  void _pull(name from, uint amt) { _move(from, msg_sender, amt); }
};

class BToken : public BTokenBase, public IERC20 {
public:
  BToken(name _self) : BTokenBase(_self) {}

  std::string _name = "Balancer Pool Token";
  std::string _symbol = "BPT";
  uint8 _decimals = 18;

  std::string namestring() { return _name; }

  std::string symbol() { return _symbol; }

  uint8 decimals() { return _decimals; }

  virtual uint allowance(name src, name dst) override {
    return _token_storage.tokens[token].allowance[src].dst2amt[dst];
  }

  virtual uint balanceOf(name whom) override {
    return _token_storage.tokens[token].balance[whom];
  }

  virtual uint totalSupply() override {
    return _token_storage.tokens[token].totalSupply;
  }

  virtual bool approve(name msg_sender, name dst, uint amt) override {
    _token_storage.tokens[token].allowance[msg_sender].dst2amt[dst] = amt;
    return true;
  }

  bool increaseApproval(name msg_sender, name dst, uint amt) {
    _token_storage.tokens[token].allowance[msg_sender].dst2amt[dst] = badd(
        _token_storage.tokens[token].allowance[msg_sender].dst2amt[dst], amt);
    return true;
  }

  bool decreaseApproval(name msg_sender, name dst, uint amt) {
    uint oldValue =
        _token_storage.tokens[token].allowance[msg_sender].dst2amt[dst];
    if (amt > oldValue) {
      _token_storage.tokens[token].allowance[msg_sender].dst2amt[dst] = 0;
    } else {
      _token_storage.tokens[token].allowance[msg_sender].dst2amt[dst] =
          bsub(oldValue, amt);
    }
    return true;
  }

  virtual bool transfer(name msg_sender, name dst, uint amt) override {
    _move(msg_sender, dst, amt);
    return true;
  }

  virtual bool transferFrom(name msg_sender, name src, name dst,
                            uint amt) override {
    require(
        msg_sender == src ||
            amt <=
                _token_storage.tokens[token].allowance[src].dst2amt[msg_sender],
        "ERR_BTOKEN_BAD_CALLER");
    _move(src, dst, amt);
    if (msg_sender != src &&
        _token_storage.tokens[token].allowance[src].dst2amt[msg_sender] !=
            uint(-1)) {
      _token_storage.tokens[token].allowance[src].dst2amt[msg_sender] = bsub(
          _token_storage.tokens[token].allowance[src].dst2amt[msg_sender], amt);
    }
    return true;
  }
};
