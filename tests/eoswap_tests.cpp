#include "eosio.system_tester.hpp"
#include <boost/test/unit_test.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/testing/tester.hpp>

#include "Runtime/Runtime.h"

#include <algorithm>
#include <boost/multiprecision/cpp_int.hpp>
#include <fc/variant_object.hpp>
#include <sstream>
using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo      = fc::mutable_variant_object;
using uint256m = uint64_t;
using namesym  = eosio::chain::uint128_t;

#define EOSWAP_DEBUG
#ifdef EOSWAP_DEBUG
#define LINE_DEBUG BOOST_TEST_CHECK(__LINE__ == 0);
#else
#define LINE_DEBUG
#endif
const int         ONE_DECIMALS      = 9;
const std::string default_lp_symbol = "EPT";

class findx {
 public:
   findx(const string str) { test = str; }
   bool operator()(const fc::variant& v) {
      if (v["key"].as_string().compare(test) == 0) {
         return true;
      }
      return false;
   }

 private:
   string test;
};

class eoswap_tester : public tester {
 public:
   eoswap_tester() {
      produce_blocks(2);

      create_accounts({N(eosio.token), N(eosio.ram), N(eosio.ramfee), N(eosio.stake), N(eosio.bpay), N(eosio.vpay),
                       N(eosio.saving), N(eosio.names), N(eosio.rex)});

      create_accounts({N(alice), N(bob), N(carol), N(eoswapeoswap)});
      create_accounts({N(poolmanagers), N(poolfactorys), N(blabsblabs11), N(tokenissuers)});
      create_accounts({N(weth), N(dai), N(mkr), N(xxx), N(roxe.ro)});
      create_accounts({N(extendxtoken)});
      produce_blocks(2);
      admin         = N(eoswapeoswap);
      nonadmin      = N(alice);
      user1         = N(bob);
      user2         = N(carol);
      tokenissuer   = N(tokenissuers);
      newcontroller = N(poolmanagers);
      newblabs      = N(blabsblabs11);
      set_code(N(eoswapeoswap), contracts::swap_wasm());
      set_abi(N(eoswapeoswap), contracts::swap_abi().data());
      produce_blocks(2);

      set_code(N(eosio.token), contracts::token_wasm());
      set_abi(N(eosio.token), contracts::token_abi().data());

      create_currency(N(eosio.token), config::system_account_name, core_sym::from_string("10000000000.0000"));
      issue(config::system_account_name, core_sym::from_string("1000000000.0000"));
      BOOST_REQUIRE_EQUAL(
          core_sym::from_string("1000000000.0000"), get_balance(config::system_account_name) +
                                                        get_balance(N(eosio.ramfee)) + get_balance(N(eosio.stake)) +
                                                        get_balance(N(eosio.ram)));

      create_currency(
          N(eosio.token), config::system_account_name, eosio::chain::asset::from_string("1000000.000000 EOS"));

      issue(config::system_account_name, eosio::chain::asset::from_string("1000000.000000 EOS"));

      set_code(config::system_account_name, contracts::system_wasm());
      set_abi(config::system_account_name, contracts::system_abi().data());
      produce_blocks();
      base_tester::push_action(
          config::system_account_name, N(init), config::system_account_name,
          mutable_variant_object()("version", 0)("core", CORE_SYM_STR));
      produce_blocks();

      create_account_with_resources(N(alice1111111), N(eosio), core_sym::from_string("1.0000"), false);
      create_account_with_resources(N(bob111111111), N(eosio), core_sym::from_string("0.4500"), false);
      create_account_with_resources(N(carol1111111), N(eosio), core_sym::from_string("1.0000"), false);
      create_account_with_resources(N(david1111111), N(eosio), core_sym::from_string("1.0000"), false);
      create_account_with_resources(N(dai2mkr11111), N(eosio), core_sym::from_string("1.0000"), false);

      set_code(N(roxe.ro), contracts::token_wasm());
      set_abi(N(roxe.ro), contracts::token_abi().data());

      std::vector<string> accounts  = {"alice1111111", "bob111111111", "carol1111111", "david1111111"};
      std::vector<name>   taccounts = {N(alice), N(bob), N(eoswapeoswap)};
      std::vector<string> tokens    = {"BTC", "USD", "MKR", "XXX"};
      int                 j         = 0;
      std::string         amt       = "100000.000000000";
      std::string         memo      = "";
      //   for (auto& acc_name : accounts) {
      //      std::string token     = tokens[j++];
      //      std::string amount    = "100000000.000000000" + token;
      //      std::string tamount   = amt + token;
      //      asset       maxsupply = eosio::chain::asset::from_string(amount.c_str());
      //      name        acc       = name(acc_name.c_str());

      //      create_currency(N(roxe.ro), acc, maxsupply);
      //      issuex(N(roxe.ro), acc, maxsupply, acc);
      //      produce_blocks(1);
      //      for (auto& to : taccounts) {
      //         transferx(N(roxe.ro), acc, to, tamount, acc, memo);
      //      }
      //   }

      is_auth_token = false;
      produce_blocks();

      const auto& accnt = control->db().get<account_object, by_name>(N(eoswapeoswap));
      abi_def     abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
      abi_ser.set_abi(abi, abi_serializer_max_time);
   }

   auto push_permission_update_auth_action(const account_name& signer) {
      auto auth = authority(eosio::testing::base_tester::get_public_key(signer, "active"));
      auth.accounts.push_back(permission_level_weight{{N(eoswapeoswap), config::eosio_code_name}, 1});

      return base_tester::push_action(
          N(eosio), N(updateauth), signer,
          mvo()("account", signer)("permission", "active")("parent", "owner")("auth", auth));
   }

   action_result push_action(const account_name& signer, const action_name& name, const variant_object& data) {
      if (!is_auth_token) {
         is_auth_token = true;
         push_permission_update_auth_action(N(poolmanagers));
         push_permission_update_auth_action(N(dai2mkr11111));
         push_permission_update_auth_action(N(tokenissuers));
         push_permission_update_auth_action(N(roxe.ro));
         push_permission_update_auth_action(admin);
         push_permission_update_auth_action(nonadmin);
         push_permission_update_auth_action(user1);
         push_permission_update_auth_action(user2);
         push_permission_update_auth_action(N(alice1111111));
      }

      string action_type_name = abi_ser.get_action_type(name);

      action act;
      act.account = N(eoswapeoswap);
      act.name    = name;
      act.data    = abi_ser.variant_to_binary(action_type_name, data, abi_serializer_max_time);

      return base_tester::push_action(std::move(act), signer.to_uint64_t());
   }

   transaction_trace_ptr create_account_with_resources(
       account_name a, account_name creator, asset ramfunds, bool multisig,
       asset net = core_sym::from_string("10.0000"), asset cpu = core_sym::from_string("10.0000")) {
      signed_transaction trx;
      set_transaction_headers(trx);

      authority owner_auth;
      if (multisig) {
         // multisig between account's owner key and creators active permission
         owner_auth = authority(
             2, {key_weight{get_public_key(a, "owner"), 1}},
             {permission_level_weight{{creator, config::active_name}, 1}});
      } else {
         owner_auth = authority(get_public_key(a, "owner"));
      }

      trx.actions.emplace_back(
          vector<permission_level>{{creator, config::active_name}},
          newaccount{
              .creator = creator, .name = a, .owner = owner_auth, .active = authority(get_public_key(a, "active"))});

      trx.actions.emplace_back(get_action(
          N(eosio), N(buyram), vector<permission_level>{{creator, config::active_name}},
          mvo()("payer", creator)("receiver", a)("quant", ramfunds)));

      trx.actions.emplace_back(get_action(
          N(eosio), N(delegatebw), vector<permission_level>{{creator, config::active_name}},
          mvo()("from", creator)("receiver", a)("stake_net_quantity", net)("stake_cpu_quantity", cpu)("transfer", 0)));

      set_transaction_headers(trx);
      trx.sign(get_private_key(creator, "active"), control->get_chain_id());
      return push_transaction(trx);
   }
   void create_currency(name contract, name manager, asset maxsupply) {
      auto act = mutable_variant_object()("issuer", manager)("maximum_supply", maxsupply);

      base_tester::push_action(contract, N(create), contract, act);
   }
   void issuex(name contract, name to, const asset& amount, name manager = config::system_account_name) {
      base_tester::push_action(
          contract, N(issue), manager, mutable_variant_object()("to", to)("quantity", amount)("memo", ""));
   }

