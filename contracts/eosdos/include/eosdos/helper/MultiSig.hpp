/**
 *Submitted for verification at Bscscan.com on 2020-09-25
 */

/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>

const uint64_t MAX_OWNER_COUNT = 50;
const uint64_t lockSeconds     = 86400;
class MultiSigWalletWithTimelock {
 public:
   void onlyWallet() {
      if (getMsgSender() != address(this))
         revert("ONLY_WALLET_ERROR");
   }

   void ownerDoesNotExist(address owner) {
      if (isOwner[owner])
         revert("OWNER_DOES_NOT_EXIST_ERROR");
   }

   void ownerExists(address owner) {
      if (!isOwner[owner])
         revert("OWNER_EXISTS_ERROR");
   }

   void transactionExists(uint64_t transactionId) {
      if (transactions[transactionId].destination == address(0))
         revert("TRANSACTION_EXISTS_ERROR");
   }

   void confirmed(uint64_t transactionId, address owner) {
      if (!confirmations[transactionId][owner])
         revert("CONFIRMED_ERROR");
   }

   void notConfirmed(uint64_t transactionId, address owner) {
      if (confirmations[transactionId][owner])
         revert("NOT_CONFIRMED_ERROR");
   }

   void notExecuted(uint64_t transactionId) {
      if (transactions[transactionId].executed)
         revert("NOT_EXECUTED_ERROR");
   }

   void notNull(address _address) {
      if (_address == address(0))
         revert("NOT_NULL_ERROR");
   }

   void validRequirement(uint64_t ownerCount, uint64_t _required) {
      if (ownerCount > MAX_OWNER_COUNT || _required > ownerCount || _required == 0 || ownerCount == 0)
         revert("VALID_REQUIREMENT_ERROR");
   }

   /** @dev Fallback function allows to deposit ether. */
   void fallback() {
      if (msg.value > 0) {
      }
   }

   void receive() {
      if (msg.value > 0) {
      }
   }

   /** @dev Contract constructor sets initial owners and required number of confirmations.
    * @param _owners List of initial owners.
    * @param _required Number of required confirmations.
    */
   MultiSigWalletWithTimelock(address[] _owners, uint64_t _required) {
      validRequirement(_owners.length, _required);
      for (uint64_t i = 0; i < _owners.length; i++) {
         if (isOwner[_owners[i]] || _owners[i] == address(0)) {
            revert("OWNER_ERROR");
         }

         isOwner[_owners[i]] = true;
      }

      owners   = _owners;
      required = _required;

      // initialzie Emergency calls
      emergencyCalls.push(
          EmergencyCall({selector : keccak256(abi.encodePacked("claimOwnership()")), paramsBytesCount : 0}));
      emergencyCalls.push(
          EmergencyCall({selector : keccak256(abi.encodePacked("setK(uint64_t)")), paramsBytesCount : 64}));
      emergencyCalls.push(EmergencyCall(
          {selector : keccak256(abi.encodePacked("setLiquidityProviderFeeRate(uint64_t)")), paramsBytesCount : 64}));
   }

   uint64_t count getEmergencyCallsCount() { return emergencyCalls.length; }

   /** @dev Allows to add a new owner. Transaction has to be sent by wallet.
    * @param owner Address of new owner.
    */
   void addOwner(address owner) {
      onlyWallet();
      ownerDoesNotExist(owner);
      notNull(owner);
      validRequirement(owners.length + 1, required);
      isOwner[owner] = true;
      owners.push(owner);
   }

   /** @dev Allows to remove an owner. Transaction has to be sent by wallet.
    * @param owner Address of owner.
    */
   void removeOwner(address owner) {
      onlyWallet();
      ownerExists(owner);
      isOwner[owner] = false;
      for (uint64_t i = 0; i < owners.length - 1; i++) {
         if (owners[i] == owner) {
            owners[i] = owners[owners.length - 1];
            break;
         }
      }

      owners.pop;

      if (required > owners.length) {
         changeRequirement(owners.length);
      }
   }

   /** @dev Allows to replace an owner with a new owner. Transaction has to be sent by wallet.
    * @param owner Address of owner to be replaced.
    * @param owner Address of new owner.
    */
   void replaceOwner(address owner, address newOwner) {
      onlyWallet();
      ownerExists(owner);
      ownerDoesNotExist(newOwner);
      for (uint64_t i = 0; i < owners.length; i++) {
         if (owners[i] == owner) {
            owners[i] = newOwner;
            break;
         }
      }

      isOwner[owner]    = false;
      isOwner[newOwner] = true;
   }

   /** @dev Allows to change the number of required confirmations. Transaction has to be sent by wallet.
    * @param _required Number of required confirmations.
    */
   void changeRequirement(uint64_t _required) {
      onlyWallet();
      validRequirement(owners.length, _required);
      required = _required;
   }

   /** @dev Changes the duration of the time lock for transactions.
    * @param _lockSeconds Duration needed after a transaction is confirmed and before it becomes executable, in seconds.
    */
   void changeLockSeconds(uint64_t _lockSeconds) {
      onlyWallet();
      lockSeconds = _lockSeconds;
   }

