
#pragma once
#include <common/defines.hpp>

// Info of each user.
struct UserInfo {
   uint64_t amount;     // How many LP tokens the user has provided.
   uint64_t rewardDebt; // Reward debt. See explanation below.
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
   EOSLIB_SERIALIZE(UserInfo, (amount)(rewardDebt))
};

// Info of each pool.
struct PoolInfo {
   address lpToken;         // Address of LP token contract.
   uint64_t allocPoint;      // How many allocation points assigned to this pool. DODOs to distribute per block.
   uint64_t lastRewardBlock; // Last block number that DODOs distribution occurs.
   uint64_t accDODOPerShare; // Accumulated DODOs per share, times 1e12. See below.
   EOSLIB_SERIALIZE(PoolInfo, (lpToken)(allocPoint)(lastRewardBlock)(accDODOPerShare))
};

struct user2info {
   std::map<address, UserInfo> a2info; // is token bound to pool

   EOSLIB_SERIALIZE(user2info, (a2info))
};

struct DODOMineStore {
   address dodoRewardVault;
   uint64_t dodoPerBlock;

   // Info of each pool.
   std::vector<PoolInfo>      poolInfos;
   std::map<address, uint64_t> lpTokenRegistry;

   // Info of each user that stakes LP tokens.
   std::map<uint64_t, user2info> userInfo;
   std::map<address, uint64_t>   realizedReward;

   // Total allocation points. Must be the sum of all allocation points in all pools.
   uint64_t totalAllocPoint;
   // The block number when DODO mining starts.
   uint64_t startBlock;
   EOSLIB_SERIALIZE(
       DODOMineStore, (dodoRewardVault)(dodoPerBlock)(poolInfos)(lpTokenRegistry)(userInfo)(realizedReward)(
                          totalAllocPoint)(startBlock))
};

struct DODORewardVaultStore {
   address dodoToken;
   EOSLIB_SERIALIZE(DODORewardVaultStore, (dodoToken))
};

struct Account2Amt {
   std::map<name, uint64_t> dst2amt; // is token bound to pool

   EOSLIB_SERIALIZE(Account2Amt, (dst2amt))
};

   //    string symbol = "DODO";
   //    string name   = "DODO bird";

   //    uint64_t decimals    = 18;
   //    uint64_t totalSupply = 1000000000 * 10 * *18; // 1 Billion

   //    mapping(address = > uint64_t) balances;
   //    mapping(address = > mapping(address = > uint64_t)) allowed;
   //    string  symbol = "DLP";

struct TokenStore {
   extended_symbol originToken;
   extended_symbol esymbol;
   OwnableStore    ownable;
   std::string                 names;
//    name            contract_self;
//    std::string                 symbols;
//    uint64_t                     decimals;
//    std::map<name, uint64_t>     balances;
//    std::map<name, uint64_t>     balanceOf;
//    std::map<name, Account2Amt> allowed;
//    std::map<name, Account2Amt> allowance;
//    uint64_t                     totalSupply;
// (contract_self)(symbols)(decimals)(balances)(allowed)(
//                        balanceOf)(allowance)(totalSupply)
   EOSLIB_SERIALIZE(
       TokenStore, (esymbol)(originToken)(ownable)(names))
};

struct LockedTokenVaultStore {

   address _TOKEN_;

   std::map<name, uint64_t> originBalances;
   std::map<name, uint64_t> claimedBalances;

   uint64_t _UNDISTRIBUTED_AMOUNT_;
   uint64_t _START_RELEASE_TIME_;
   uint64_t _RELEASE_DURATION_;
   uint64_t _CLIFF_RATE_;

   bool _DISTRIBUTE_FINISHED_;

   EOSLIB_SERIALIZE(
       LockedTokenVaultStore, (_TOKEN_)(originBalances)(claimedBalances)(_UNDISTRIBUTED_AMOUNT_)(_START_RELEASE_TIME_)(
                                  _RELEASE_DURATION_)(_CLIFF_RATE_)(_DISTRIBUTE_FINISHED_))
};

struct [[eosio::table("token"), eosio::contract("eosdos")]] TokenStorage {
   std::map<namesym, TokenStore> tokens;
   EOSLIB_SERIALIZE(TokenStorage, (tokens))
};

typedef eosio::singleton<"token"_n, TokenStorage> TokenStorageSingleton;

struct [[eosio::table("mining"), eosio::contract("eosdos")]] MiningStorage {
   DODOMineStore         mine;
   DODORewardVaultStore  rewardvault;
   LockedTokenVaultStore lockedtoken;
   EOSLIB_SERIALIZE(MiningStorage, (mine)(rewardvault)(lockedtoken))
};

typedef eosio::singleton<"mining"_n, MiningStorage> MiningStorageSingleton;