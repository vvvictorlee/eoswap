/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eodos/intf/IERC20.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/Ownable.hpp>
#include <eodos/lib/SafeERC20.hpp>
#include <eodos/lib/SafeMath.hpp>

/**
 * @title LockedTokenVault
 * @author DODO Breeder
 *
 * @notice Lock Token and release it linearly
 */
#ifdef LockedTokenVault
class LockedTokenVault : public Ownable {
 public:
   // ============ Modifiers ============

   void beforeStartRelease() { require(block.timestamp < _START_RELEASE_TIME_, "RELEASE START"); }

   void afterStartRelease() { require(block.timestamp >= _START_RELEASE_TIME_, "RELEASE NOT START"); }

   void distributeNotFinished() { require(!_DISTRIBUTE_FINISHED_, "DISTRIBUTE FINISHED"); }

   // ============ Init Functions ============

   LockedTokenVault(address _token, uint256 _startReleaseTime, uint256 _releaseDuration, uint256 _cliffRate) {
      _TOKEN_              = _token;
      _START_RELEASE_TIME_ = _startReleaseTime;
      _RELEASE_DURATION_   = _releaseDuration;
      _CLIFF_RATE_         = _cliffRate;
   }

   void deposit(uint256 amount) {
      onlyOwner();
      _tokenTransferIn(_OWNER_, amount);
      _UNDISTRIBUTED_AMOUNT_ = _UNDISTRIBUTED_AMOUNT_.add(amount);
   }

   void withdraw(uint256 amount) {
      onlyOwner();
      _UNDISTRIBUTED_AMOUNT_ = _UNDISTRIBUTED_AMOUNT_.sub(amount);
      _tokenTransferOut(_OWNER_, amount);
   }

   void finishDistribute() {
      onlyOwner();
      _DISTRIBUTE_FINISHED_ = true;
   }

   // ============ For Owner ============

   void grant(address[] holderList, uint256[] amountList) {
      onlyOwner();
      require(holderList.length == amountList.length, "batch grant length not match");
      uint256 amount = 0;
      for (uint256 i = 0; i < holderList.length; ++i) {
         // for saving gas, no
         amount = amount.add(amountList[i]);
      }
      _UNDISTRIBUTED_AMOUNT_ = _UNDISTRIBUTED_AMOUNT_.sub(amount);
   }

   void recall(address holder) {
      onlyOwner();
      distributeNotFinished();
      _UNDISTRIBUTED_AMOUNT_  = _UNDISTRIBUTED_AMOUNT_.add(originBalances[holder]).sub(claimedBalances[holder]);
      originBalances[holder]  = 0;
      claimedBalances[holder] = 0;
   }

   // ============ For Holder ============

   void transferLockedToken(address to) {
      originBalances[to]  = originBalances[to].add(originBalances[getMsgSender()]);
      claimedBalances[to] = claimedBalances[to].add(claimedBalances[getMsgSender()]);

      originBalances[getMsgSender()]  = 0;
      claimedBalances[getMsgSender()] = 0;
   }

   void claim() {
      uint256 claimableToken = getClaimableBalance(getMsgSender());
      _tokenTransferOut(getMsgSender(), claimableToken);
      claimedBalances[getMsgSender()] = claimedBalances[getMsgSender()].add(claimableToken);
      emit Claim(getMsgSender(), originBalances[getMsgSender()], claimedBalances[getMsgSender()], claimableToken);
   }

   // ============ View ============

   bool isReleaseStart() { return block.timestamp >= _START_RELEASE_TIME_; }

   uint256 getOriginBalance(address holder) { return originBalances[holder]; }

   uint256 getClaimedBalance(address holder) { return claimedBalances[holder]; }

   uint256 getClaimableBalance(address holder) {
      uint256 remainingToken = getRemainingBalance(holder);
      return originBalances[holder].sub(remainingToken).sub(claimedBalances[holder]);
   }

   uint256 getRemainingBalance(address holder) {
      uint256 remainingRatio = getRemainingRatio(block.timestamp);
      return DecimalMath.mul(originBalances[holder], remainingRatio);
   }

   uint256 getRemainingRatio(uint256 timestamp) {
      if (timestamp < _START_RELEASE_TIME_) {
         return DecimalMath.ONE;
      }
      uint256 timePast = timestamp.sub(_START_RELEASE_TIME_);
      if (timePast < _RELEASE_DURATION_) {
         uint256 remainingTime = _RELEASE_DURATION_.sub(timePast);
         return DecimalMath.ONE.sub(_CLIFF_RATE_).mul(remainingTime).div(_RELEASE_DURATION_);
      } else {
         return 0;
      }
   }

   // ============ Internal Helper ============

   void _tokenTransferIn(address from, uint256 amount) {
      IERC20(_TOKEN_).safeTransferFrom(from, address(this), amount);
   }

   void _tokenTransferOut(address to, uint256 amount) { IERC20(_TOKEN_).safeTransfer(to, amount); }
};
#endif