   void issue(name to, const asset& amount, name manager = config::system_account_name) {
      base_tester::push_action(
          N(eosio.token), N(issue), manager, mutable_variant_object()("to", to)("quantity", amount)("memo", ""));
   }
   void transfer(
       name from, name to, const string& amount, name manager = config::system_account_name,
       const std::string& memo = "") {
      base_tester::push_action(
          N(eosio.token), N(transfer), manager,
          mutable_variant_object()("from", from)("to", to)("quantity", core_sym::from_string(amount))("memo", memo));
   }

   void transferx(
       name contract, name from, name to, const string& amount, name manager = config::system_account_name,
       const std::string& memo = "") {
      base_tester::push_action(
          contract, N(transfer), manager,
          mutable_variant_object()("from", from)("to", to)("quantity", eosio::chain::asset::from_string(amount))(
              "memo", memo));
   }

   asset get_balance(const account_name& act) {
      // return get_currency_balance( config::system_account_name,
      // symbol(CORE_SYMBOL), act ); temporary code. current get_currency_balancy
      // uses table name N(accounts) from currency.h generic_currency table name
      // is N(account).
      const auto& db = control->db();
      const auto* tbl =
          db.find<table_id_object, by_code_scope_table>(boost::make_tuple(N(eosio.token), act, N(accounts)));
      share_type result = 0;

      // the balance is implied to be 0 if either the table or row does not exist
      if (tbl) {
         const auto* obj =
             db.find<key_value_object, by_scope_primary>(boost::make_tuple(tbl->id, symbol(CORE_SYM).to_symbol_code()));
         if (obj) {
            // balance is the first field in the serialization
            fc::datastream<const char*> ds(obj->value.data(), obj->value.size());
            fc::raw::unpack(ds, result);
         }
      }
      return asset(result, symbol(CORE_SYM));
   }

   asset get_balancex(const account_name& act, const symbol& sym, name contract = N(roxe.ro)) {
      // return get_currency_balance( config::system_account_name,
      // symbol(CORE_SYMBOL), act ); temporary code. current get_currency_balancy
      // uses table name N(accounts) from currency.h generic_currency table name
      // is N(account).
      const auto& db     = control->db();
      const auto* tbl    = db.find<table_id_object, by_code_scope_table>(boost::make_tuple(contract, act, N(accounts)));
      share_type  result = 0;

      // the balance is implied to be 0 if either the table or row does not exist
      if (tbl) {
         const auto* obj =
             db.find<key_value_object, by_scope_primary>(boost::make_tuple(tbl->id, sym.to_symbol_code()));
         if (obj) {
            // balance is the first field in the serialization
            fc::datastream<const char*> ds(obj->value.data(), obj->value.size());
            fc::raw::unpack(ds, result);
         }
      }
      return asset(result, sym);
   }

   /////////////factory///////////
   action_result setblabs(account_name msg_sender, account_name blabs) {
      return push_action(msg_sender, N(setblabs), mvo()("msg_sender", msg_sender)("blabs", blabs));
   }

   action_result collect(account_name msg_sender, account_name pool_name) {
      return push_action(msg_sender, N(collect), mvo()("msg_sender", msg_sender)("pool_name", pool_name));
   }

   action_result newpool(account_name msg_sender, account_name pool_name) {
      return push_action(msg_sender, N(newpool), mvo()("msg_sender", msg_sender)("pool_name", pool_name));
   }

   ///////////////pool ////////////////////
   // msg_sender, name dst, uint256m amt
   action_result setswapfee(account_name msg_sender, account_name pool_name, uint256m swapFee) {
      return push_action(
          msg_sender, N(setswapfee), mvo()("msg_sender", msg_sender)("pool_name", pool_name)("swapFee", swapFee));
   }

   action_result setcontroler(account_name msg_sender, account_name pool_name, name manager) {
      return push_action(
          msg_sender, N(setcontroler), mvo()("msg_sender", msg_sender)("pool_name", pool_name)("manager", manager));
   }

   action_result setpubswap(account_name msg_sender, account_name pool_name, bool public_) {
      return push_action(
          msg_sender, N(setpubswap), mvo()("msg_sender", msg_sender)("pool_name", pool_name)("public_", public_));
   }

   action_result finalize(account_name msg_sender, account_name pool_name) {
      return push_action(msg_sender, N(finalize), mvo()("msg_sender", msg_sender)("pool_name", pool_name));
   }

   action_result bind(account_name msg_sender, account_name pool_name, const extended_asset& balance, uint256m denorm) {
      return push_action(
          msg_sender, N(bind),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("balance", balance)("denorm", denorm));
   }

   action_result
   rebind(account_name msg_sender, account_name pool_name, const extended_asset& balance, uint256m denorm) {
      return push_action(
          msg_sender, N(rebind),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("balance", balance)("denorm", denorm));
   }

   action_result unbind(account_name msg_sender, account_name pool_name, const extended_symbol& token) {
      return push_action(
          msg_sender, N(unbind), mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token));
   }

   action_result gulp(account_name msg_sender, account_name pool_name, const extended_symbol& token) {
      return push_action(msg_sender, N(gulp), mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token));
   }

   action_result joinpool(
       account_name msg_sender, account_name pool_name, uint256m poolAmountOut, std::vector<uint256m> maxAmountsIn) {
      return push_action(
          msg_sender, N(joinpool),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("poolAmountOut", poolAmountOut)(
              "maxAmountsIn", maxAmountsIn));
   }

   action_result exitpool(
       account_name msg_sender, account_name pool_name, uint256m poolAmountIn, std::vector<uint256m> minAmountsOut) {
      return push_action(
          msg_sender, N(exitpool),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("poolAmountIn", poolAmountIn)(
              "minAmountsOut", minAmountsOut));
   }

   action_result swapamtin(
       name msg_sender, name pool_name, const extended_asset& tokenAmountIn, const extended_asset& minAmountOut,
       uint64_t maxPrice) {
      return push_action(
          msg_sender, N(swapamtin),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("tokenAmountIn", tokenAmountIn)(
              "minAmountOut", minAmountOut)("maxPrice", maxPrice));
   }

   action_result swapamtout(
       name msg_sender, name pool_name, const extended_asset& maxAmountIn, const extended_asset& tokenAmountOut,
       uint64_t maxPrice) {
      return push_action(
          msg_sender, N(swapamtout),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("maxAmountIn", maxAmountIn)(
              "tokenAmountOut", tokenAmountOut)("maxPrice", maxPrice));
   }

