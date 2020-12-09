/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>
#include <common/instance_mgmt.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
#include <eosdos/dodo.hpp>
#include <eosdos/helper/TestERC20.hpp>
#include <eosdos/helper/TestWETH.hpp>
#include <eosdos/impl/DODOLpToken.hpp>

/**
 * @title DODOZoo
 * @author DODO Breeder
 *
 * @notice Register of All DODO
 */
class DODOZoo {
 private:
   name           self;
   instance_mgmt& _instance_mgmt;
   ZooStorage&    zoo_storage;
   name           msg_sender;

 public:
   DODOZoo(name _self, instance_mgmt& __instance_mgmt)
       : self(_self)
       , _instance_mgmt(__instance_mgmt)
       , zoo_storage(__instance_mgmt.get_storage_mgmt().get_zoo_store()) {}
   name getMsgSender() { return msg_sender; }
   void setMsgSender(name _msg_sender, bool flag = false) {
      if (flag) {
         require_auth(_msg_sender);
      }
      msg_sender = _msg_sender;
   }
   void init(address _dodoLogic, address _cloneFactory, address _defaultSupervisor) {
      zoo_storage.ownable._OWNER_      = self;
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
    //   onlyOwner();
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
         namesym baseToken  = dodo._BASE_TOKEN_();
         namesym quoteToken = dodo._QUOTE_TOKEN_();

         require(isDODORegistered(baseToken, quoteToken), "DODO_NOT_REGISTERED");
         zoo_storage._DODO_REGISTER_[baseToken].q2d[quoteToken] = address(0);
      });

      for (uint64_t i = 0; i <= zoo_storage._DODOs.size() - 1; i++) {
         if (zoo_storage._DODOs[i] == _dodo) {
            zoo_storage._DODOs[i] = zoo_storage._DODOs[zoo_storage._DODOs.size() - 1];
            zoo_storage._DODOs.pop_back();
            break;
         }
      }
   }

   void addDODO(address _dodo) {
      _instance_mgmt.get_dodo(self, _dodo, [&](auto& dodo) {
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
       const extended_symbol& oracle, uint64_t lpFeeRate, uint64_t mtFeeRate, uint64_t k, uint64_t gasPriceLimit) {

      namesym nbaseToken  = to_namesym(baseToken);
      namesym nquoteToken = to_namesym(quoteToken);

      require(!isDODORegistered(nbaseToken, nquoteToken), "DODO_REGISTERED");

      _instance_mgmt.newDODO(
          getMsgSender(), dodo_name, zoo_storage.ownable._OWNER_, zoo_storage._DEFAULT_SUPERVISOR_, maintainer,
          baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);

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
};
