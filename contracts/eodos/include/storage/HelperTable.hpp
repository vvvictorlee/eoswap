
#pragma once
#include <common/defines.hpp>

struct MigrationsStore {
   address owner;
   uint256 last_completed_migration;
   EOSLIB_SERIALIZE(Migrations, (bound)(index)(denorm)(balance)(exsym))
};

struct ConstOracleStore {
   uint256 tokenPrice;
   EOSLIB_SERIALIZE(ConstOracleStore, (bound)(index)(denorm)(balance)(exsym))
};

struct MinimumOracleStore {
   address _OWNER_;
   uint256 tokenPrice;
   EOSLIB_SERIALIZE(MinimumOracleStore, (bound)(index)(denorm)(balance)(exsym))
};

struct Transaction {
   address destination;
   uint256 value;
   bytes   data;
   bool    executed;
}

struct EmergencyCall {
   bytes32 selector;
   uint256 paramsBytesCount;
}

struct MultiSigWalletWithTimelockStore {
   uint256 constant MAX_OWNER_COUNT = 50;
   uint256          lockSeconds     = 86400;

   mapping(uint256 = > Transaction) public transactions;
   mapping(uint256 = > mapping(address = > bool)) public confirmations;
   mapping(address = > bool) public isOwner;
   mapping(uint256 = > uint256) public unlockTimes;

   address[] public owners;
   uint256 public required;
   uint256 public transactionCount;

   // Functions bypass the time lock process
   EmergencyCall[] public emergencyCalls;

   bool mutex;

   name factory;    // BFactory name to push token exitFee to
   name controller; // has CONTROL role
   bool publicSwap; // true if PUBLIC can call SWAP functions

   // `setSwapFee` and `finalize` require CONTROL
   // `finalize` sets `PUBLIC can SWAP`, `PUBLIC can JOIN`
   uint swapFee;
   bool finalized;

   std::vector<namesym>      tokens;
   std::map<namesym, Record> records;
   uint                      totalWeight;

   EOSLIB_SERIALIZE(
       MultiSigWalletWithTimelockStore,
       (mutex)(factory)(controller)(publicSwap)(swapFee)(finalized)(tokens)(records)(totalWeight))
};

struct TestERC20Store {
   string name;
   uint8 public decimals;

   mapping(address = > uint256) balances;
   mapping(address = > mapping(address = > uint256)) allowed;

   std::string                 names;
   std::string                 symbol;
   uint8                       decimals;
   std::map<name, uint>        balance;
   std::map<name, Account2Amt> allowance;
   uint                        totalSupply;
   EOSLIB_SERIALIZE(TestERC20Store, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};

struct WETH9Store {
   string name           = "Wrapped Ether";
   string symbol         = "WETH";
   uint8 public decimals = 18;

   mapping(address = > uint256) balanceOf;
   mapping(address = > mapping(address = > uint256)) allowance;

   std::string                 names;
   std::string                 symbol;
   uint8                       decimals;
   std::map<name, uint>        balance;
   std::map<name, Account2Amt> allowance;
   uint                        totalSupply;
   EOSLIB_SERIALIZE(WETH9tore, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};

struct UniswapV2ERC20Store {
   string constant name           = "Uniswap V2";
   string constant       symbol   = "UNI-V2";
   uint8 public constant decimals = 18;
   uint256               totalSupply;
   mapping(address = > uint256) balanceOf;
   mapping(address = > mapping(address = > uint256)) allowance;

   bytes32 public DOMAIN_SEPARATOR;
   mapping(address = > uint256) nonces;

   std::string                 names;
   std::string                 symbol;
   uint8                       decimals;
   std::map<name, uint>        balance;
   std::map<name, Account2Amt> allowance;
   uint                        totalSupply;
   EOSLIB_SERIALIZE(TestERC20Store, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};

struct IUniswapV2FactoryStore {
   address factory;
   address token0;
   address token1;

   uint112 private reserve0;          // uses single storage slot, accessible via getReserves
   uint112 private reserve1;          // uses single storage slot, accessible via getReserves
   uint32 private blockTimestampLast; // uses single storage slot, accessible via getReserves

   uint256 price0CumulativeLast;
   uint256 price1CumulativeLast;
   uint256 kLast; // reserve0 * reserve1, as of immediately after the most recent liquidity
   uint256 private unlocked = 1;

   std::string                 names;
   std::string                 symbol;
   uint8                       decimals;
   std::map<name, uint>        balance;
   std::map<name, Account2Amt> allowance;
   uint                        totalSupply;
   EOSLIB_SERIALIZE(TestERC20Store, (names)(symbol)(decimals)(balance)(allowance)(totalSupply))
};

struct NaiveOracleStore {
   uint256 tokenPrice;
   EOSLIB_SERIALIZE(
       MultiSigWalletWithTimelockStore,
       (mutex)(factory)(controller)(publicSwap)(swapFee)(finalized)(tokens)(records)(totalWeight))
};

struct [[eosio::table("poolstore"), eosio::contract("eoswap")]] BPoolStorage {
   std::map<name, BPoolStore> pools;
   EOSLIB_SERIALIZE(BPoolStorage, (pools))
};


struct UniswapArbitrageurStore {
   address _UNISWAP_;
   address _DODO_;
   address _BASE_;
   address _QUOTE_;

   bool _REVERSE_; // true if dodo.baseToken=uniswap.token0

   EOSLIB_SERIALIZE(
       MultiSigWalletWithTimelockStore,
       (mutex)(factory)(controller)(publicSwap)(swapFee)(finalized)(tokens)(records)(totalWeight))
};


typedef eosio::singleton<"helpstore"_n, BPoolStorage> BPoolStorageSingleton;
