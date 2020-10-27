
#pragma once
#pragma once
#include <common/defines.hpp>
#include<storage/LibTable.hpp>
#include <storage/TokenTable.hpp>

struct MigrationsStore {
   address owner;
   uint256 last_completed_migration;
   EOSLIB_SERIALIZE(MigrationsStore, (owner)(last_completed_migration))
};

struct OracleStore {
   OwnableStore ownable;
   address      _OWNER_;
   extended_asset      tokenPrice;
   EOSLIB_SERIALIZE(OracleStore, (ownable)(_OWNER_)(tokenPrice))
};

struct Transaction {
   address destination;
   uint256 value;
   bytes   data;
   bool    executed;
   EOSLIB_SERIALIZE(Transaction, (destination)(value)(data)(executed))
};

struct EmergencyCall {
   bytes32 selector;
   uint256 paramsBytesCount;
   EOSLIB_SERIALIZE(EmergencyCall, (selector)(paramsBytesCount))
};

struct a2b {
   std::map<address, bool> a2c;
   EOSLIB_SERIALIZE(a2b, (a2c))
};

struct MultiSigWalletWithTimelockStore {
   std::map<uint256, Transaction> transactions;
   std::map<uint256, a2b>         confirmations;
   std::map<address, bool>        isOwner;
   std::map<address, address>     unlockTimes;

   std::vector<address> owners;
   uint256              required;
   uint256              transactionCount;

   // Functions bypass the time lock process
   std::vector<EmergencyCall> emergencyCalls;

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
   std::map<name, uint256>        balanceOf;
   std::map<name, Account2Amt> allowance;
   EOSLIB_SERIALIZE(WETH9Store, (names)(symbol)(decimals)(balanceOf)(allowance))
};

struct UniswapV2ERC20Store {
   //    string constant name           = "Uniswap V2";
   //    string constant       symbol   = "UNI-V2";
   //    uint8 public constant decimals = 18;
   //    uint256               totalSupply;
   //    mapping(address = > uint256) balanceOf;
   //    mapping(address = > mapping(address = > uint256)) allowance;

   //    bytes32 public DOMAIN_SEPARATOR;
   //    mapping(address = > uint256) nonces;

   std::string                 names;
   std::string                 symbol;
   uint8                       decimals;
   std::map<name, uint256>        balanceOf;
   std::map<name, Account2Amt> allowance;
   uint256                        totalSupply;
   EOSLIB_SERIALIZE(UniswapV2ERC20Store, (names)(symbol)(decimals)(balanceOf)(allowance)(totalSupply))
};

struct IUniswapV2FactoryStore {
   address factory;
   address token0;
   address token1;

   uint112 reserve0;           // uses single storage slot, accessible via getReserves
   uint112 reserve1;           // uses single storage slot, accessible via getReserves
   uint32  blockTimestampLast; // uses single storage slot, accessible via getReserves

   uint256 price0CumulativeLast;
   uint256 price1CumulativeLast;
   uint256 kLast; // reserve0 * reserve1, as of immediately after the most recent liquidity
   uint256 unlocked = 1;

   EOSLIB_SERIALIZE(
       IUniswapV2FactoryStore, (factory)(token0)(token1)(reserve0)(reserve1)(blockTimestampLast)(price0CumulativeLast)(
                                   price1CumulativeLast)(kLast)(unlocked))
};

struct UniswapArbitrageurStore {
   address _UNISWAP_;
   address _DODO_;
   address _BASE_;
   address _QUOTE_;

   bool _REVERSE_; // true if dodo.baseToken=uniswap.token0

   EOSLIB_SERIALIZE(UniswapArbitrageurStore, (_UNISWAP_)(_DODO_)(_BASE_)(_QUOTE_)(_REVERSE_))
};

struct [[eosio::table("helpstore"), eosio::contract("eosdos")]] HelperStorage {
   MigrationsStore                 mig;
   MultiSigWalletWithTimelockStore msig;
   TestERC20Store                  testerc20;
   WETH9Store                      weth9;
   UniswapV2ERC20Store             erc20;
   IUniswapV2FactoryStore          factory;
   UniswapArbitrageurStore         arbit;

   EOSLIB_SERIALIZE(HelperStorage, (mig)(msig)(testerc20)(weth9)(erc20)(factory)(arbit))
};

typedef eosio::singleton<"helpstore"_n, HelperStorage> HelperStorageSingleton;

struct [[eosio::table("oracle"), eosio::contract("eosdos")]] OracleStorage {
   std::map<namesym, OracleStore> oracles;
   EOSLIB_SERIALIZE(OracleStorage, (oracles))
};

typedef eosio::singleton<"oracle"_n, OracleStorage> OracleStorageSingleton;