/////////////////////Test   swap in out ///////////
   action_result tswapamtin(
       name msg_sender, name pool_name, const extended_asset& tokenAmountIn, const extended_asset& minAmountOut,
       uint64_t maxPrice, const std::vector<int64_t>& params) {
      return push_action(
          msg_sender, N(tswapamtin),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("tokenAmountIn", tokenAmountIn)(
              "minAmountOut", minAmountOut)("maxPrice", maxPrice)("params",params));
   }

   action_result tswapamtout(
       name msg_sender, name pool_name, const extended_asset& maxAmountIn, const extended_asset& tokenAmountOut,
       uint64_t maxPrice, const std::vector<int64_t>& params) {
      return push_action(
          msg_sender, N(tswapamtout),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("maxAmountIn", maxAmountIn)(
              "tokenAmountOut", tokenAmountOut)("maxPrice", maxPrice)("params",params));
   }

   action_result cppool2table(account_name msg_sender, account_name pool_name) {
      return push_action(msg_sender, N(cppool2table), mvo()("msg_sender", msg_sender)("pool_name", pool_name));
   }

   ////////////////TOKEN//////////////
   action_result extransfer(name from, name to, const extended_asset& quantity, const std::string& memo = "") {
      return push_action(from, N(extransfer), mvo()("from", from)("to", to)("quantity", quantity)("memo", memo));
   }

   action_result newtoken(account_name msg_sender, const extended_asset& token) {
      return push_action(msg_sender, N(newtoken), mvo()("msg_sender", msg_sender)("token", token));
   }

   action_result newtokenex(account_name msg_sender, const extended_asset& token) {
      return push_action(N(eoswapeoswap), N(newtokenex), mvo()("msg_sender", msg_sender)("token", token));
   }
   action_result transferex(account_name msg_sender, account_name dst, const extended_asset& amt) {
      return push_action(msg_sender, N(transferex), mvo()("msg_sender", msg_sender)("dst", dst)("amt", amt));
   }

   action_result mint(account_name msg_sender, const extended_asset& amt) {
      return push_action(msg_sender, N(mint), mvo()("msg_sender", msg_sender)("amt", amt));
   }
   action_result mintex(account_name msg_sender, const extended_asset& amt) {
      return push_action(tokenissuer, N(mintex), mvo()("msg_sender", msg_sender)("amt", amt));
   }
   action_result burn(account_name msg_sender, const extended_asset& amt) {
      return push_action(msg_sender, N(burn), mvo()("msg_sender", msg_sender)("amt", amt));
   }
   action_result burnex(account_name msg_sender, const extended_asset& amt) {
      return push_action(msg_sender, N(burnex), mvo()("msg_sender", msg_sender)("amt", amt));
   }
   // symbol& symbol
   action_result setparameter(const std::string& symbol, const std::vector<int64_t> params) {
      return push_action(N(eoswapeoswap), N(setparameter), mvo()("symbol", symbol)("params", params));
   }

   ////////////////get table//////////////

   fc::variant get_factory_store() {
      vector<char> data = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap), N(factorystore), N(factorystore));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("BFactoryStorage", data, abi_serializer_max_time);
   }

   fc::variant get_pool_table(name pool_name) {
      name         table_name = N(pools);
      vector<char> data       = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap), table_name, pool_name);
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("pool_storage", data, abi_serializer_max_time);
   }

   ///////////////utils///////////////////////////
   void to_kv_helper(const fc::variant& v, std::function<void(const std::string&, const std::string&)>&& append) {
      if (v.is_object()) {
         const auto&              obj = v.get_object();
         static const std::string sep = ".";

         for (const auto& entry : obj) {
            to_kv_helper(entry.value(), [&append, &entry](const std::string& path, const std::string& value) {
               append(sep + entry.key() + path, value);
            });
         }
      } else if (v.is_array()) {
         const auto& arr = v.get_array();
         for (size_t idx = 0; idx < arr.size(); idx++) {
            const auto& entry = arr.at(idx);
            to_kv_helper(entry, [&append, idx](const std::string& path, const std::string& value) {
               append(std::string("[") + std::to_string(idx) + std::string("]") + path, value);
            });
         }
      } else if (!v.is_null()) {
         append("", v.as_string());
      }
   }

   auto to_kv(const fc::variant& v) {
      std::map<std::string, std::string> result;
      to_kv_helper(v, [&result](const std::string& k, const std::string& v) { result.emplace(k, v); });
      return result;
   }

   std::string find_value(const fc::variant& v, const std::string& key) {
      auto a = v.get_array();
      auto i = std::find_if(a.begin(), a.end(), findx(key));
      if (i != a.end()) {
         fc::variant vv = *i;
         return vv["value"].as_string();
      }
      return std::string("");
   }

   fc::variant find_variant(const fc::variant& v, const std::string& key) {
      auto a = v.get_array();
      auto i = std::find_if(a.begin(), a.end(), findx(key));
      if (i != a.end()) {
         fc::variant vv = *i;
         return vv["value"];
      }
      return fc::variant();
   }

   bool isBPool(name pool_name) {
      const auto f = get_factory_store();
      const auto p = find_variant(f["isBPool"], pool_name.to_string());
      return p.as<bool>();
   }

   fc::variant records(name pool_name, const extended_symbol& token) {
      const auto ps = get_pool_table(pool_name);
      const auto r  = find_variant(ps["pools"]["records"], ns_to_string(to_namesym(token)));
      return r;
   }

   fc::variant pools(name pool_name) {
      const auto ps = get_pool_table(pool_name);
      //   const auto p  = find_variant(ps["pools"], pool_name.to_string());
      const auto p = ps["pools"];

      return p;
   }

   uint8_t get_decimal(const std::string& sym) {
      std::map<std::string, uint8_t> sym2dec = {std::make_pair("BTC", 8), std::make_pair("USD", 6)};
      auto                           it      = sym2dec.find(sym);
      uint8_t                        dec     = ONE_DECIMALS;
      if (it != sym2dec.end()) {
         dec = it->second;
      }

      return dec;
   }

   std::string get_decimal_str(const std::string& sym) { return std::to_string(get_decimal(sym)) + "," + sym; }

   uint256m to_wei(uint256m value, uint8_t decimal = ONE_DECIMALS) { return value * pow(10, decimal); }

   uint256m to_weight(uint256m value, uint8_t decimal = 4) { return value * pow(10, decimal); }

   extended_asset to_pool_asset(name pool_name, int64_t value) {
      return extended_asset{asset{value, symbol{ONE_DECIMALS, default_lp_symbol.c_str()}}, pool_name};
   }

   extended_symbol to_pool_sym(name pool_name) {
      return extended_symbol{symbol{ONE_DECIMALS, default_lp_symbol.c_str()}, pool_name};
   }

   extended_asset to_ext_asset(int64_t value, const std::string& sym) {
      return extended_asset{asset{value, to_sym(sym).sym}, name{"roxe.ro"}};
   }

   extended_asset to_asset(int64_t value, const std::string& sym) {
      return extended_asset{asset{value, to_sym(sym).sym}, name{"roxe.ro"}};
   }
   extended_asset to_wei_asset(uint256m value, const std::string& sym) {
      return to_asset(static_cast<int64_t>(to_wei(value, get_decimal(sym))), sym);
   }

   extended_symbol to_sym(const std::string& sym) {
      return extended_symbol{symbol{get_decimal(sym), sym.c_str()}, name{"roxe.ro"}};
   }

   extended_asset to_maximum_supply(const std::string& sym) { return to_asset(static_cast<int64_t>(pow(10, 18)), sym); }

   namesym to_namesym(const extended_symbol& exsym) {
      namesym ns = exsym.contract.to_uint64_t();
      return ns << 64 | exsym.sym.to_symbol_code().value;
   }

   std::string boost_to_string(boost::multiprecision::uint128_t t) {
      using namespace boost::multiprecision;
      std::stringstream ss;
      ss << t;
      return ss.str();
   }

   std::string ns_to_string(namesym ns) {
      std::string s = boost_to_string(ns);
      return s;
   }

   void newpoolBefore() {
      LINE_DEBUG;
      newpool(admin, N(dai2mkr11111));
      setcontroler(admin, N(dai2mkr11111), newcontroller);
   }

   void setswapfeeBefore() {
      LINE_DEBUG;
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(newcontroller, N(dai2mkr11111), 3000);
   }

   void setswapfeeBefore1() {
      LINE_DEBUG;
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(newcontroller, N(dai2mkr11111), 1);
   }

   void setswapfeeBefore2() {
      LINE_DEBUG;
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(newcontroller, N(dai2mkr11111), 1000000);
   }

   void mintBefore() {
      LINE_DEBUG;
      newtoken(tokenissuer, to_maximum_supply("BTC"));
      newtoken(tokenissuer, to_maximum_supply("MKR"));
      newtoken(tokenissuer, to_maximum_supply("USD"));
      newtoken(tokenissuer, to_maximum_supply("XXX"));

      mint(admin, to_wei_asset(50, "BTC"));
      mint(admin, to_wei_asset(20, "MKR"));
      mint(admin, to_wei_asset(10000, "USD"));
      mint(admin, to_wei_asset(10, "XXX"));

      mint(newcontroller, to_wei_asset(50, "BTC"));
      mint(newcontroller, to_wei_asset(20, "MKR"));
      mint(newcontroller, to_wei_asset(10000, "USD"));
      mint(newcontroller, to_wei_asset(10, "XXX"));

      mint(user1, to_wei_asset(25, "BTC"));
      mint(user1, to_wei_asset(4, "MKR"));
      mint(user1, to_wei_asset(40000, "USD"));
      mint(user1, to_wei_asset(10, "XXX"));

      mint(user2, to_asset(12222200, "BTC"));
      mint(user2, to_asset(1015333, "MKR"));
      mint(user2, to_asset(1, "USD"));
      mint(user2, to_wei_asset(51, "XXX"));

      mint(nonadmin, to_wei_asset(1, "BTC"));
      mint(nonadmin, to_wei_asset(200, "USD"));
   }

   void bindBefore() {
      LINE_DEBUG;
      bind(newcontroller, N(dai2mkr11111), to_wei_asset(50, "BTC"), to_weight(5));
      bind(newcontroller, N(dai2mkr11111), to_wei_asset(20, "MKR"), to_weight(5));
      bind(newcontroller, N(dai2mkr11111), to_wei_asset(10000, "USD"), to_weight(5));
   }

   void finalizeBefore() {
      LINE_DEBUG;
      finalize(newcontroller, N(dai2mkr11111));
   }

   void joinpoolBefore() {
      LINE_DEBUG;
      std::vector<uint256m> v{uint256m(-1), uint256m(-1), uint256m(-1)};
      joinpool(user1, N(dai2mkr11111), to_weight(5), v);
   }

   void before() {
      newpoolBefore();
      setswapfeeBefore();
      mintBefore();
      bindBefore();
      finalizeBefore();
      joinpoolBefore();
   }

   void mintBefore1() {
      LINE_DEBUG;
      newtoken(tokenissuer, to_maximum_supply("BTC"));
      newtoken(tokenissuer, to_maximum_supply("USD"));

      mint(admin, to_wei_asset(5, "BTC"));
      mint(admin, to_wei_asset(200, "USD"));

      mint(newcontroller, to_wei_asset(10000, "BTC"));
      mint(newcontroller, to_wei_asset(20000, "USD"));

      mint(nonadmin, to_wei_asset(10000, "BTC"));
      mint(nonadmin, to_wei_asset(20000, "USD"));

      mint(user1, to_wei_asset(90000, "BTC"));
      mint(user1, to_wei_asset(90000, "USD"));
   }

   void bindBefore1() {
      LINE_DEBUG;
      bind(newcontroller, N(dai2mkr11111), to_wei_asset(10000, "BTC"), to_weight(5));
      bind(newcontroller, N(dai2mkr11111), to_wei_asset(20000, "USD"), to_weight(5));
   }

   void joinpoolBefore1() {
      LINE_DEBUG;
      std::vector<uint256m> v{uint256m(-1), uint256m(-1)};
      joinpool(nonadmin, N(dai2mkr11111), to_wei(10), v);
   }

   void exitpoolBefore1() {
      LINE_DEBUG;
      exitpool(nonadmin, N(dai2mkr11111), to_wei(10), std::vector<uint256m>{0, 0});
   }
   void before1() {
      newpoolBefore();
      setswapfeeBefore1();
      mintBefore1();
      bindBefore1();
      finalizeBefore();
      joinpoolBefore1();
      exitpoolBefore1();
   }

   void before2() {
      name pool_name = N(dai2mkr11111);
      LINE_DEBUG;
      newpool(admin, pool_name);
      setcontroler(admin, pool_name, newcontroller);

      LINE_DEBUG;
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(newcontroller, pool_name, 1000000);

      std::vector<name>        users{admin, newcontroller, nonadmin, user1};
      std::vector<std::string> tokens{"BTC", "USD"};
      LINE_DEBUG;
      for (auto token : tokens) {
         newtoken(tokenissuer, to_maximum_supply(token));
      }

      for (auto token : tokens) {
         for (auto user : users) {
            mint(user, to_wei_asset(90000, token));
         }
      }

      mint(user2, to_wei_asset(1, "BTC"));
      mint(user2, to_wei_asset(1, "USD"));

      std::vector<std::tuple<uint64_t, std::string, uint64_t>> bind_data{std::make_tuple(100, "BTC", 5),
                                                                         std::make_tuple(200, "USD", 5)};
      LINE_DEBUG;
      for (auto d : bind_data) {
         bind(newcontroller, pool_name, to_wei_asset(std::get<0>(d), std::get<1>(d)), to_wei(std::get<2>(d)));
      }

      LINE_DEBUG;
      finalize(newcontroller, pool_name);

      LINE_DEBUG;
      std::vector<uint256m> v{uint256m(-1), uint256m(-1), uint256m(-1)};
      joinpool(user1, pool_name, to_weight(5), v);
   }

   void before3() {
      name pool_name = N(dai2mkr11111);
      LINE_DEBUG;
      newpool(admin, pool_name);
      setcontroler(admin, pool_name, newcontroller);

      LINE_DEBUG;
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(newcontroller, pool_name, 1000000);

      std::vector<name>        users{admin, newcontroller, nonadmin, user1};
      std::vector<std::string> tokens{"BTC", "USD"};
      LINE_DEBUG;
      for (auto token : tokens) {
         newtoken(tokenissuer, to_maximum_supply(token));
      }

      for (auto token : tokens) {
         for (auto user : users) {
            mint(user, to_wei_asset(200000, token));
         }
      }

      mint(user2, to_wei_asset(1, "BTC"));
      mint(user2, to_wei_asset(1, "USD"));

      std::vector<std::tuple<uint64_t, std::string, uint64_t>> bind_data{std::make_tuple(10000000000000, "BTC", 5),
                                                                         std::make_tuple(191170019117, "USD", 5)};
      LINE_DEBUG;
      for (auto d : bind_data) {
         bind(newcontroller, pool_name, to_asset(std::get<0>(d), std::get<1>(d)), to_wei(std::get<2>(d)));
      }

      LINE_DEBUG;
      finalize(newcontroller, pool_name);

      //   LINE_DEBUG;
      //   std::vector<uint256m> v{uint256m(-1), uint256m(-1), uint256m(-1)};
      //   joinpool(user1, pool_name, to_weight(5), v);
   }

   void before4maxsupply() {
      name pool_name = N(dai2mkr11111);
      LINE_DEBUG;
      newpool(admin, pool_name);
      setcontroler(admin, pool_name, newcontroller);

      LINE_DEBUG;
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(newcontroller, pool_name, 1000000);

      std::vector<name>        users{admin, newcontroller, nonadmin, user1};
      std::vector<std::string> tokens{"BTC", "USD"};
      LINE_DEBUG;
      for (auto token : tokens) {
         newtoken(tokenissuer, to_maximum_supply(token));
      }

      for (auto token : tokens) {
         for (auto user : users) {
            mint(user, to_wei_asset(200000000, token));
         }
      }

      mint(user2, to_wei_asset(1, "BTC"));
      mint(user2, to_wei_asset(1, "USD"));

      std::vector<std::tuple<uint64_t, std::string, uint64_t>> bind_data{std::make_tuple(10000, "BTC", 5),
                                                                         std::make_tuple(10000, "USD", 5)};
      LINE_DEBUG;
      for (auto d : bind_data) {
         bind(newcontroller, pool_name, to_asset(std::get<0>(d), std::get<1>(d)), to_wei(std::get<2>(d)));
      }

      LINE_DEBUG;
      finalize(newcontroller, pool_name);
   }