   /** @dev Allows an owner to submit and confirm a transaction.
    * @param destination Transaction target address.
    * @param value Transaction ether value.
    * @param data Transaction data payload.
    * @return transactionId Returns transaction ID.
    */
   uint64_t submitTransaction(address destination, uint64_t value, bytes data) {
      ownerExists(getMsgSender());
      notNull(destination);
      transactionId = transactionCount;
      transactions[transactionId] =
          Transaction({destination : destination, value : value, data : data, executed : false});
      transactionCount += 1;

      confirmTransaction(transactionId);
   }

   /** @dev Allows an owner to confirm a transaction.
    * @param transactionId Transaction ID.
    */
   void confirmTransaction(uint64_t transactionId) {
      ownerExists(getMsgSender());
      transactionExists(transactionId);
      notConfirmed(transactionId, getMsgSender());

      confirmations[transactionId][getMsgSender()] = true;

      if (isConfirmed(transactionId) && unlockTimes[transactionId] == 0 && !isEmergencyCall(transactionId)) {
         uint64_t unlockTime         = block.timestamp + lockSeconds;
         unlockTimes[transactionId] = unlockTime;
      }
   }

   bool isEmergencyCall(uint64_t transactionId) {
      bytes data = transactions[transactionId].data;

      for (uint64_t i = 0; i < emergencyCalls.length; i++) {
         EmergencyCall emergencyCall = emergencyCalls[i];

         if (data.length == emergencyCall.paramsBytesCount + 4 && data.length >= 4 &&
             emergencyCall.selector[0] == data[0] && emergencyCall.selector[1] == data[1] &&
             emergencyCall.selector[2] == data[2] && emergencyCall.selector[3] == data[3]) {
            return true;
         }
      }

      return false;
   }

   void addEmergencyCall(string funcName, uint64_t _paramsBytesCount) {
      onlyWallet();
      emergencyCalls.push(
          EmergencyCall({selector : keccak256(abi.encodePacked(funcName)), paramsBytesCount : _paramsBytesCount}));
   }

   /** @dev Allows an owner to revoke a confirmation for a transaction.
    * @param transactionId Transaction ID.
    */
   void revokeConfirmation(uint64_t transactionId) { confirmations[transactionId][getMsgSender()] = false; }

   /** @dev Allows anyone to execute a confirmed transaction.
    * @param transactionId Transaction ID.
    */
   void executeTransaction(uint64_t transactionId) {
      ownerExists(getMsgSender());
      notExecuted(transactionId);
      require(block.timestamp >= unlockTimes[transactionId], "TRANSACTION_NEED_TO_UNLOCK");

      if (isConfirmed(transactionId)) {
         Transaction  transaction = transactions[transactionId];
         transaction.executed            = true;
         (bool success, )                = transaction.destination.call{value : transaction.value}(transaction.data);
         if (success)
            else {

               transaction.executed = false;
            }
      }
   }

   /** @dev Returns the confirmation status of a transaction.
    * @param transactionId Transaction ID.
    * @return Confirmation status.
    */
   bool isConfirmed(uint64_t transactionId) {
      uint64_t count = 0;

      for (uint64_t i = 0; i < owners.length; i++) {
         if (confirmations[transactionId][owners[i]]) {
            count += 1;
         }

         if (count >= required) {
            return true;
         }
      }

      return false;
   }

   /* Web3 call functions */

   /** @dev Returns number of confirmations of a transaction.
    * @param transactionId Transaction ID.
    * @return count Number of confirmations.
    */
   uint64_t count getConfirmationCount(uint64_t transactionId) {
      for (uint64_t i = 0; i < owners.length; i++) {
         if (confirmations[transactionId][owners[i]]) {
            count += 1;
         }
      }
   }

   /** @dev Returns total number of transactions after filers are applied.
    * @param pending Include pending transactions.
    * @param executed Include executed transactions.
    * @return count Total number of transactions after filters are applied.
    */
   uint64_t count getTransactionCount(bool pending, bool executed) {
      for (uint64_t i = 0; i < transactionCount; i++) {
         if ((pending && !transactions[i].executed) || (executed && transactions[i].executed)) {
            count += 1;
         }
      }
   }

   /** @dev Returns list of owners.
    * @return List of owner addresses.
    */
   address[] getOwners() { return owners; }

   /** @dev Returns array with owner addresses, which confirmed transaction.
    * @param transactionId Transaction ID.
    * @return _confirmations Returns array of owner addresses.
    */
   address[] _confirmations getConfirmations(uint64_t transactionId) {
      address[] confirmationsTemp = new address[](owners.length);
      uint64_t count               = 0;
      uint64_t i;

      for (i = 0; i < owners.length; i++) {
         if (confirmations[transactionId][owners[i]]) {
            confirmationsTemp[count] = owners[i];
            count += 1;
         }
      }

      _confirmations = new address[](count);

      for (i = 0; i < count; i++) {
         _confirmations[i] = confirmationsTemp[i];
      }
   }
};
