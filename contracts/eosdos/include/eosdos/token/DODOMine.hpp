/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>

#include <eosdos/DODORewardVault.hpp>
#include <eosdos/intf/IERC20.hpp>
#include <eosdos/lib/DecimalMath.hpp>
#include <eosdos/lib/Ownable.hpp>
#include <eosdos/lib/SafeERC20.hpp>
#include <eosdos/lib/SafeMath.hpp>
#ifdef DODOMINE
class DODOMine : public Ownable {
   DODOMine(address _dodoToken, uint64_t _startBlock) public {
      dodoRewardVault = address(new DODORewardVault(_dodoToken));
      startBlock      = _startBlock;
   }

   // ============ Modifiers ============

   void lpTokenExist(address lpToken) { require(lpTokenRegistry[lpToken] > 0, "LP Token Not Exist"); }

   void lpTokenNotExist(address lpToken) { require(lpTokenRegistry[lpToken] == 0, "LP Token Already Exist"); }

   // ============ Helper ============

   uint64_t poolLength() { return poolInfos.length; }

   uint64_t getPid(address _lpToken) {
      lpTokenExist();
      return lpTokenRegistry[_lpToken] - 1;
   }

   uint64_t getUserLpBalance(address _lpToken, address _user) {
      uint64_t pid = getPid(_lpToken);
      return userInfo[pid][_user].amount;
   }

   // ============ Ownable ============

   void addLpToken(address _lpToken, uint64_t _allocPoint, bool _withUpdate) {
      lpTokenNotExist(_lpToken);
      onlyOwner();
      if (_withUpdate) {
         massUpdatePools();
      }
      uint64_t lastRewardBlock = block.number > startBlock ? block.number : startBlock;
      totalAllocPoint         = totalAllocPoint.add(_allocPoint);
      poolInfos.push(PoolInfo(
          {lpToken : _lpToken, allocPoint : _allocPoint, lastRewardBlock : lastRewardBlock, accDODOPerShare : 0}));
      lpTokenRegistry[_lpToken] = poolInfos.length;
   }

   void setLpToken(address _lpToken, uint64_t _allocPoint, bool _withUpdate) {
      onlyOwner();
      if (_withUpdate) {
         massUpdatePools();
      }
      uint64_t pid               = getPid(_lpToken);
      totalAllocPoint           = totalAllocPoint.sub(poolInfos[pid].allocPoint).add(_allocPoint);
      poolInfos[pid].allocPoint = _allocPoint;
   }

   void setReward(uint64_t _dodoPerBlock, bool _withUpdate) {
      onlyOwner();
      if (_withUpdate) {
         massUpdatePools();
      }
      dodoPerBlock = _dodoPerBlock;
   }

   // ============ View Rewards ============

   uint64_t getPendingReward(address _lpToken, address _user) {
      uint64_t  pid                     = getPid(_lpToken);
      PoolInfo  pool            = poolInfos[pid];
      UserInfo  user            = userInfo[pid][_user];
      uint64_t          accDODOPerShare = pool.accDODOPerShare;
      uint64_t          lpSupply        = IERC20(pool.lpToken).balanceOf(address(this));
      if (block.number > pool.lastRewardBlock && lpSupply != 0) {
         uint64_t DODOReward =
             block.number.sub(pool.lastRewardBlock).mul(dodoPerBlock).mul(pool.allocPoint).div(totalAllocPoint);
         accDODOPerShare = accDODOPerShare.add(DecimalMath.divFloor(DODOReward, lpSupply));
      }
      return DecimalMath.mul(user.amount, accDODOPerShare).sub(user.rewardDebt);
   }

   uint64_t getAllPendingReward(address _user) {
      uint64_t length      = poolInfos.length;
      uint64_t totalReward = 0;
      for (uint64_t pid = 0; pid < length; ++pid) {
         if (userInfo[pid][_user].amount == 0 || poolInfos[pid].allocPoint == 0) {
            continue; // save gas
         }
         PoolInfo  pool            = poolInfos[pid];
         UserInfo  user            = userInfo[pid][_user];
         uint64_t          accDODOPerShare = pool.accDODOPerShare;
         uint64_t          lpSupply        = IERC20(pool.lpToken).balanceOf(address(this));
         if (block.number > pool.lastRewardBlock && lpSupply != 0) {
            uint64_t DODOReward =
                block.number.sub(pool.lastRewardBlock).mul(dodoPerBlock).mul(pool.allocPoint).div(totalAllocPoint);
            accDODOPerShare = accDODOPerShare.add(DecimalMath.divFloor(DODOReward, lpSupply));
         }
         totalReward = totalReward.add(DecimalMath.mul(user.amount, accDODOPerShare).sub(user.rewardDebt));
      }
      return totalReward;
   }

