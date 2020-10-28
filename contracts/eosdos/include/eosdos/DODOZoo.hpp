/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
#include <eosdos/dodo.hpp>
#include <eosdos/helper/CloneFactory.hpp>
#include <eosdos/helper/MinimumOracle.hpp>
#include <eosdos/helper/TestERC20.hpp>
#include <eosdos/helper/TestWETH.hpp>
#include <eosdos/impl/DODOLpToken.hpp>
#include <eosdos/lib/Ownable.hpp>

/**
 * @title DODOZoo
 * @author DODO Breeder
 *
 * @notice Register of All DODO
 */
class DODOZoo : public Ownable, public IStorage {
 private:
   name          self;
   name          msg_sender;
   storage_mgmt  _storage_mgmt;
   transfer_mgmt _transfer_mgmt;
   ZooStorage&   zoo_storage;

 public:
   DODOZoo(name _self)
       : self(_self)
       , _storage_mgmt(_self)
       , _transfer_mgmt(_self)
       , zoo_storage(get_storage_mgmt().get_zoo_store())
       , Ownable(get_storage_mgmt().get_zoo_store().ownable)
       , IStorage(*this) {}
   name           get_self() override { return self; }
   storage_mgmt&  get_storage_mgmt() override { return _storage_mgmt; }
   transfer_mgmt& get_transfer_mgmt() override { return _transfer_mgmt; }

   void init(address _dodoLogic, address _cloneFactory, address _defaultSupervisor) {
      zoo_storage.ownable._OWNER_      = getMsgSender();
      zoo_storage._DODO_LOGIC_         = _dodoLogic;
      zoo_storage._CLONE_FACTORY_      = _cloneFactory;
      zoo_storage._DEFAULT_SUPERVISOR_ = _defaultSupervisor;
   }

   // ============ Admin Function ============

   //    void setDODOLogic(address _dodoLogic) {
   //       onlyOwner();
   //       _DODO_LOGIC_ = _dodoLogic;
   //    }

   //    void setCloneFactory(address _cloneFactory) {
   //       onlyOwner();
   //       _CLONE_FACTORY_ = _cloneFactory;
   //    }

   //    void setDefaultSupervisor(address _defaultSupervisor) {
   //       onlyOwner();
   //       _DEFAULT_SUPERVISOR_ = _defaultSupervisor;
   //    }

   void removeDODO(address _dodo) {
      onlyOwner();
      get_dodo(_dodo, [&](auto& dodo) {
         namesym baseToken  = dodo._BASE_TOKEN_();
         namesym quoteToken = dodo._QUOTE_TOKEN_();

         require(isDODORegistered(baseToken, quoteToken), "DODO_NOT_REGISTERED");
         zoo_storage._DODO_REGISTER_[baseToken].q2d[quoteToken] = address(0);
      });

      for (uint256 i = 0; i <= zoo_storage._DODOs.size() - 1; i++) {
         if (zoo_storage._DODOs[i] == _dodo) {
            zoo_storage._DODOs[i] = zoo_storage._DODOs[zoo_storage._DODOs.size() - 1];
            zoo_storage._DODOs.pop_back();
            break;
         }
      }
   }

   void addDODO(address _dodo) {
      get_dodo(_dodo, [&](auto& dodo) {
         namesym baseToken  = dodo._BASE_TOKEN_();
         namesym quoteToken = dodo._QUOTE_TOKEN_();

         require(!isDODORegistered(baseToken, quoteToken), "DODO_REGISTERED");
         zoo_storage._DODO_REGISTER_[baseToken].q2d[quoteToken] = _dodo;
      });
      zoo_storage._DODOs.push_back(_dodo);
   }

   // ============ Breed DODO Function ============
   address breedDODO(
       name dodo_name, address maintainer, const extended_symbol& baseToken, const extended_symbol& quoteToken,
       const extended_symbol& oracle, uint256 lpFeeRate, uint256 mtFeeRate, uint256 k, uint256 gasPriceLimit) {

      namesym nbaseToken  = to_namesym(baseToken);
      namesym nquoteToken = to_namesym(quoteToken);

      require(!isDODORegistered(nbaseToken, nquoteToken), "DODO_REGISTERED");

      newDODO(
          dodo_name, zoo_storage.ownable._OWNER_, zoo_storage._DEFAULT_SUPERVISOR_, maintainer, baseToken, quoteToken,
          oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);

      addDODO(dodo_name);

      return dodo_name;
   }

   // ============ View Functions ============

   bool isDODORegistered(namesym baseToken, namesym quoteToken) {
      if (zoo_storage._DODO_REGISTER_[baseToken].q2d[quoteToken] == address(0) &&
          zoo_storage._DODO_REGISTER_[quoteToken].q2d[baseToken] == address(0)) {
         return false;
      } else {
         return true;
      }
   }

   address getDODO(namesym baseToken, namesym quoteToken) {
      return zoo_storage._DODO_REGISTER_[baseToken].q2d[quoteToken];
   }

   std::vector<address>& getDODOs() { return zoo_storage._DODOs; }

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
