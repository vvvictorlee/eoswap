
#pragma once
#include <common/defines.hpp>

struct MigrationsStore {
   address owner;
   uint256 last_completed_migration;
   EOSLIB_SERIALIZE(MigrationsStore, (owner)(last_completed_migration))
};

struct ConstOracleStore {
   uint256 tokenPrice;
   EOSLIB_SERIALIZE(ConstOracleStore, (tokenPrice))
};

struct MinimumOracleStore {
   address _OWNER_;
   uint256 tokenPrice;
   EOSLIB_SERIALIZE(MinimumOracleStore, (_OWNER_)(tokenPrice))
};

struct Transaction {
   address destination;
   uint256 value;
   bytes   data;
   bool    executed;
EOSLIB_SERIALIZE(Transaction, (destination)(value)(data)(executed))
}

struct EmergencyCall {
   bytes32 selector;
   uint256 paramsBytesCount;
 EOSLIB_SERIALIZE(EmergencyCall, (selector)(paramsBytesCount))
}

struct a2b {
   std::map<address, bool> a2c;
   EOSLIB_SERIALIZE(a2b, (a2c))
};

struct MultiSigWalletWithTimelockStore {
   std::map<uint256, Transaction>   transactions;
   std::map<uint256, a2b>   confirmations;
   std::map<address, bool>  isOwner;
   std::map<address, address>   unlockTimes;

   std::vector<address> owners;
   uint256 required;
   uint256 transactionCount;

   // Functions bypass the time lock process
   std::vector<EmergencyCall>  emergencyCalls;

   bool mutex;

   EOSLIB_SERIALIZE(
       MultiSigWalletWithTimelockStore,
       (transactions)(confirmations)(isOwner)(unlockTimes)(owners)(required)(transactionCount)(emergencyCalls)(mutex))
};

struct TestERC20Store {
//    mapping(address = > mapping(address = > uint256)) allowed;

   std::string                 names;
   uint8                       decimals;
   std::map<name, uint256>     balances;
   std::map<name, Account2Amt> allowed;
   EOSLIB_SERIALIZE(TestERC20Store, (names)(decimals)(balances)(allowed))
};

struct WETH9Store {
//    string name           = "Wrapped Ether";
//    string symbol         = "WETH";
//    uint8 public decimals = 18;

//    mapping(address = > uint256) balanceOf;
//    mapping(address = > mapping(address = > uint256)) allowance;

   std::string                 names;
   std::string                 symbol;
   uint8                       decimals;
   std::map<name, uint>        balanceOf;
   std::map<name, Account2Amt> allowance;
   EOSLIB_SERIALIZE(WETH9tore, (names)(symbol)(decimals)(balanceOf)(allowance))
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
