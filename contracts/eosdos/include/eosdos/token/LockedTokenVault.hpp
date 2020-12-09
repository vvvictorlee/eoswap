/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

#include <eosdos/intf/IERC20.hpp>
#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/SafeERC20.hpp>
#include <eosdos/lib/SafeMath.hpp>

/**
 * @title LockedTokenVault
 * @author DODO Breeder
 *
 * @notice Lock Token and release it linearly
 */
#ifdef LockedTokenVault
class LockedTokenVault  {
 public:
   // ============ Modifiers ============

   void beforeStartRelease() { require(block.timestamp < _START_RELEASE_TIME_, "RELEASE START"); }

   void afterStartRelease() { require(block.timestamp >= _START_RELEASE_TIME_, "RELEASE NOT START"); }

   void distributeNotFinished() { require(!_DISTRIBUTE_FINISHED_, "DISTRIBUTE FINISHED"); }

   // ============ Init Functions ============

   LockedTokenVault(address _token, uint64_t _startReleaseTime, uint64_t _releaseDuration, uint64_t _cliffRate) {
      _TOKEN_              = _token;
      _START_RELEASE_TIME_ = _startReleaseTime;
      _RELEASE_DURATION_   = _releaseDuration;
      _CLIFF_RATE_         = _cliffRate;
   }

   void deposit(uint64_t amount) {
      onlyOwner();
      _tokenTransferIn(_OWNER_, amount);
      _UNDISTRIBUTED_AMOUNT_ = _UNDISTRIBUTED_AMOUNT_.add(amount);
   }

   void withdraw(uint64_t amount) {
      onlyOwner();
      _UNDISTRIBUTED_AMOUNT_ = _UNDISTRIBUTED_AMOUNT_.sub(amount);
      _tokenTransferOut(_OWNER_, amount);
   }

   void finishDistribute() {
      onlyOwner();
      _DISTRIBUTE_FINISHED_ = true;
   }

   // ============ For Owner ============

   void grant(address[] holderList, uint64_t[] amountList) {
      onlyOwner();
      require(holderList.length == amountList.length, "batch grant length not match");
      uint64_t amount = 0;
      for (uint64_t i = 0; i < holderList.length; ++i) {
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
      uint64_t claimableToken = getClaimableBalance(getMsgSender());
      _tokenTransferOut(getMsgSender(), claimableToken);
      claimedBalances[getMsgSender()] = claimedBalances[getMsgSender()].add(claimableToken);
      emit Claim(getMsgSender(), originBalances[getMsgSender()], claimedBalances[getMsgSender()], claimableToken);
   }

   // ============ View ============

   bool isReleaseStart() { return block.timestamp >= _START_RELEASE_TIME_; }

   uint64_t getOriginBalance(address holder) { return originBalances[holder]; }

   uint64_t getClaimedBalance(address holder) { return claimedBalances[holder]; }

   uint64_t getClaimableBalance(address holder) {
      uint64_t remainingToken = getRemainingBalance(holder);
      return originBalances[holder].sub(remainingToken).sub(claimedBalances[holder]);
   }

   uint64_t getRemainingBalance(address holder) {
      uint64_t remainingRatio = getRemainingRatio(block.timestamp);
      return DecimalMath.mul(originBalances[holder], remainingRatio);
   }

   uint64_t getRemainingRatio(uint64_t timestamp) {
      if (timestamp < _START_RELEASE_TIME_) {
         return DecimalMath.ONE;
      }
      uint64_t timePast = timestamp.sub(_START_RELEASE_TIME_);
      if (timePast < _RELEASE_DURATION_) {
         uint64_t remainingTime = _RELEASE_DURATION_.sub(timePast);
         return DecimalMath.ONE.sub(_CLIFF_RATE_).mul(remainingTime).div(_RELEASE_DURATION_);
      } else {
         return 0;
      }
   }

   // ============ Internal Helper ============

   void _tokenTransferIn(address from, uint64_t amount) {
      IERC20(_TOKEN_).safeTransferFrom(from, address(this), amount);
   }

   void _tokenTransferOut(address to, uint64_t amount) { IERC20(_TOKEN_).safeTransfer(to, amount); }
};
#endif