void beforejoinpooleqbind() {
      name pool_name = N(dai2mkr11111);
      LINE_DEBUG;
      newpool(admin, pool_name);
      setcontroler(admin, pool_name, newcontroller);

      LINE_DEBUG;
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(newcontroller, pool_name, 1000000);

      std::vector<name>        users{admin, newcontroller, nonadmin, user1};
      std::vector<std::string> tokens{"BTC", "USD"};
      LINE_DEBUG;
      for (auto token : tokens) {
         newtoken(tokenissuer, to_maximum_supply(token));
      }

      for (auto token : tokens) {
         for (auto user : users) {
            mint(user, to_wei_asset(90000, token));
         }
      }

      mint(user2, to_wei_asset(1, "BTC"));
      mint(user2, to_wei_asset(1, "USD"));

      std::vector<std::tuple<uint64_t, std::string, uint64_t>> bind_data{std::make_tuple(100, "BTC", 5),
                                                                         std::make_tuple(200, "USD", 5)};
      LINE_DEBUG;
      for (auto d : bind_data) {
         bind(newcontroller, pool_name, to_wei_asset(std::get<0>(d), std::get<1>(d)), to_wei(std::get<2>(d)));
      }

      LINE_DEBUG;
      finalize(newcontroller, pool_name);

    //   LINE_DEBUG;
    //   std::vector<uint256m> v{uint256m(-1), uint256m(-1), uint256m(-1)};
    //   joinpool(user1, pool_name, to_weight(5), v);
   }


   void initparam() {
      // "8,BTC"  0,30000,420,0,0,0
      // "9,ETH"  0,30000,160000,0,0,0
      // "6,USD"  0,30000,100000,0,0,0
      // "6,GBP"  0,30000,70000,0,0,0
      // "6,HKD"  0,30000,780000,0,0,0

      std::map<std::string, std::vector<int64_t>> params = {
          std::pair<std::string, std::vector<int64_t>>("8,BTC", {0, 30000, 420, 0, 0, 0}),
          std::pair<std::string, std::vector<int64_t>>("9,ETH", {0, 30000, 160000, 0, 0, 0}),
          std::pair<std::string, std::vector<int64_t>>("6,USD", {0, 30000, 100000, 0, 0, 0}),
          std::pair<std::string, std::vector<int64_t>>("6,GBP", {0, 30000, 70000, 0, 0, 0}),
          std::pair<std::string, std::vector<int64_t>>("6,HKD", {0, 30000, 780000, 0, 0, 0})};

      LINE_DEBUG;
      for (auto p : params) {
         setparameter(p.first, p.second);
      }
   }

   /////////////extended token///////////////////////
   /////////////extended token////////////////////////
   ////////////extended token///////////////////////

   std::vector<std::string> parse_string(const std::string& source, const std::string& delimiter = " ") {
      std::vector<std::string> results;
      // const std::string delimiter = ",";
      size_t prev = 0;
      size_t next = 0;

      while ((next = source.find_first_of(delimiter.c_str(), prev)) != std::string::npos) {
         if (next - prev != 0) {
            results.push_back(source.substr(prev, next - prev));
         }
         prev = next + 1;
      }

      if (prev < source.size()) {
         results.push_back(source.substr(prev));
      }

      return results;
   }

   int64_t to_int(const std::string& str) {
      bool        isOK   = false;
      const char* nptr   = str.c_str();
      char*       endptr = NULL;
      errno              = 0;
      int64_t val        = std::strtoll(nptr, &endptr, 10);
      // error ocur
      if ((errno == ERANGE && (val == ULLONG_MAX)) || (errno != 0 && val == 0)) {

      }
      // no digit find
      else if (endptr == nptr) {

      } else if (*endptr != '\0') {
         // printf("Further characters after number: %s\n", endptr);
      } else {
         isOK = true;
      }

      return val;
   }

   extended_asset ext_asset_from_string(const std::string& tokenstr) {
      std::vector<std::string> strvec = parse_string(tokenstr);
      const int                len    = strvec.size();
      BOOST_REQUIRE_EQUAL(len, 2);
      std::string            str     = strvec[0];
      std::string            sym     = strvec[1];
      uint8_t                decmils = 0;
      std::string::size_type pos     = str.find(".");
      if (pos != std::string::npos) {
         decmils = str.size() - pos - 1;
      }

      str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
      int64_t value = static_cast<int64_t>(to_int(str));
      return extended_asset{asset{value, symbol{decmils, sym.c_str()}}, N(extendxtoken)};
   }

   extended_symbol ext_sym_from_string(const std::string& tokenstr) {

      std::vector<std::string> strvec = parse_string(tokenstr, ",");
      const int                len    = strvec.size();
      BOOST_REQUIRE_EQUAL(len, 2);
      std::string str     = strvec[0];
      std::string sym     = strvec[1];
      uint8_t     decmils = static_cast<uint8_t>(to_int(str));

      return extended_symbol{symbol{decmils, sym.c_str()}, N(extendxtoken)};
   }

   symbol sym_from_string(const std::string& tokenstr) { return ext_sym_from_string(tokenstr).sym; }

   std::string to_symbol_code_string(const std::string& tokenstr) {
      std::vector<std::string> strvec = parse_string(tokenstr);
      const int                len    = strvec.size();
      BOOST_REQUIRE_EQUAL(len, 2);
      std::string            str     = strvec[0];
      std::string            sym     = strvec[1];
      uint8_t                decmils = 0;
      std::string::size_type pos     = str.find(".");
      if (pos != std::string::npos) {
         decmils = str.size() - pos - 1;
      }
      return std::to_string(decmils) + "," + sym;

      //   str.erase(std::remove(str.begin(), str.end(), '.'), str.end());
      //   int64_t value = static_cast<int64_t>(to_int(str));
      //   return extended_asset{asset{value, symbol{decmils, sym.c_str()}}, name{"extendxtoken"}};
   }

   //    action_result push_action(const account_name& signer, const action_name& name, const variant_object& data) {
   //       string action_type_name = abi_ser.get_action_type(name);

   //       action act;
   //       act.account = N(extendxtoken);
   //       act.name    = name;
   //       act.data    = abi_ser.variant_to_binary(action_type_name, data, abi_serializer_max_time);

   //       return base_tester::push_action(std::move(act), signer.to_uint64_t());
   //    }

   fc::variant get_stats(const string& symbolname, name contract_name = N(extendxtoken)) {
      auto         symb        = eosio::chain::symbol::from_string(symbolname);
      auto         symbol_code = symb.to_symbol_code().value;
      vector<char> data        = get_row_by_account(N(eoswapeoswap), contract_name, N(stat), account_name(symbol_code));
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("currency_stats", data, abi_serializer_max_time);
   }

   std::string get_account(account_name acc, const string& symbolname, name contract_name = N(extendxtoken)) {
      //   auto         symb        = eosio::chain::symbol::from_string(symbolname);
      //   auto         symbol_code = symb.to_symbol_code().value;
      //   vector<char> data        = get_row_by_account(N(eoswapeoswap), acc, N(accounts),
      //   account_name(symbol_code)); return data.empty() ? fc::variant() : abi_ser.binary_to_variant("account",
      //   data, abi_serializer_max_time);
      auto a = get_accountx(acc, symbolname);
      return (a.is_null() ? "" : a.as_string());
   }

   fc::variant get_accountx(account_name acc, const string& symbolname) {
      vector<char> data;
      const auto&  db  = control->db();
      namespace chain  = eosio::chain;
      const auto* t_id = db.find<eosio::chain::table_id_object, chain::by_code_scope_table>(
          boost::make_tuple(N(eoswapeoswap), acc, N(accounts)));
      if (!t_id) {
         LINE_DEBUG;
         return fc::variant();
      }

      const auto& idx = db.get_index<chain::key_value_index, chain::by_scope_primary>();

      auto itr = idx.lower_bound(boost::make_tuple(t_id->id, 0));
      if (itr == idx.end() || itr->t_id != t_id->id || 0 != itr->primary_key) {
         LINE_DEBUG;

         return fc::variant();
      }

      for (; itr != idx.end(); ++itr) {
         data.resize(itr->value.size());
         memcpy(data.data(), itr->value.data(), data.size());
         LINE_DEBUG;

         if (!data.empty()) {
            LINE_DEBUG;

            auto d = abi_ser.binary_to_variant("account", data, abi_serializer_max_time);
            // BOOST_TEST_CHECK(nullptr == d);
            std::string str = to_symbol_code_string(d["balance"]["quantity"].as_string());
            if (str == symbolname) {
               LINE_DEBUG;

               return d["balance"]["quantity"];
            }
         }
      }
      LINE_DEBUG;

      return fc::variant();
   }

   action_result create(account_name issuer, extended_asset maximum_supply) {

      return push_action(N(eoswapeoswap), N(create), mvo()("issuer", issuer)("maximum_supply", maximum_supply));
   }

   action_result issue(account_name issuer, extended_asset quantity, string memo) {
      return push_action(issuer, N(issue), mvo()("to", issuer)("quantity", quantity)("memo", memo));
   }

   action_result retire(account_name issuer, extended_asset quantity, string memo) {
      return push_action(issuer, N(retire), mvo()("quantity", quantity)("memo", memo));
   }

   action_result transfer(account_name from, account_name to, extended_asset quantity, string memo) {
      return push_action(from, N(transfer), mvo()("from", from)("to", to)("quantity", quantity)("memo", memo));
   }

   action_result open(account_name owner, const extended_symbol& symbol, account_name ram_payer) {
      return push_action(ram_payer, N(open), mvo()("owner", owner)("symbol", symbol)("ram_payer", ram_payer));
   }

   action_result close(account_name owner, const extended_symbol& symbol) {
      return push_action(owner, N(close), mvo()("owner", owner)("symbol", symbol));
   }

   void check_pools(name pool_name) {
#ifdef EOSWAP_DEBUG
      auto pool = get_pool_table(pool_name);
      BOOST_TEST_CHECK(nullptr == pool);
#endif
   }
   void check_balances(name pool_name, name acc, const std::vector<std::vector<std::string>>& expected_token_balances) {
#ifdef EOSWAP_DEBUG
      BOOST_TEST_CHECK(acc == N(acc));
      check_pools(pool_name);
      for (auto token_name : expected_token_balances) {
         auto r = records(pool_name, to_sym(token_name[0]));
         BOOST_TEST_CHECK(token_name[1] == r["balance"].as_string());

         const auto ab = get_balancex(acc, to_sym(token_name[0]).sym);
         BOOST_TEST_CHECK(eosio::chain::asset::from_string(token_name[2] + " " + token_name[0]) == ab);
      }
#endif
   }

   /////////////extended token////////////////////
   bool is_auth_token;

   name           tokenissuer;
   name           newcontroller;
   name           newblabs;
   name           admin;
   name           nonadmin;
   name           user1;
   name           user2;
   abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eoswap_tests)
