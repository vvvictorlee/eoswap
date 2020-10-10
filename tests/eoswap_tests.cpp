#include "eosio.system_tester.hpp"
#include <boost/test/unit_test.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/testing/tester.hpp>

#include "Runtime/Runtime.h"

#include <fc/variant_object.hpp>

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo      = fc::mutable_variant_object;
using uint_eth = uint64_t;

class findx {
 public:
   findx(const string str) { test = str; }
   bool operator()(const fc::variant& v) {
      if (v["key"].as_string().compare(test) == 0)
         return true;
      else
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

      create_accounts({N(alice), N(bob), N(carol), N(eoswapeoswap), N(pool)});
      create_accounts({N(weth), N(dai), N(mkr), N(xxx), N(eoswap.token)});
      produce_blocks(2);
      admin    = N(eoswapeoswap);
      nonadmin = N(alice);
      user1    = N(bob);
      user2    = N(carol);
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
          N(eosio.token), config::system_account_name, eosio::chain::asset::from_string("10000000000.0000 EOS"));

      issue(config::system_account_name, eosio::chain::asset::from_string("1000000000.0000 EOS"));

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

      set_code(N(eoswap.token), contracts::token_wasm());
      set_abi(N(eoswap.token), contracts::token_abi().data());

      std::vector<string> accounts = {"alice1111111", "bob111111111", "carol1111111", "david1111111"};
      std::vector<string> tokens   = {"WETH", "DAI", "MKR", "XXX"};
      int                 j        = 0;
      for (auto& acc_name : accounts) {
         std::string amount    = "10000000000.0000 " + tokens[j++];
         asset       maxsupply = eosio::chain::asset::from_string(amount.c_str());
         name        acc       = name(acc_name.c_str());

         create_currency(N(eoswap.token), acc, maxsupply);
         issuex(N(eoswap.token), acc, maxsupply, acc);
         produce_blocks(1);
      }

      produce_blocks();