   uint64_t getRealizedReward(address _user) { return realizedReward[_user]; }

   uint64_t getDlpMiningSpeed(address _lpToken) {
      uint64_t  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      return dodoPerBlock.mul(pool.allocPoint).div(totalAllocPoint);
   }

   // ============ Update Pools ============

   // Update reward vairables for all pools. Be careful of gas spending!
   void massUpdatePools() {
      uint64_t length = poolInfos.length;
      for (uint64_t pid = 0; pid < length; ++pid) {
         updatePool(pid);
      }
   }

   // Update reward variables of the given pool to be up-to-date.
   void updatePool(uint64_t _pid) {
      PoolInfo  pool = poolInfos[_pid];
      if (block.number <= pool.lastRewardBlock) {
         return;
      }
      uint64_t lpSupply = IERC20(pool.lpToken).balanceOf(address(this));
      if (lpSupply == 0) {
         pool.lastRewardBlock = block.number;
         return;
      }
      uint64_t DODOReward =
          block.number.sub(pool.lastRewardBlock).mul(dodoPerBlock).mul(pool.allocPoint).div(totalAllocPoint);
      pool.accDODOPerShare = pool.accDODOPerShare.add(DecimalMath.divFloor(DODOReward, lpSupply));
      pool.lastRewardBlock = block.number;
   }

   // ============ Deposit & Withdraw & Claim ============
   // Deposit & withdraw will also trigger claim

   void deposit(address _lpToken, uint64_t _amount) {
      uint64_t  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      updatePool(pid);
      if (user.amount > 0) {
         uint64_t pending = DecimalMath.mul(user.amount, pool.accDODOPerShare).sub(user.rewardDebt);
         safeDODOTransfer(getMsgSender(), pending);
      }
      IERC20(pool.lpToken).safeTransferFrom(address(getMsgSender()), address(this), _amount);
      user.amount     = user.amount.add(_amount);
      user.rewardDebt = DecimalMath.mul(user.amount, pool.accDODOPerShare);
   }

   void withdraw(address _lpToken, uint64_t _amount) {
      uint64_t  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      require(user.amount >= _amount, "withdraw too much");
      updatePool(pid);
      uint64_t pending = DecimalMath.mul(user.amount, pool.accDODOPerShare).sub(user.rewardDebt);
      safeDODOTransfer(getMsgSender(), pending);
      user.amount     = user.amount.sub(_amount);
      user.rewardDebt = DecimalMath.mul(user.amount, pool.accDODOPerShare);
      IERC20(pool.lpToken).safeTransfer(address(getMsgSender()), _amount);
   }

   void withdrawAll(address _lpToken) {
      uint64_t balance = getUserLpBalance(_lpToken, getMsgSender());
      withdraw(_lpToken, balance);
   }

   // Withdraw without caring about rewards. EMERGENCY ONLY.
   void emergencyWithdraw(address _lpToken) {
      uint64_t  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      IERC20(pool.lpToken).safeTransfer(address(getMsgSender()), user.amount);
      user.amount     = 0;
      user.rewardDebt = 0;
   }

   void claim(address _lpToken) {
      uint64_t pid = getPid(_lpToken);
      if (userInfo[pid][getMsgSender()].amount == 0 || poolInfos[pid].allocPoint == 0) {
         return; // save gas
      }
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      updatePool(pid);
      uint64_t pending = DecimalMath.mul(user.amount, pool.accDODOPerShare).sub(user.rewardDebt);
      user.rewardDebt = DecimalMath.mul(user.amount, pool.accDODOPerShare);
      safeDODOTransfer(getMsgSender(), pending);
   }

   void claimAll() {
      uint64_t length  = poolInfos.length;
      uint64_t pending = 0;
      for (uint64_t pid = 0; pid < length; ++pid) {
         if (userInfo[pid][getMsgSender()].amount == 0 || poolInfos[pid].allocPoint == 0) {
            continue; // save gas
         }
         PoolInfo  pool = poolInfos[pid];
         UserInfo  user = userInfo[pid][getMsgSender()];
         updatePool(pid);
         pending         = pending.add(DecimalMath.mul(user.amount, pool.accDODOPerShare).sub(user.rewardDebt));
         user.rewardDebt = DecimalMath.mul(user.amount, pool.accDODOPerShare);
      }
      safeDODOTransfer(getMsgSender(), pending);
   }

   // Safe DODO transfer function
   void safeDODOTransfer(address _to, uint64_t _amount) {
      IDODORewardVault(dodoRewardVault).reward(_to, _amount);
      realizedReward[_to] = realizedReward[_to].add(_amount);
   }
};
#endif