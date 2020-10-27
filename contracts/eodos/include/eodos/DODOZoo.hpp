/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
#include <eodos/dodo.hpp>
#include <eodos/helper/CloneFactory.hpp>
#include <eodos/helper/MinimumOracle.hpp>
#include <eodos/impl/DODOLpToken.hpp>
#include <eodos/lib/Ownable.hpp>

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
       , Ownable(get_storage_mgmt().get_zoo_store().ownable),IStorage(*this) {}
   name           get_self() override { return self; }
   storage_mgmt&  get_storage_mgmt() override { return _storage_mgmt; }
   transfer_mgmt& get_transfer_mgmt() override { return _transfer_mgmt; }

   //    DODOZoo(address _dodoLogic, address _cloneFactory, address _defaultSupervisor)  {
   //       _DODO_LOGIC_         = _dodoLogic;
   //       _CLONE_FACTORY_      = _cloneFactory;
   //       _DEFAULT_SUPERVISOR_ = _defaultSupervisor;
   //    }

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
       address maintainer, const extended_symbol& baseToken, const extended_symbol& quoteToken,
       const extended_symbol& oracle, uint256 lpFeeRate, uint256 mtFeeRate, uint256 k, uint256 gasPriceLimit) {

      namesym nbaseToken  = to_namesym(baseToken);
      namesym nquoteToken = to_namesym(quoteToken);

      require(!isDODORegistered(nbaseToken, nquoteToken), "DODO_REGISTERED");

      auto _dodo = _storage_mgmt.newDodoStore(zoo_storage._DODO_LOGIC_);

      get_dodo(_dodo, [&](auto& dodo) {
         dodo.init(
             zoo_storage.ownable._OWNER_, zoo_storage._DEFAULT_SUPERVISOR_, maintainer, baseToken, quoteToken, oracle,
             lpFeeRate, mtFeeRate, k, gasPriceLimit);
      });
      //   newBornDODO = ICloneFactory(_CLONE_FACTORY_).clone(_DODO_LOGIC_);

      //   IDODO(newBornDODO)
      //       .init(
      //           zoo_storage._OWNER_, zoo_storage._DEFAULT_SUPERVISOR_, maintainer, baseToken, quoteToken, oracle,
      //           lpFeeRate, mtFeeRate, k, gasPriceLimit);

      addDODO(_dodo);

      return _dodo;
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
      func(dodo);
   }

   template <typename T>
   void get_lptoken(const extended_symbol& lptoken, T func) {
      DODOTokenStore& lptokenStore  = _storage_mgmt.get_lptoken_store(lptoken);
      DODOTokenStore& olptokenStore = _storage_mgmt.get_token_store(lptokenStore.originToken);
      DODOLpToken     token(lptokenStore, olptokenStore);
      func(token);
   }

   template <typename T>
   void get_oracle(const extended_symbol& oracle, T func) {
      OracleStore&  oracleStore = _storage_mgmt.get_oracle_store(oracle);
      MinimumOracle minioracle(oracleStore);
      func(minioracle);
   }
};

template <typename T>
void IStorage::get_dodo(name dodo_name, T func) {
   subStorage.get_dodo(dodo_name, func);
}

template <typename T>
void IStorage::get_lptoken(const extended_symbol& lptoken, T func) {
   subStorage.get_lptoken(lptoken, func);
}

template <typename T>
void IStorage::get_oracle(const extended_symbol& oracle, T func) {
   subStorage.get_oracle(oracle, func);
}