      const auto& accnt = control->db().get<account_object, by_name>(N(eoswapeoswap));
      abi_def     abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
      abi_ser.set_abi(abi, abi_serializer_max_time);
   }

   action_result push_action(const account_name& signer, const action_name& name, const variant_object& data) {
      string action_type_name = abi_ser.get_action_type(name);

      action act;
      act.account = N(eoswapeoswap);
      act.name    = name;
      act.data    = abi_ser.variant_to_binary(action_type_name, data, abi_serializer_max_time);

      return base_tester::push_action(std::move(act), signer.to_uint64_t());
   }

   auto push_permission_update_auth_action(const account_name& signer) {
      auto auth = authority(eosio::testing::base_tester::get_public_key(signer, "active"));
      auth.accounts.push_back(permission_level_weight{{N(eoswapeoswap), config::eosio_code_name}, 1});

      return base_tester::push_action(
          N(eosio), N(updateauth), signer,
          mvo()("account", signer)("permission", "active")("parent", "owner")("auth", auth));
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

   void transferex(
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

   asset get_balancex(const account_name& act, const symbol& sym, name contract = N(eoswap.token)) {
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
   // msg_sender, name dst, uint_eth amt
   action_result setswapfee(account_name msg_sender, account_name pool_name, uint_eth swapFee) {
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

   action_result
   bind(account_name msg_sender, account_name pool_name, account_name token, uint_eth balance, uint_eth denorm) {
      return push_action(
          msg_sender, N(bind),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token)("balance", balance)(
              "denorm", denorm));
   }

   action_result
   rebind(account_name msg_sender, account_name pool_name, account_name token, uint_eth balance, uint_eth denorm) {
      return push_action(
          msg_sender, N(rebind),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token)("balance", balance)(
              "denorm", denorm));
   }

   action_result unbind(account_name msg_sender, account_name pool_name, account_name token) {
      return push_action(
          msg_sender, N(unbind), mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token));
   }

   action_result gulp(account_name msg_sender, account_name pool_name, account_name token) {
      return push_action(msg_sender, N(gulp), mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token));
   }

   action_result joinpool(
       account_name msg_sender, account_name pool_name, uint_eth poolAmountOut, std::vector<uint_eth> maxAmountsIn) {
      return push_action(
          msg_sender, N(joinpool),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("poolAmountOut", poolAmountOut)(
              "maxAmountsIn", maxAmountsIn));
   }

   action_result exitpool(
       account_name msg_sender, account_name pool_name, uint_eth poolAmountIn, std::vector<uint_eth> minAmountsOut) {
      return push_action(
          msg_sender, N(exitpool),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("poolAmountIn", poolAmountIn)(
              "minAmountsOut", minAmountsOut));
   }

   action_result swapamtin(
       name msg_sender, name pool_name, name tokenIn, uint tokenAmountIn, name tokenOut, uint minAmountOut,
       uint maxPrice) {
      return push_action(
          msg_sender, N(swapamtin),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("tokenIn", tokenIn)("tokenAmountIn", tokenAmountIn)(
              "tokenOut", tokenOut)("minAmountOut", minAmountOut)("maxPrice", maxPrice));
   }

   action_result swapamtout(
       name msg_sender, name pool_name, name tokenIn, uint maxAmountIn, name tokenOut, uint tokenAmountOut,
       uint maxPrice) {
      return push_action(
          msg_sender, N(swapamtout),
          mvo()("msg_sender", msg_sender)("pool_name", pool_name)("tokenIn", tokenIn)("maxAmountIn", maxAmountIn)(
              "tokenOut", tokenOut)("tokenAmountOut", tokenAmountOut)("maxPrice", maxPrice));
   }

   ////////////////TOKEN//////////////
   action_result extransfer(name from, name to, const extended_asset& quantity, const std::string& memo = "") {
      return push_action(from, N(extransfer), mvo()("from", from)("to", to)("quantity", quantity)("memo", memo));
   }

   action_result newtoken(account_name msg_sender, account_name token) {
      return push_action(msg_sender, N(newtoken), mvo()("msg_sender", msg_sender)("token", token));
   }

   action_result approve(account_name msg_sender, account_name token, account_name dst, uint_eth amt) {
      return push_action(
          msg_sender, N(approve), mvo()("msg_sender", msg_sender)("token", token)("dst", dst)("amt", amt));
   }

   action_result transfer(account_name msg_sender, account_name token, account_name dst, uint_eth amt) {
      return push_action(
          msg_sender, N(transfer), mvo()("msg_sender", msg_sender)("token", token)("dst", dst)("amt", amt));
   }

   action_result
   transferfrom(account_name msg_sender, account_name token, account_name src, account_name dst, uint_eth amt) {
      return push_action(
          msg_sender, N(transferfrom),
          mvo()("msg_sender", msg_sender)("token", token)("src", src)("dst", dst)("amt", amt));
   }

   action_result incapproval(account_name msg_sender, account_name token, account_name dst, uint_eth amt) {
      return push_action(
          msg_sender, N(incapproval), mvo()("msg_sender", msg_sender)("token", token)("dst", dst)("amt", amt));
   }

   action_result decapproval(account_name msg_sender, account_name token, account_name dst, uint_eth amt) {
      return push_action(
          msg_sender, N(decapproval), mvo()("msg_sender", msg_sender)("token", token)("dst", dst)("amt", amt));
   }

   action_result mint(account_name msg_sender, account_name token, uint_eth amt) {
      return push_action(msg_sender, N(mint), mvo()("msg_sender", msg_sender)("token", token)("amt", amt));
   }

   action_result burn(account_name msg_sender, account_name token, uint_eth amt) {
      return push_action(msg_sender, N(burn), mvo()("msg_sender", msg_sender)("token", token)("amt", amt));
   }

   action_result move(account_name msg_sender, account_name token, name dst, uint_eth amt) {
      return push_action(msg_sender, N(move), mvo()("msg_sender", msg_sender)("token", token)("dst", dst)("amt", amt));
   }

   ////////////////get table//////////////

   fc::variant get_factory_store() {
      vector<char> data = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap), N(factorystore), N(factorystore));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("BFactoryStorage", data, abi_serializer_max_time);
   }

   fc::variant get_pool_store() {
      vector<char> data = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap), N(poolstore), N(poolstore));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("BPoolStorage", data, abi_serializer_max_time);
   }

   fc::variant get_token_store() {
      vector<char> data = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap), N(tokenstore), N(tokenstore));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("BTokenStorage", data, abi_serializer_max_time);
   }

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

   std::string balanceOf(name token, name account) {
      const auto s = get_token_store();
      const auto t = find_variant(s["tokens"], token.to_string());
      const auto b = find_value(t["balance"], account.to_string());
      return b;
   }

   std::string allowance(name token, name src, name dst) {
      const auto s   = get_token_store();
      const auto t   = find_variant(s["tokens"], token.to_string());
      const auto a   = find_variant(t["allowance"], src.to_string());
      const auto amt = find_value(a["dst2amt"], dst.to_string());
      return amt;
   }

   fc::variant records(name pool_name, name token) {
      const auto ps = get_pool_store();
      const auto p  = find_variant(ps["pools"], pool_name.to_string());
      const auto r  = find_value(p["records"], token.to_string());
      return r;
   }

   fc::variant pools(name pool_name) {
      const auto ps = get_pool_store();
      const auto p  = find_variant(ps["pools"], pool_name.to_string());
      return p;
   }

   uint_eth to_wei(uint_eth value) { return value * pow(10, 6); }

   void newpoolBefore() { newpool(admin, N(pool)); }

   void setswapfeeBefore() {
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(admin, N(pool), 3000);
   }

   void mintBefore() {

      uint_eth eamount  = to_wei(5);
      uint_eth damount  = to_wei(200);
      uint_eth neamount = to_wei(1);
      uint_eth ndamount = to_wei(200);
      uint_eth jamount  = to_wei(10);

      newtoken(admin, N(weth));
      newtoken(admin, N(mkr));
      newtoken(admin, N(dai));
      newtoken(admin, N(xxx));

      mint(admin, N(weth), to_wei(50));
      mint(admin, N(mkr), to_wei(20));
      mint(admin, N(dai), to_wei(10000));
      mint(admin, N(xxx), to_wei(10));

      mint(user1, N(weth), to_wei(25));
      mint(user1, N(mkr), to_wei(4));
      mint(user1, N(dai), to_wei(40000));
      mint(user1, N(xxx), to_wei(10));

      mint(user2, N(weth), 12222200);
      mint(user2, N(mkr), 1015333);
      mint(user2, N(dai), 0);
      mint(user2, N(xxx), to_wei(51));

      mint(nonadmin, N(weth), to_wei(1));
      mint(nonadmin, N(dai), to_wei(200));
   }

   void bindBefore() {
      bind(admin, N(pool), N(weth), to_wei(50), to_wei(5));
      bind(admin, N(pool), N(mkr), to_wei(20), to_wei(5));
      bind(admin, N(pool), N(dai), to_wei(10000), to_wei(5));
   }

   void finalizeBefore() { finalize(admin, N(pool)); }

   void joinpoolBefore() {
      std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1), uint_eth(-1)};
      joinpool(user1, N(pool), to_wei(5), v);
   }

   void before() {
      newpoolBefore();
      setswapfeeBefore();
      mintBefore();
      bindBefore();
      finalizeBefore();
      joinpoolBefore();
   }

   name           admin;
   name           nonadmin;
   name           user1;
   name           user2;
   abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eoswap_tests)