////////////////factory////////////////////
BOOST_FIXTURE_TEST_CASE(newpool_tests, eoswap_tester) try { newpool(admin, N(dai2mkr11111)); }
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(setblabs_tests, eoswap_tester) try {
   setblabs(N(eoswapeoswap), N(alice));
   std::string b = get_factory_store()["blabs"].as_string();
   BOOST_REQUIRE_EQUAL("alice", b);
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////

BOOST_FIXTURE_TEST_CASE(bind_tests, eoswap_tester) try {
   newpoolBefore();
   mintBefore1();
   bind(newcontroller, N(dai2mkr11111), to_wei_asset(5, "BTC"), to_weight(5));
   bind(newcontroller, N(dai2mkr11111), to_wei_asset(200, "USD"), to_weight(5));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(unbind_tests, eoswap_tester) try {
   newpoolBefore();
   mintBefore1();
 bind(newcontroller, N(dai2mkr11111), to_wei_asset(5, "BTC"), to_weight(5));
   bind(newcontroller, N(dai2mkr11111), to_wei_asset(200, "USD"), to_weight(5));
//  bind(newcontroller, N(dai2mkr11111), to_wei_asset(200, "MKR"), to_weight(5));
   unbind(newcontroller, N(dai2mkr11111), to_sym("BTC"));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(bind_decimal_tests, eoswap_tester) try {
   newpoolBefore();
   mintBefore1();
   std::string sym = "BTC";
   bind(newcontroller, N(dai2mkr11111), to_wei_asset(5, sym), to_weight(5));
   auto with_dec_one = extended_asset{asset{500000000, symbol{static_cast<uint8_t>(get_decimal(sym) + 1), sym.c_str()}},
                                      name{"roxe.ro"}};
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("symbol precision mismatch"), bind(newcontroller, N(dai2mkr11111), with_dec_one, to_weight(5)));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(finalize_tests, eoswap_tester) try {
   newpoolBefore();
   mintBefore1();
   bindBefore1();

   finalize(newcontroller, N(dai2mkr11111));
   bool flag = pools(N(dai2mkr11111))["finalized"].as<bool>();
   BOOST_REQUIRE_EQUAL(true, flag);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(joinpool_tests, eoswap_tester) try {

   newpoolBefore();
   mintBefore1();
   bindBefore1();
   finalizeBefore();
   name                                  pool_name = N(dai2mkr11111);
   std::vector<std::vector<std::string>> expected_token_balances{std::vector<std::string>{"BTC", "10000", "1000"},
                                                                 std::vector<std::string>{"USD", "10000", "10000"}};
   check_balances(pool_name, nonadmin, expected_token_balances);

   std::vector<uint256m> v{uint256m(-1), uint256m(-1)};
   joinpool(nonadmin, pool_name, to_wei(10), v);

   check_balances(pool_name, nonadmin, expected_token_balances);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(joinpool1_tests, eoswap_tester) try {

   before3();
   name                                  pool_name = N(dai2mkr11111);
   std::vector<std::vector<std::string>> expected_token_balances{std::vector<std::string>{"BTC", "10000", "1000"},
                                                                 std::vector<std::string>{"USD", "10000", "10000"}};
   check_balances(pool_name, nonadmin, expected_token_balances);

   std::vector<uint256m> v{uint256m(-1), uint256m(-1)};
   joinpool(nonadmin, pool_name, 100000001, v);

   check_balances(pool_name, nonadmin, expected_token_balances);

   // -191170019
   // -191170019117
   // 191.170019
   // 191170019000
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(joinpool4maxsupply_tests, eoswap_tester) try {

   before4maxsupply();

   name pool_name = N(dai2mkr11111);

   LINE_DEBUG;
   std::vector<uint256m> v{uint256m(-1), uint256m(-1), uint256m(-1)};
   joinpool(user1, pool_name, to_wei(100000), v);

   joinpool(user1, pool_name, static_cast<int64_t>(pow(10, 5)), v);

   //    std::vector<std::vector<std::string>> expected_token_balances{std::vector<std::string>{"BTC", "10000", "1000"},
   //                                                                  std::vector<std::string>{"USD", "10000",
   //                                                                  "10000"}};
   //    check_balances(pool_name, nonadmin, expected_token_balances);

   //    std::vector<uint256m> v{uint256m(-1), uint256m(-1)};
   //    joinpool(nonadmin, pool_name, 100000001, v);

   //    check_balances(pool_name, nonadmin, expected_token_balances);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(joinpooleq0_tests, eoswap_tester) try {

   before4maxsupply();

   name pool_name = N(dai2mkr11111);

   LINE_DEBUG;
   std::vector<uint256m> v{uint256m(-1), uint256m(-1), uint256m(-1)};
   joinpool(user1, pool_name, to_wei(0), v);

//    joinpool(user1, pool_name, static_cast<int64_t>(pow(10, 5)), v);

}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(joinpooleqbind_tests, eoswap_tester) try {

   beforejoinpooleqbind();
   name pool_name = N(dai2mkr11111);
   std::vector<std::vector<std::string>> expected_token_balances{std::vector<std::string>{"BTC", "10000", "1000"},
                                                                 std::vector<std::string>{"USD", "10000", "10000"}};

    check_balances(pool_name, nonadmin, expected_token_balances);
   LINE_DEBUG;
   std::vector<uint256m> v{uint256m(-1), uint256m(-1), uint256m(-1)};
   joinpool(user1, pool_name, 10000, v);
 check_balances(pool_name, nonadmin, expected_token_balances);
//    joinpool(user1, pool_name, static_cast<int64_t>(pow(10, 5)), v);

}
FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE(exitpool_tests, eoswap_tester) try {
   newpoolBefore();
   mintBefore1();
   bindBefore1();
   finalizeBefore();
   joinpoolBefore1();
   name                                  pool_name = N(dai2mkr11111);
   std::vector<std::vector<std::string>> expected_token_balances{std::vector<std::string>{"BTC", "10000", "1000"},
                                                                 std::vector<std::string>{"USD", "10000", "10000"}};
   check_balances(pool_name, nonadmin, expected_token_balances);

   exitpool(nonadmin, pool_name, to_wei(10), std::vector<uint256m>{0, 0});

   check_balances(pool_name, nonadmin, expected_token_balances);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(collect_tests, eoswap_tester) try {
   before1();
   collect(admin, N(dai2mkr11111));
   const auto ab = get_account(newcontroller, std::to_string(ONE_DECIMALS) + "," + default_lp_symbol, N(dai2mkr11111));
   BOOST_REQUIRE_EQUAL("100.000000000 " + default_lp_symbol, ab);

   //  std::string token_name = "EPT";
   //    const auto ab = get_balancex(admin, sym_from_string("4," + token_name), N(dai2mkr11111));
   //    BOOST_REQUIRE_EQUAL(eosio::chain::asset::from_string("100.000000 " + token_name), ab);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountIn_tests, eoswap_tester) try {
   before2();
   name                                  pool_name = N(dai2mkr11111);
   std::vector<std::vector<std::string>> expected_token_balances{std::vector<std::string>{"BTC", "10000", "1000"},
                                                                 std::vector<std::string>{"USD", "10000", "10000"}};
   check_balances(pool_name, user1, expected_token_balances);

   LINE_DEBUG;
   swapamtin(user1, pool_name, to_asset(210000, "BTC"), to_asset(100, "USD"), to_wei(50000000));

   check_balances(pool_name, user1, expected_token_balances);

   LINE_DEBUG;
   swapamtin(user1, pool_name, to_asset(2100, "BTC"), to_asset(10, "USD"), to_wei(50000000));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountOut_tests, eoswap_tester) try {
   before2();
   name pool_name = N(dai2mkr11111);
   auto pool      = get_pool_table(pool_name);

   LINE_DEBUG;
   swapamtout(user1, pool_name, to_asset(43008800502, "USD"), to_asset(21000, "BTC"), to_wei(50000000));
   pool = get_pool_table(pool_name);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountOut1_tests, eoswap_tester) try {
   before2();
   name pool_name = N(dai2mkr11111);
   auto pool      = get_pool_table(pool_name);

   LINE_DEBUG;
   swapamtout(user1, pool_name, to_asset(43008800502, "USD"), to_asset(2100000, "BTC"), to_wei(50000000));

   pool = get_pool_table(pool_name);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountOut_balance_not_enough_tests, eoswap_tester) try {
   before2();
   name                                  pool_name = N(dai2mkr11111);
   std::vector<std::vector<std::string>> expected_token_balances{std::vector<std::string>{"BTC", "10000", "1000"},
                                                                 std::vector<std::string>{"USD", "10000", "10000"}};
   check_balances(pool_name, user1, expected_token_balances);
   LINE_DEBUG;

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("overdrawn balance"),
       swapamtout(user2, pool_name, to_wei_asset(4, "USD"), to_wei_asset(1, "BTC"), to_wei(50000000)));

   check_balances(pool_name, user1, expected_token_balances);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(transfer_fee_tests, eoswap_tester) try {
   before2();
   initparam();
   name pool_name = N(dai2mkr11111);
   auto pool      = get_pool_table(pool_name);

   LINE_DEBUG;
   swapamtout(user1, pool_name, to_asset(430088, "USD"), to_asset(2100000, "BTC"), to_wei(50000000));

   LINE_DEBUG;
   swapamtin(user1, pool_name, to_asset(2100, "BTC"), to_asset(10, "USD"), to_wei(50000000));

   pool = get_pool_table(pool_name);
}
FC_LOG_AND_RETHROW()

////////////////token////////////////////
BOOST_FIXTURE_TEST_CASE(mint_tests, eoswap_tester) try {
   newpool(admin, N(dai2mkr11111));
   std::string token_name = "BTC";
   newtoken(tokenissuer, to_maximum_supply(token_name));
   newtoken(tokenissuer, to_maximum_supply("USD"));
   mint(N(alice), to_wei_asset(5, token_name));
   mint(N(alice), to_wei_asset(200, "USD"));

   const auto ab = get_balancex(N(alice), to_sym(token_name).sym);
   BOOST_REQUIRE_EQUAL(eosio::chain::asset::from_string("5.00000000 " + token_name), ab);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(burn_tests, eoswap_tester) try {
   LINE_DEBUG;
   std::string token_name = "POOL";
   newtokenex(tokenissuer, to_maximum_supply(token_name));
   auto stats = get_stats(get_decimal_str(token_name), N(roxe.ro));
   REQUIRE_MATCHING_OBJECT(
       stats, mvo()("supply", "0.000000000 POOL")("max_supply", "1000000.000000000 POOL")("issuer", tokenissuer));

   LINE_DEBUG;
   mintex(tokenissuer, to_asset(300, token_name));
   auto alice_balance = get_account(tokenissuer, get_decimal_str(token_name), N(roxe.ro));
   BOOST_REQUIRE_EQUAL(alice_balance, "0.000000300 POOL");
   LINE_DEBUG;
   burnex(tokenissuer, to_asset(300, token_name));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swap_transfer_tests, eoswap_tester) try {
   //    push_permission_update_auth_action(admin);
   LINE_DEBUG;
   std::string token_name = "POOL";
   newtokenex(tokenissuer, to_maximum_supply(token_name));
   auto stats = get_stats(get_decimal_str(token_name), N(roxe.ro));
   REQUIRE_MATCHING_OBJECT(
       stats, mvo()("supply", "0.000000000 POOL")("max_supply", "1000000.000000000 POOL")("issuer", tokenissuer));

   LINE_DEBUG;
   mintex(N(alice), to_asset(300, token_name));
   auto alice_balance = get_account(N(alice), get_decimal_str(token_name), N(roxe.ro));
   BOOST_REQUIRE_EQUAL(alice_balance, "0.000000300 POOL");

   LINE_DEBUG;
   transferex(N(alice), N(bob), to_asset(300, token_name));
   auto bob_balance = get_account(N(bob), get_decimal_str(token_name), N(roxe.ro));
   BOOST_REQUIRE_EQUAL(bob_balance, "0.000000300 POOL");

   LINE_DEBUG;
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(extransfer_tests, eoswap_tester) try {
   const std::string     token_name = "POOL";
   const extended_asset& max_supply = to_asset(1, token_name);
   newtoken(tokenissuer, to_maximum_supply(token_name));
   mint(N(alice1111111), max_supply);
   const symbol& sym = max_supply.quantity.get_symbol();
   extransfer(N(alice1111111), user2, max_supply);

   BOOST_REQUIRE_EQUAL(eosio::chain::asset::from_string("0.000000001 " + token_name), get_balancex(user2, sym));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(setparameter_tests, eoswap_tester) try { initparam(); }
FC_LOG_AND_RETHROW()

/// in order to move data from old table to new table
// BOOST_FIXTURE_TEST_CASE(cppool2table_tests, eoswap_tester) try {
//    before();
//    auto pool = get_pool_table(N(dai2mkr11111));
//    //    BOOST_TEST_CHECK(nullptr == pool);
//    cppool2table(admin, N(dai2mkr11111));
//    pool = get_pool_table(N(dai2mkr11111));
//    //    BOOST_TEST_CHECK(nullptr == pool);
// }
// FC_LOG_AND_RETHROW()

// #define EXTENED_TOKEN_TEST
#ifdef EXTENED_TOKEN_TEST
//////////////extened token///////////////////////////////
//////////////extened token///////////////////////////////
//////////////extened token///////////////////////////////
//////////////extened token///////////////////////////////
//////////////extened token///////////////////////////////

BOOST_FIXTURE_TEST_CASE(create_tests, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("1000.000 TKN"));
   auto stats = get_stats("3,TKN");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "0.000 TKN")("max_supply", "1000.000 TKN")("issuer", "alice"));
   produce_blocks(1);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(create_negative_max_supply, eoswap_tester) try {

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("max-supply must be positive"), create(N(alice), ext_asset_from_string("-1000.000 TKN")));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(symbol_already_exists, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("100 TKN"));
   auto stats = get_stats("0,TKN");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "0 TKN")("max_supply", "100 TKN")("issuer", "alice"));
   produce_blocks(1);

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("token with symbol already exists"), create(N(alice), ext_asset_from_string("100 TKN")));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(create_max_supply, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("4611686018427387903 TKN"));
   auto stats = get_stats("0,TKN");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "0 TKN")("max_supply", "4611686018427387903 TKN")("issuer", "alice"));
   produce_blocks(1);

   extended_asset max =
       extended_asset{asset{10, symbol(SY(0, NKT))}, name{"eoswapeoswap"}}; // max(10, symbol(SY(0, NKT)));
   share_type amount = 4611686018427387904;
   static_assert(sizeof(share_type) <= sizeof(asset), "asset changed so test is no longer valid");
   static_assert(std::is_trivially_copyable<asset>::value, "asset is not trivially copyable");
   memcpy(&max.quantity, &amount, sizeof(share_type)); // hack in an invalid amount

   BOOST_CHECK_EXCEPTION(create(N(alice), max), asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(create_max_decimals, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("1.0000000000000000 TKN"));
   auto stats = get_stats("18,TKN");
   REQUIRE_MATCHING_OBJECT(
       stats, mvo()("supply", "0.0000000000000000 TKN")("max_supply", "1.0000000000000000 TKN")("issuer", "alice"));
   produce_blocks(1);

   // extended_asset{asset{value, symbol{4, sym.c_str()}}, name{"eoswapeoswap"}}
   extended_asset max = extended_asset{asset{10, symbol(SY(0, NKT))}, name{"eoswapeoswap"}};
   // 1.0 000 000 000 000 000 000 => 0x8ac7230489e80000L
   share_type amount = 0x8ac7230489e80000L;
   static_assert(sizeof(share_type) <= sizeof(asset), "asset changed so test is no longer valid");
   static_assert(std::is_trivially_copyable<asset>::value, "asset is not trivially copyable");
   memcpy(&max.quantity, &amount, sizeof(share_type)); // hack in an invalid amount

   BOOST_CHECK_EXCEPTION(create(N(alice), max), asset_type_exception, [](const asset_type_exception& e) {
      return expect_assert_message(e, "magnitude of asset amount must be less than 2^62");
   });
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(issue_tests, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("1000.000 TKN"));
   produce_blocks(1);

   LINE_DEBUG;
   issue(N(alice), ext_asset_from_string("500.000 TKN"), "hola");
   LINE_DEBUG;
   auto stats = get_stats("3,TKN");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "500.000 TKN")("max_supply", "1000.000 TKN")("issuer", "alice"));
   LINE_DEBUG;
   auto alice_balance = get_account(N(alice), "3,TKN");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("quantity", "500.000 TKN"));
   BOOST_REQUIRE_EQUAL(alice_balance, "500.000 TKN");
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("quantity exceeds available supply"),
       issue(N(alice), ext_asset_from_string("500.001 TKN"), "hola"));
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("must issue positive quantity"), issue(N(alice), ext_asset_from_string("-1.000 TKN"), "hola"));
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(success(), issue(N(alice), ext_asset_from_string("1.000 TKN"), "hola"));
   LINE_DEBUG;
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(retire_tests, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("1000.000 TKN"));
   produce_blocks(1);
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(success(), issue(N(alice), ext_asset_from_string("500.000 TKN"), "hola"));
   LINE_DEBUG;
   auto stats = get_stats("3,TKN");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "500.000 TKN")("max_supply", "1000.000 TKN")("issuer", "alice"));
   LINE_DEBUG;
   auto alice_balance = get_account(N(alice), "3,TKN");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "500.000 TKN"));
   BOOST_REQUIRE_EQUAL(alice_balance, "500.000 TKN");
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(success(), retire(N(alice), ext_asset_from_string("200.000 TKN"), "hola"));
   LINE_DEBUG;
   stats = get_stats("3,TKN");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "300.000 TKN")("max_supply", "1000.000 TKN")("issuer", "alice"));
   LINE_DEBUG;
   alice_balance = get_account(N(alice), "3,TKN");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "300.000 TKN"));
   BOOST_REQUIRE_EQUAL(alice_balance, "300.000 TKN");
   LINE_DEBUG;
   // should fail to retire more than current supply
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("overdrawn balance"), retire(N(alice), ext_asset_from_string("500.000 TKN"), "hola"));
   LINE_DEBUG;

   BOOST_REQUIRE_EQUAL(success(), transfer(N(alice), N(bob), ext_asset_from_string("200.000 TKN"), "hola"));
   LINE_DEBUG;
   // should fail to retire since tokens are not on the issuer's balance
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("overdrawn balance"), retire(N(alice), ext_asset_from_string("300.000 TKN"), "hola"));
   LINE_DEBUG;
   // transfer tokens back
   BOOST_REQUIRE_EQUAL(success(), transfer(N(bob), N(alice), ext_asset_from_string("200.000 TKN"), "hola"));
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(success(), retire(N(alice), ext_asset_from_string("300.000 TKN"), "hola"));
   LINE_DEBUG;
   stats = get_stats("3,TKN");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "0.000 TKN")("max_supply", "1000.000 TKN")("issuer", "alice"));
   LINE_DEBUG;
   alice_balance = get_account(N(alice), "3,TKN");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "0.000 TKN"));
   BOOST_REQUIRE_EQUAL(alice_balance, "0.000 TKN");
   LINE_DEBUG;
   // trying to retire tokens with zero supply
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("overdrawn balance"), retire(N(alice), ext_asset_from_string("1.000 TKN"), "hola"));
   LINE_DEBUG;
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(transfer_tests, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("1000 CERO"));
   produce_blocks(1);
   LINE_DEBUG;
   issue(N(alice), ext_asset_from_string("1000 CERO"), "hola");
   LINE_DEBUG;
   auto stats = get_stats("0,CERO");
   REQUIRE_MATCHING_OBJECT(stats, mvo()("supply", "1000 CERO")("max_supply", "1000 CERO")("issuer", "alice"));
   LINE_DEBUG;
   auto alice_balance = get_account(N(alice), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "1000 CERO"));
   BOOST_REQUIRE_EQUAL(alice_balance, "1000 CERO");
   LINE_DEBUG;
   transfer(N(alice), N(bob), ext_asset_from_string("300 CERO"), "hola");
   LINE_DEBUG;
   alice_balance = get_account(N(alice), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "700 CERO")("frozen", 0)("whitelist", 1));
   BOOST_REQUIRE_EQUAL(alice_balance, "700 CERO");
   LINE_DEBUG;
   auto bob_balance = get_account(N(bob), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(bob_balance, mvo()("balance", "300 CERO")("frozen", 0)("whitelist", 1));
   BOOST_REQUIRE_EQUAL(bob_balance, "300 CERO");
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("overdrawn balance"), transfer(N(alice), N(bob), ext_asset_from_string("701 CERO"), "hola"));
   LINE_DEBUG;
   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("must transfer positive quantity"),
       transfer(N(alice), N(bob), ext_asset_from_string("-1000 CERO"), "hola"));
   LINE_DEBUG;
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(open_tests, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("1000 CERO"));

   auto alice_balance = get_account(N(alice), "0,CERO");
   BOOST_REQUIRE_EQUAL("", alice_balance);
   //    BOOST_REQUIRE_EQUAL(
   //        wasm_assert_msg("tokens can only be issued to issuer account"),
   //        push_action(N(alice), N(issue), mvo()("to", "bob")("quantity", ext_asset_from_string("1000
   //        CERO"))("memo",
   //        "")));
   BOOST_REQUIRE_EQUAL(success(), issue(N(alice), ext_asset_from_string("1000 CERO"), "issue"));

   alice_balance = get_account(N(alice), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "1000 CERO"));
   BOOST_REQUIRE_EQUAL(alice_balance, "1000 CERO");

   auto bob_balance = get_account(N(bob), "0,CERO");
   BOOST_REQUIRE_EQUAL("", bob_balance);

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("owner account does not exist"), open(N(nonexistent), ext_sym_from_string("0,CERO"), N(alice)));
   BOOST_REQUIRE_EQUAL(success(), open(N(bob), ext_sym_from_string("0,CERO"), N(alice)));

   bob_balance = get_account(N(bob), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(bob_balance, mvo()("balance", "0 CERO"));
   BOOST_REQUIRE_EQUAL(bob_balance, "0 CERO");

   BOOST_REQUIRE_EQUAL(success(), transfer(N(alice), N(bob), ext_asset_from_string("200 CERO"), "hola"));

   bob_balance = get_account(N(bob), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(bob_balance, mvo()("balance", "200 CERO"));
   BOOST_REQUIRE_EQUAL(bob_balance, "200 CERO");

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("symbol does not exist"), open(N(carol), ext_sym_from_string("0,INVALID"), N(alice)));

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("symbol precision mismatch"), open(N(carol), ext_sym_from_string("1,CERO"), N(alice)));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(close_tests, eoswap_tester) try {

   auto token = create(N(alice), ext_asset_from_string("1000 CERO"));

   auto alice_balance = get_account(N(alice), "0,CERO");
   BOOST_REQUIRE_EQUAL("", alice_balance);

   BOOST_REQUIRE_EQUAL(success(), issue(N(alice), ext_asset_from_string("1000 CERO"), "hola"));

   alice_balance = get_account(N(alice), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "1000 CERO"));
   BOOST_REQUIRE_EQUAL(alice_balance, "1000 CERO");

   BOOST_REQUIRE_EQUAL(success(), transfer(N(alice), N(bob), ext_asset_from_string("1000 CERO"), "hola"));

   alice_balance = get_account(N(alice), "0,CERO");
   //    REQUIRE_MATCHING_OBJECT(alice_balance, mvo()("balance", "0 CERO"));
   BOOST_REQUIRE_EQUAL(alice_balance, "0 CERO");

   BOOST_REQUIRE_EQUAL(success(), close(N(alice), ext_sym_from_string("0,CERO")));
   alice_balance = get_account(N(alice), "0,CERO");
   BOOST_REQUIRE_EQUAL("", alice_balance);
}
FC_LOG_AND_RETHROW()
#endif

BOOST_AUTO_TEST_SUITE_END()
