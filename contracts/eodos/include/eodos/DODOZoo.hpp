/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

#include <eodos/helper/CloneFactory.hpp>
#include <eodos/intf/IDODO.hpp>
#include <eodos/lib/Ownable.hpp>

/**
 * @title DODOZoo
 * @author DODO Breeder
 *
 * @notice Register of All DODO
 */
class DODOZoo : public Ownable {
  
   // ============ Events ============

   constructor(address _dodoLogic, address _cloneFactory, address _defaultSupervisor) public {
      _DODO_LOGIC_         = _dodoLogic;
      _CLONE_FACTORY_      = _cloneFactory;
      _DEFAULT_SUPERVISOR_ = _defaultSupervisor;
   }

   // ============ Admin Function ============

   void setDODOLogic(address _dodoLogic) {
      onlyOwner();
      _DODO_LOGIC_ = _dodoLogic;
   }

   void setCloneFactory(address _cloneFactory) {
      onlyOwner();
      _CLONE_FACTORY_ = _cloneFactory;
   }

   void setDefaultSupervisor(address _defaultSupervisor) {
      onlyOwner();
      _DEFAULT_SUPERVISOR_ = _defaultSupervisor;
   }

   void removeDODO(address dodo) {
      onlyOwner();
      address baseToken = IDODO(dodo)._BASE_TOKEN_();

      address quoteToken = IDODO(dodo)._QUOTE_TOKEN_();

      require(isDODORegistered(baseToken, quoteToken), "DODO_NOT_REGISTERED");

      _DODO_REGISTER_[baseToken][quoteToken] = address(0);

      for (uint256 i = 0; i <= _DODOs.length - 1; i++) {
         if (_DODOs[i] == dodo) {
            _DODOs[i] = _DODOs[_DODOs.length - 1];
            _DODOs.pop();
            break;
         }
      }
   }

   void addDODO(address dodo) {
      address baseToken  = IDODO(dodo)._BASE_TOKEN_();
      address quoteToken = IDODO(dodo)._QUOTE_TOKEN_();
      require(!isDODORegistered(baseToken, quoteToken), "DODO_REGISTERED");
      _DODO_REGISTER_[baseToken][quoteToken] = dodo;
      _DODOs.push(dodo);
   }

   // ============ Breed DODO Function ============

   address breedDODO(
       address maintainer, address baseToken, address quoteToken, address oracle, uint256 lpFeeRate, uint256 mtFeeRate,
       uint256 k, uint256 gasPriceLimit) {
      require(!isDODORegistered(baseToken, quoteToken), "DODO_REGISTERED");
      newBornDODO = ICloneFactory(_CLONE_FACTORY_).clone(_DODO_LOGIC_);
      IDODO(newBornDODO)
          .init(
              _OWNER_, _DEFAULT_SUPERVISOR_, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k,
              gasPriceLimit);
      addDODO(newBornDODO);

      return newBornDODO;
   }

   // ============ View Functions ============

   bool isDODORegistered(address baseToken, address quoteToken) {
      if (_DODO_REGISTER_[baseToken][quoteToken] == address(0) &&
          _DODO_REGISTER_[quoteToken][baseToken] == address(0)) {
         return false;
      } else {
         return true;
      }
   }

   address getDODO(address baseToken, address quoteToken) { return _DODO_REGISTER_[baseToken][quoteToken]; }

   address[] getDODOs() { return _DODOs; }
}