////////////////factory////////////////////
BOOST_FIXTURE_TEST_CASE(newpool_tests, eoswap_tester) try { newpool(admin, N(pool)); }
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(setblabs_tests, eoswap_tester) try {
   setblabs(N(eoswapeoswap), N(alice));
   std::string b = get_factory_store()["blabs"].as_string();
   BOOST_REQUIRE_EQUAL("alice", b);
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////

BOOST_FIXTURE_TEST_CASE(bind_tests, eoswap_tester) try {

   uint_eth eamount = to_wei(5);
   uint_eth damount = to_wei(200);
   uint_eth jamount = to_wei(10);
   newpool(admin, N(pool));
   newtoken(admin, N(weth));
   newtoken(admin, N(dai));
   mint(admin, N(weth), eamount);
   mint(admin, N(dai), damount);
   bind(admin, N(pool), N(weth), eamount, to_wei(5));
   bind(admin, N(pool), N(dai), damount, to_wei(5));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(finalize_tests, eoswap_tester) try {

   uint_eth eamount  = to_wei(5);
   uint_eth damount  = to_wei(200);
   uint_eth neamount = to_wei(1);
   uint_eth ndamount = to_wei(200);
   uint_eth jamount  = to_wei(10);
   newpool(admin, N(pool));
   newtoken(admin, N(weth));
   newtoken(admin, N(dai));
   mint(admin, N(weth), eamount);
   mint(admin, N(dai), damount);
   mint(nonadmin, N(weth), neamount);
   mint(nonadmin, N(dai), ndamount);

   bind(admin, N(pool), N(weth), eamount, to_wei(5));
   bind(admin, N(pool), N(dai), damount, to_wei(5));
   finalize(admin, N(pool));
   bool flag = pools(N(pool))["finalized"].as<bool>();
   BOOST_REQUIRE_EQUAL(true, flag);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(joinpool_tests, eoswap_tester) try {

   uint_eth eamount  = to_wei(5);
   uint_eth damount  = to_wei(200);
   uint_eth neamount = to_wei(1);
   uint_eth ndamount = to_wei(200);
   uint_eth jamount  = to_wei(10);
   newpool(admin, N(pool));
   newtoken(admin, N(weth));
   newtoken(admin, N(dai));
   mint(admin, N(weth), eamount);
   mint(admin, N(dai), damount);
   mint(nonadmin, N(weth), neamount);
   mint(nonadmin, N(dai), ndamount);

   bind(admin, N(pool), N(weth), eamount, to_wei(5));
   bind(admin, N(pool), N(dai), damount, to_wei(5));
   finalize(admin, N(pool));
   bool flag = pools(N(pool))["finalized"].as<bool>();
   BOOST_REQUIRE_EQUAL(true, flag);
   std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1)};
   //   const auto t = get_pool_store();
   //   BOOST_TEST_CHECK(nullptr==t);
   joinpool(nonadmin, N(pool), jamount, v);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(exitpool_tests, eoswap_tester) try {

   uint_eth eamount  = to_wei(5);
   uint_eth damount  = to_wei(200);
   uint_eth neamount = to_wei(1);
   uint_eth ndamount = to_wei(200);
   uint_eth jamount  = to_wei(10);
   newpool(admin, N(pool));
   newtoken(admin, N(weth));
   newtoken(admin, N(dai));
   mint(admin, N(weth), eamount);
   mint(admin, N(dai), damount);
   mint(nonadmin, N(weth), neamount);
   mint(nonadmin, N(dai), ndamount);

   bind(admin, N(pool), N(weth), eamount, to_wei(5));
   bind(admin, N(pool), N(dai), damount, to_wei(5));
   finalize(admin, N(pool));
   std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1)};
   const auto            t = get_token_store();
   joinpool(nonadmin, N(pool), jamount, v);
   exitpool(nonadmin, N(pool), jamount, std::vector<uint_eth>{0, 0});
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(collect_tests, eoswap_tester) try {

   uint_eth eamount  = to_wei(5);
   uint_eth damount  = to_wei(200);
   uint_eth neamount = to_wei(1);
   uint_eth ndamount = to_wei(200);
   uint_eth jamount  = to_wei(10);
   newpool(admin, N(pool));
   newtoken(admin, N(weth));
   newtoken(admin, N(dai));
   mint(admin, N(weth), eamount);
   mint(admin, N(dai), damount);
   mint(nonadmin, N(weth), neamount);
   mint(nonadmin, N(dai), ndamount);

   bind(admin, N(pool), N(weth), eamount, to_wei(5));
   bind(admin, N(pool), N(dai), damount, to_wei(5));
   finalize(admin, N(pool));
   std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1)};
   const auto            t = get_token_store();
   joinpool(nonadmin, N(pool), jamount, v);
   exitpool(nonadmin, N(pool), jamount, std::vector<uint_eth>{0, 0});
   collect(admin, N(pool));
   const auto ab = balanceOf(N(pool), admin);

   BOOST_REQUIRE_EQUAL(std::to_string(to_wei(100)), ab);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountIn_tests, eoswap_tester) try {

   before();
   swapamtin(N(bob), N(pool), N(weth), 2500000, N(dai), to_wei(475), to_wei(200));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountOut_tests, eoswap_tester) try {

   before();
   swapamtout(N(bob), N(pool), N(weth), to_wei(3), N(mkr), to_wei(1), to_wei(500));
}
FC_LOG_AND_RETHROW()

