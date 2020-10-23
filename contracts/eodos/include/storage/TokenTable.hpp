
#pragma once

#include <common/defines.hpp>




   // Info of each user.
   struct UserInfo {
      uint256 amount;     // How many LP tokens the user has provided.
      uint256 rewardDebt; // Reward debt. See explanation below.
                          //
                          // We do some fancy math here. Basically, any point in time, the amount of DODOs
                          // entitled to a user but is pending to be distributed is:
                          //
                          //   pending reward = (user.amount * pool.accDODOPerShare) - user.rewardDebt
                          //
                          // Whenever a user deposits or withdraws LP tokens to a pool. Here's what happens:
                          //   1. The pool's `accDODOPerShare` (and `lastRewardBlock`) gets updated.
                          //   2. User receives the pending reward sent to his/her address.
                          //   3. User's `amount` gets updated.
                          //   4. User's `rewardDebt` gets updated.
   }

   // Info of each pool.
   struct PoolInfo {
      address lpToken;         // Address of LP token contract.
      uint256 allocPoint;      // How many allocation points assigned to this pool. DODOs to distribute per block.
      uint256 lastRewardBlock; // Last block number that DODOs distribution occurs.
      uint256 accDODOPerShare; // Accumulated DODOs per share, times 1e12. See below.
   }

struct DODOMineStore {
   address dodoRewardVault;
   uint256 dodoPerBlock;

   // Info of each pool.
   PoolInfo[] public poolInfos;
   mapping(address = > uint256) lpTokenRegistry;

   // Info of each user that stakes LP tokens.
   mapping(uint256 = > mapping(address = > UserInfo)) userInfo;
   mapping(address = > uint256) realizedReward;

   // Total allocation points. Must be the sum of all allocation points in all pools.
   uint256 totalAllocPoint = 0;
   // The block number when DODO mining starts.
   uint256 startBlock;
  EOSLIB_SERIALIZE(Account2Amt, (dst2amt))
};

struct DODORewardVaultStore  {
  

   address dodoToken;
  EOSLIB_SERIALIZE(Account2Amt, (dst2amt))
};


struct Account2Amt {
  std::map<name, uint> dst2amt; // is token bound to pool

  EOSLIB_SERIALIZE(Account2Amt, (dst2amt))
};



struct DODOTokenStore {

   string symbol = "DODO";
   string name = "DODO bird";

   uint256 decimals = 18;
   uint256 totalSupply = 1000000000 * 10**18; // 1 Billion

    mapping(address => uint256)  balances;
    mapping(address => mapping(address => uint256))  allowed;

  std::string names;
  std::string symbol ;
  uint8 decimals ;
  std::map<name, uint> balance;
  std::map<name, Account2Amt> allowance;
  uint totalSupply;
  EOSLIB_SERIALIZE(DODOTokenStore, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};

struct LockedTokenVault {

  address _TOKEN_;

    mapping(address => uint256)  originBalances;
    mapping(address => uint256)  claimedBalances;

   uint256 _UNDISTRIBUTED_AMOUNT_;
   uint256 _START_RELEASE_TIME_;
   uint256 _RELEASE_DURATION_;
   uint256 _CLIFF_RATE_;

    bool _DISTRIBUTE_FINISHED_;

  std::string names;
  std::string symbol ;
  uint8 decimals ;
  std::map<name, uint> balance;
  std::map<name, Account2Amt> allowance;
  uint totalSupply;
  EOSLIB_SERIALIZE(DODOTokenStore, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};


struct [[eosio::table("tokenstore"), eosio::contract("eoswap")]] BTokenStorage {
  std::map<namesym, BTokenStore> tokens;
  EOSLIB_SERIALIZE(BTokenStorage, (tokens))
};

typedef eosio::singleton<"tokenstore"_n, BTokenStorage> BTokenStorageSingleton;