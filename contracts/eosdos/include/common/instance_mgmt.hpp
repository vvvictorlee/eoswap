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

#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
#include <eosdos/dodo.hpp>
#include <eosdos/helper/MinimumOracle.hpp>
#include <eosdos/helper/TestERC20.hpp>
#include <eosdos/helper/TestWETH.hpp>
#include <eosdos/impl/DODOLpToken.hpp>
class instance_mgmt : public IFactory {
 private:
   name          self;
   name          msg_sender;
   storage_mgmt  _storage_mgmt;
   transfer_mgmt _transfer_mgmt;

 public:
   instance_mgmt(name _self)
       : self(_self)
       , _storage_mgmt(_self)
       , _transfer_mgmt(_self)
       , IFactory(*this) {}
   ~instance_mgmt() {}
   name           get_self() override { return self; }
   storage_mgmt&  get_storage_mgmt() override { return _storage_mgmt; }
   transfer_mgmt& get_transfer_mgmt() override { return _transfer_mgmt; }
   name           getMsgSender() { return msg_sender; }
   void           setMsgSender(name _msg_sender) { msg_sender = _msg_sender; }

   template <typename T>
   void get_dodo(name dodo_name, T func) {
      DODOStore& dodoStore = _storage_mgmt.get_dodo_store(dodo_name);
      DODO       dodo(dodoStore, *this);
      dodo.setMsgSender(getMsgSender());
      func(dodo);
   }

   template <typename T>
   void get_lptoken(const extended_symbol& lptoken, T func) {
      TokenStore& lptokenStore  = _storage_mgmt.get_token_store(lptoken);
      TokenStore& olptokenStore = _storage_mgmt.get_token_store(lptokenStore.originToken);
      DODOLpToken token(lptokenStore, olptokenStore);
      token.setMsgSender(getMsgSender());
      func(token);
   }

   template <typename TT, typename T>
   void get_token(const extended_symbol& _token, T func) {
      TokenStore& tokenStore = _storage_mgmt.get_token_store(_token);
      TT          token(tokenStore);
      token.setMsgSender(getMsgSender());
      func(token);
   }

   template <typename T>
   void get_ethtoken(const extended_symbol& _token, T func) {
      TokenStore& tokenStore = _storage_mgmt.get_token_store(_token);
      WETH9       token(tokenStore);
      token.setMsgSender(getMsgSender());
      func(token);
   }

   template <typename T>
   void get_oracle(const extended_symbol& oracle, T func) {
      OracleStore&  oracleStore = _storage_mgmt.get_oracle_store(oracle);
      MinimumOracle minioracle(oracleStore);
      minioracle.setMsgSender(getMsgSender());
      func(minioracle);
   }

   void newDODO(
       name dodo_name, address owner, address supervisor, address maintainer, const extended_symbol& baseToken,
       const extended_symbol& quoteToken, const extended_symbol& oracle, uint256 lpFeeRate, uint256 mtFeeRate,
       uint256 k, uint256 gasPriceLimit) {
      DODOStore& dodoStore = _storage_mgmt.newDodoStore(dodo_name);
      DODO       dodo(dodoStore, *this);
      dodo.setMsgSender(getMsgSender());
      dodo.init(owner, supervisor, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
   }

   void newToken(const extended_asset& tokenx) {
      const extended_symbol& exsym      = tokenx.get_extended_symbol();
      const symbol&          sym        = exsym.get_symbol();
      TokenStore&            tokenStore = _storage_mgmt.newTokenStore(exsym);
      TestERC20              otoken(tokenStore);
      otoken.setMsgSender(getMsgSender());
      otoken.init(sym.code().to_string(), sym.precision(), exsym);
   }

   extended_symbol newLpToken(const extended_symbol& tokenx) override {
      extended_symbol exsym         = extended_symbol(tokenx.get_symbol(), LP_TOKEN_CONTRACT);
      TokenStore&     tokenStore    = _storage_mgmt.newTokenStore(exsym);
      TokenStore&     olptokenStore = _storage_mgmt.get_token_store(exsym);
      DODOLpToken     token(tokenStore, olptokenStore);
      token.setMsgSender(getMsgSender());
      token.init(tokenx);
      return exsym;
   }

   void newEthToken(const extended_asset& tokenx) {
      const extended_symbol& exsym      = tokenx.get_extended_symbol();
      TokenStore&            tokenStore = _storage_mgmt.newTokenStore(exsym);
      WETH9                  otoken(tokenStore);
      otoken.setMsgSender(getMsgSender());
      otoken.init(getMsgSender(), exsym);
   }
};