////////////////token////////////////////
BOOST_FIXTURE_TEST_CASE(mint_tests, eoswap_tester) try {

   uint_eth eamount = to_wei(5);
   uint_eth damount = to_wei(200);
   uint_eth jamount = to_wei(10);
   newpool(admin, N(pool));
   newtoken(admin, N(weth));
   newtoken(admin, N(dai));
   mint(N(alice), N(weth), eamount);
   mint(N(alice), N(dai), damount);

   const auto ab = balanceOf(N(weth), N(alice));
   BOOST_REQUIRE_EQUAL(std::to_string(eamount), ab);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(burn_tests, eoswap_tester) try {

   newpool(admin, N(pool));
   mint(N(alice), N(pool), 300);
   burn(N(alice), N(pool), 300);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(approve_tests, eoswap_tester) try {

   newpool(admin, N(pool));
   approve(N(alice), N(pool), N(bob), 300);
   const auto b = allowance(N(pool), N(alice), N(bob));

   BOOST_REQUIRE_EQUAL("300", b);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(transfer_tests, eoswap_tester) try {
   newpool(admin, N(pool));
   mint(N(alice), N(pool), 300);
   transfer(N(alice), N(pool), N(bob), 300);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(extransfer_tests, eoswap_tester) try {
   push_permission_update_auth_action(N(alice1111111));
   symbol sym{4, "WETH"};
   extransfer(N(alice1111111), admin, extended_asset{asset{int64_t{1}, sym}, name{"eoswap.token"}});

   BOOST_REQUIRE_EQUAL(
       eosio::chain::asset::from_string("0.0001 WETH"), get_balancex(admin,sym));

}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
