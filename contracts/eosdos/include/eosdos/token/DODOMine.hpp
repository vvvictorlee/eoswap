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
   DODOMine(address _dodoToken, uint256 _startBlock) public {
      dodoRewardVault = address(new DODORewardVault(_dodoToken));
      startBlock      = _startBlock;
   }

   // ============ Modifiers ============

   void lpTokenExist(address lpToken) { require(lpTokenRegistry[lpToken] > 0, "LP Token Not Exist"); }

   void lpTokenNotExist(address lpToken) { require(lpTokenRegistry[lpToken] == 0, "LP Token Already Exist"); }

   // ============ Helper ============

   uint256 poolLength() { return poolInfos.length; }

   uint256 getPid(address _lpToken) {
      lpTokenExist();
      return lpTokenRegistry[_lpToken] - 1;
   }

   uint256 getUserLpBalance(address _lpToken, address _user) {
      uint256 pid = getPid(_lpToken);
      return userInfo[pid][_user].amount;
   }

   // ============ Ownable ============

   void addLpToken(address _lpToken, uint256 _allocPoint, bool _withUpdate) {
      lpTokenNotExist(_lpToken);
      onlyOwner();
      if (_withUpdate) {
         massUpdatePools();
      }
      uint256 lastRewardBlock = block.number > startBlock ? block.number : startBlock;
      totalAllocPoint         = totalAllocPoint.add(_allocPoint);
      poolInfos.push(PoolInfo(
          {lpToken : _lpToken, allocPoint : _allocPoint, lastRewardBlock : lastRewardBlock, accDODOPerShare : 0}));
      lpTokenRegistry[_lpToken] = poolInfos.length;
   }

   void setLpToken(address _lpToken, uint256 _allocPoint, bool _withUpdate) {
      onlyOwner();
      if (_withUpdate) {
         massUpdatePools();
      }
      uint256 pid               = getPid(_lpToken);
      totalAllocPoint           = totalAllocPoint.sub(poolInfos[pid].allocPoint).add(_allocPoint);
      poolInfos[pid].allocPoint = _allocPoint;
   }

   void setReward(uint256 _dodoPerBlock, bool _withUpdate) {
      onlyOwner();
      if (_withUpdate) {
         massUpdatePools();
      }
      dodoPerBlock = _dodoPerBlock;
   }

   // ============ View Rewards ============

   uint256 getPendingReward(address _lpToken, address _user) {
      uint256  pid                     = getPid(_lpToken);
      PoolInfo  pool            = poolInfos[pid];
      UserInfo  user            = userInfo[pid][_user];
      uint256          accDODOPerShare = pool.accDODOPerShare;
      uint256          lpSupply        = IERC20(pool.lpToken).balanceOf(address(this));
      if (block.number > pool.lastRewardBlock && lpSupply != 0) {
         uint256 DODOReward =
             block.number.sub(pool.lastRewardBlock).mul(dodoPerBlock).mul(pool.allocPoint).div(totalAllocPoint);
         accDODOPerShare = accDODOPerShare.add(DecimalMath.divFloor(DODOReward, lpSupply));
      }
      return DecimalMath.mul(user.amount, accDODOPerShare).sub(user.rewardDebt);
   }

   uint256 getAllPendingReward(address _user) {
      uint256 length      = poolInfos.length;
      uint256 totalReward = 0;
      for (uint256 pid = 0; pid < length; ++pid) {
         if (userInfo[pid][_user].amount == 0 || poolInfos[pid].allocPoint == 0) {
            continue; // save gas
         }
         PoolInfo  pool            = poolInfos[pid];
         UserInfo  user            = userInfo[pid][_user];
         uint256          accDODOPerShare = pool.accDODOPerShare;
         uint256          lpSupply        = IERC20(pool.lpToken).balanceOf(address(this));
         if (block.number > pool.lastRewardBlock && lpSupply != 0) {
            uint256 DODOReward =
                block.number.sub(pool.lastRewardBlock).mul(dodoPerBlock).mul(pool.allocPoint).div(totalAllocPoint);
            accDODOPerShare = accDODOPerShare.add(DecimalMath.divFloor(DODOReward, lpSupply));
         }
         totalReward = totalReward.add(DecimalMath.mul(user.amount, accDODOPerShare).sub(user.rewardDebt));
      }
      return totalReward;
   }

   uint256 getRealizedReward(address _user) { return realizedReward[_user]; }

   uint256 getDlpMiningSpeed(address _lpToken) {
      uint256  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      return dodoPerBlock.mul(pool.allocPoint).div(totalAllocPoint);
   }

   // ============ Update Pools ============

   // Update reward vairables for all pools. Be careful of gas spending!
   void massUpdatePools() {
      uint256 length = poolInfos.length;
      for (uint256 pid = 0; pid < length; ++pid) {
         updatePool(pid);
      }
   }

   // Update reward variables of the given pool to be up-to-date.
   void updatePool(uint256 _pid) {
      PoolInfo  pool = poolInfos[_pid];
      if (block.number <= pool.lastRewardBlock) {
         return;
      }
      uint256 lpSupply = IERC20(pool.lpToken).balanceOf(address(this));
      if (lpSupply == 0) {
         pool.lastRewardBlock = block.number;
         return;
      }
      uint256 DODOReward =
          block.number.sub(pool.lastRewardBlock).mul(dodoPerBlock).mul(pool.allocPoint).div(totalAllocPoint);
      pool.accDODOPerShare = pool.accDODOPerShare.add(DecimalMath.divFloor(DODOReward, lpSupply));
      pool.lastRewardBlock = block.number;
   }

   // ============ Deposit & Withdraw & Claim ============
   // Deposit & withdraw will also trigger claim

   void deposit(address _lpToken, uint256 _amount) {
      uint256  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      updatePool(pid);
      if (user.amount > 0) {
         uint256 pending = DecimalMath.mul(user.amount, pool.accDODOPerShare).sub(user.rewardDebt);
         safeDODOTransfer(getMsgSender(), pending);
      }
      IERC20(pool.lpToken).safeTransferFrom(address(getMsgSender()), address(this), _amount);
      user.amount     = user.amount.add(_amount);
      user.rewardDebt = DecimalMath.mul(user.amount, pool.accDODOPerShare);
   }

   void withdraw(address _lpToken, uint256 _amount) {
      uint256  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      require(user.amount >= _amount, "withdraw too much");
      updatePool(pid);
      uint256 pending = DecimalMath.mul(user.amount, pool.accDODOPerShare).sub(user.rewardDebt);
      safeDODOTransfer(getMsgSender(), pending);
      user.amount     = user.amount.sub(_amount);
      user.rewardDebt = DecimalMath.mul(user.amount, pool.accDODOPerShare);
      IERC20(pool.lpToken).safeTransfer(address(getMsgSender()), _amount);
   }

   void withdrawAll(address _lpToken) {
      uint256 balance = getUserLpBalance(_lpToken, getMsgSender());
      withdraw(_lpToken, balance);
   }

   // Withdraw without caring about rewards. EMERGENCY ONLY.
   void emergencyWithdraw(address _lpToken) {
      uint256  pid          = getPid(_lpToken);
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      IERC20(pool.lpToken).safeTransfer(address(getMsgSender()), user.amount);
      user.amount     = 0;
      user.rewardDebt = 0;
   }

   void claim(address _lpToken) {
      uint256 pid = getPid(_lpToken);
      if (userInfo[pid][getMsgSender()].amount == 0 || poolInfos[pid].allocPoint == 0) {
         return; // save gas
      }
      PoolInfo  pool = poolInfos[pid];
      UserInfo  user = userInfo[pid][getMsgSender()];
      updatePool(pid);
      uint256 pending = DecimalMath.mul(user.amount, pool.accDODOPerShare).sub(user.rewardDebt);
      user.rewardDebt = DecimalMath.mul(user.amount, pool.accDODOPerShare);
      safeDODOTransfer(getMsgSender(), pending);
   }

   void claimAll() {
      uint256 length  = poolInfos.length;
      uint256 pending = 0;
      for (uint256 pid = 0; pid < length; ++pid) {
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
   void safeDODOTransfer(address _to, uint256 _amount) {
      IDODORewardVault(dodoRewardVault).reward(_to, _amount);
      realizedReward[_to] = realizedReward[_to].add(_amount);
   }
};
#endif