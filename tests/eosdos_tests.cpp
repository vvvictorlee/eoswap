#include "eosio.system_tester.hpp"
#include <boost/test/unit_test.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/testing/tester.hpp>

#include "Runtime/Runtime.h"

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
using uint_eth = uint64_t;
using uint256  = uint64_t;
using namesym  = eosio::chain::uint128_t;
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

class eosdos_tester : public tester {
 public:
   eosdos_tester() {
      produce_blocks(2);

      create_accounts({N(eosio.token), N(eosio.ram), N(eosio.ramfee), N(eosio.stake), N(eosio.bpay), N(eosio.vpay),
                       N(eosio.saving), N(eosio.names), N(eosio.rex)});

      create_accounts({N(alice), N(bob), N(carol), N(eosdoseosdos), N(pool)});
      create_accounts({N(weth), N(dai), N(mkr), N(xxx), N(eosdosxtoken)});
      produce_blocks(2);
      admin    = N(eosdoseosdos);
      nonadmin = N(alice);
      user1    = N(bob);
      user2    = N(carol);
      set_code(N(eosdoseosdos), contracts::dos_wasm());
      set_abi(N(eosdoseosdos), contracts::dos_abi().data());
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

      set_code(N(eosdosxtoken), contracts::token_wasm());
      set_abi(N(eosdosxtoken), contracts::token_abi().data());

      std::vector<string> accounts  = {"alice1111111", "bob111111111", "carol1111111", "david1111111"};
      std::vector<name>   taccounts = {N(alice), N(bob), N(eosdoseosdos)};
      std::vector<string> tokens    = {"WETH", "DAI", "MKR", "XXX"};
      int                 j         = 0;
      std::string         amt       = "100000.0000 ";
      std::string         memo      = "";
      //   for (auto& acc_name : accounts) {
      //      std::string token     = tokens[j++];
      //      std::string amount    = "10000000000.0000 " + token;
      //      std::string tamount   = amt + token;
      //      asset       maxsupply = eosio::chain::asset::from_string(amount.c_str());
      //      name        acc       = name(acc_name.c_str());

      //      create_currency(N(eosdosxtoken), acc, maxsupply);
      //      issuex(N(eosdosxtoken), acc, maxsupply, acc);
      //      produce_blocks(1);
      //      for (auto& to : taccounts) {
      //         transferex(N(eosdosxtoken), acc, to, tamount, acc, memo);
      //      }
      //   }

      is_auth_token = false;
      produce_blocks();

      const auto& accnt = control->db().get<account_object, by_name>(N(eosdoseosdos));
      abi_def     abi;
      BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
      abi_ser.set_abi(abi, abi_serializer_max_time);
   }

   auto push_permission_update_auth_action(const account_name& signer) {
      auto auth = authority(eosio::testing::base_tester::get_public_key(signer, "active"));
      auth.accounts.push_back(permission_level_weight{{N(eosdoseosdos), config::eosio_code_name}, 1});

      return base_tester::push_action(
          N(eosio), N(updateauth), signer,
          mvo()("account", signer)("permission", "active")("parent", "owner")("auth", auth));
   }

   action_result push_action(const account_name& signer, const action_name& name, const variant_object& data) {
      if (!is_auth_token) {
         is_auth_token = true;
         push_permission_update_auth_action(N(eosdosxtoken));
         push_permission_update_auth_action(nonadmin);
         push_permission_update_auth_action(user1);
         push_permission_update_auth_action(N(alice1111111));
      }

      string action_type_name = abi_ser.get_action_type(name);

      action act;
      act.account = N(eosdoseosdos);
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

   asset get_balancex(const account_name& act, const symbol& sym, name contract = N(eosdosxtoken)) {
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

   /////////////zoo///////////
   action_result adddodo(name msg_sender, address _dodo) {
      return push_action(msg_sender, N(adddodo), mvo()("msg_sender", msg_sender)("_dodo", _dodo));
   }

   action_result removedodo(name msg_sender, address _dodo) {
      return push_action(msg_sender, N(removedodo), mvo()("msg_sender", msg_sender)("_dodo", _dodo));
   }

   action_result breeddodo(
       name msg_sender, address maintainer, const extended_symbol& baseToken, const extended_symbol& quoteToken,
       const extended_symbol& oracle, uint256 lpFeeRate, uint256 mtFeeRate, uint256 k, uint256 gasPriceLimit) {
      return push_action(
          msg_sender, N(breeddodo),
          mvo()("msg_sender", msg_sender)("maintainer", maintainer)("baseToken", baseToken)("quoteToken", quoteToken)(
              "oracle",
              oracle)("lpFeeRate", lpFeeRate)("mtFeeRate", mtFeeRate)("k", k)("gasPriceLimit", gasPriceLimit));
   }

   ///////////////proxy ////////////////////
   // msg_sender, name dst, uint_eth amt
   action_result sellethtoken(name msg_sender, const extended_asset& ethToken, const extended_asset& minReceiveToken) {
      return push_action(
          msg_sender, N(sellethtoken),
          mvo()("msg_sender", msg_sender)("ethToken", ethToken)("minReceiveToken", minReceiveToken));
   }

   action_result
   buyeth1token(name msg_sender, const extended_asset& ethToken, const extended_asset& maxPayTokenAmount) {
      return push_action(
          msg_sender, N(buyeth1token),
          mvo()("msg_sender", msg_sender)("ethToken", ethToken)("maxPayTokenAmount", maxPayTokenAmount));
   }

   action_result selltokeneth(name msg_sender, const extended_asset& baseToken, const extended_asset& minReceiveEth) {
      return push_action(
          msg_sender, N(selltokeneth),
          mvo()("msg_sender", msg_sender)("baseToken", baseToken)("minReceiveEth", minReceiveEth));
   }

   action_result buytoken1eth(name msg_sender, const extended_asset& baseToken, const extended_asset& maxPayEthAmount) {
      return push_action(
          msg_sender, N(buytoken1eth),
          mvo()("msg_sender", msg_sender)("baseToken", baseToken)("maxPayEthAmount", maxPayEthAmount));
   }

   action_result
   depositethab(name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      return push_action(
          msg_sender, N(depositethab),
          mvo()("msg_sender", msg_sender)("ethtokenamount", ethtokenamount)("quoteToken", quoteToken));
   }

   action_result withdraweab(name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      return push_action(
          msg_sender, N(withdraweab),
          mvo()("msg_sender", msg_sender)("ethtokenamount", ethtokenamount)("quoteToken", quoteToken));
   }

   action_result withdrawaeab(name msg_sender, const extended_symbol& quoteToken) {
      return push_action(msg_sender, N(withdrawaeab), mvo()("msg_sender", msg_sender)("quoteToken", quoteToken));
   }

   action_result depositethaq(name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      return push_action(
          msg_sender, N(depositethaq),
          mvo()("msg_sender", msg_sender)("ethtokenamount", ethtokenamount)("baseToken", baseToken));
   }

   action_result withdraweaq(name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      return push_action(
          msg_sender, N(withdraweaq),
          mvo()("msg_sender", msg_sender)("ethtokenamount", ethtokenamount)("baseToken", baseToken));
   }

   action_result withdrawaeaq(name msg_sender, const extended_symbol& baseToken) {
      return push_action(msg_sender, N(withdrawaeaq), mvo()("msg_sender", msg_sender)("baseToken", baseToken));
   }

   //////////////////Oracle//////////////
   action_result neworacle(name msg_sender, const extended_symbol& token) {
      return push_action(from, N(neworacle), mvo()("msg_sender", msg_sender)("token", token));
   }

   action_result setprice(name msg_sender, const extended_asset& amt) {
      return push_action(msg_sender, N(setprice), mvo()("msg_sender", msg_sender)("amt", amt));
   }

   //////////////////TOKEN//////////////
   action_result extransfer(name from, name to, const extended_asset& quantity, const std::string& memo = "") {
      return push_action(from, N(extransfer), mvo()("from", from)("to", to)("quantity", quantity)("memo", memo));
   }

   action_result newtoken(account_name msg_sender, const extended_asset& token) {
      return push_action(msg_sender, N(newtoken), mvo()("msg_sender", msg_sender)("token", token));
   }

   ////////////////get table//////////////
   fc::variant get_zoo_store() {
      vector<char> data = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), N(zoo), N(zoo));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("ZooStorage", data, abi_serializer_max_time);
   }

   fc::variant get_proxy_store() {
      vector<char> data = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), N(proxy), N(proxy));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("ProxyStorage", data, abi_serializer_max_time);
   }

   fc::variant get_dodo_store() {
      vector<char> data = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), N(dodo), N(dodo));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("DODOStorage", data, abi_serializer_max_time);
   }

   fc::variant get_token_store() {
      vector<char> data = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), N(token), N(token));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("TokenStorage", data, abi_serializer_max_time);
   }

   fc::variant get_oracle_store() {
      vector<char> data = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), N(oracle), N(oracle));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("OracleStorage", data, abi_serializer_max_time);
   }

   fc::variant get_helperstore() {
      vector<char> data = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), N(helper), N(helper));
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("HelperStorage", data, abi_serializer_max_time);
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

   std::string balanceOf(const extended_symbol& token, name account) {
      const auto s = get_token_store();
      const auto t = find_variant(s["tokens"], ns_to_string(to_namesym(token)));
      const auto b = find_value(t["balance"], account.to_string());
      return b;
   }

   std::string allowance(const extended_symbol& token, name src, name dst) {
      const auto s   = get_token_store();
      const auto t   = find_variant(s["tokens"], ns_to_string(to_namesym(token)));
      const auto a   = find_variant(t["allowance"], src.to_string());
      const auto amt = find_value(a["dst2amt"], dst.to_string());
      return amt;
   }

   fc::variant records(name pool_name, const extended_symbol& token) {
      const auto ps = get_pool_store();
      const auto p  = find_variant(ps["pools"], pool_name.to_string());
      const auto r  = find_value(p["records"], ns_to_string(to_namesym(token)));
      return r;
   }

   fc::variant pools(name pool_name) {
      const auto ps = get_pool_store();
      const auto p  = find_variant(ps["pools"], pool_name.to_string());
      return p;
   }

   uint_eth to_wei(uint_eth value) { return value * pow(10, 6); }

   extended_asset to_pool_asset(name pool_name, int64_t value) {
      return extended_asset{asset{value, symbol{4, "BPT"}}, pool_name};
   }

   extended_symbol to_pool_sym(name pool_name) { return extended_symbol{symbol{4, "BPT"}, pool_name}; }

   extended_asset to_asset(const std::string& sym, int64_t value) {
      return extended_asset{asset{value, symbol{4, sym.c_str()}}, name{"eosdosxtoken"}};
   }
   extended_asset to_wei_asset(const std::string& sym, uint_eth value) {
      return to_asset(sym, static_cast<int64_t>(to_wei(value)));
   }

   extended_symbol to_sym(const std::string& sym) {
      return extended_symbol{symbol{4, sym.c_str()}, name{"eosdosxtoken"}};
   }

   extended_asset to_maximum_supply(const std::string& sym) {
      return extended_asset{asset{1000000000000000, symbol{4, sym.c_str()}}, name{"eosdosxtoken"}};
   }

   namesym to_namesym(const extended_symbol& exsym) {
      namesym ns = exsym.contract.to_uint64_t();
      return ns << 64 | exsym.sym.value();
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

   void newpoolBefore() { newpool(admin, N(pool)); }

   void setswapfeeBefore() {
      // await pool.setSwapFee(toWei('0.003'));
      setswapfee(admin, N(pool), 3000);
   }

   void mintBefore() {
      newtoken(admin, to_maximum_supply("WETH"));
      newtoken(admin, to_maximum_supply("MKR"));
      newtoken(admin, to_maximum_supply("DAI"));
      newtoken(admin, to_maximum_supply("XXX"));

      mint(admin, to_wei_asset("WETH", 50));
      mint(admin, to_wei_asset("MKR", 20));
      mint(admin, to_wei_asset("DAI", 10000));
      mint(admin, to_wei_asset("XXX", 10));

      mint(user1, to_wei_asset("WETH", 25));
      mint(user1, to_wei_asset("MKR", 4));
      mint(user1, to_wei_asset("DAI", 40000));
      mint(user1, to_wei_asset("XXX", 10));

      mint(user2, to_asset("WETH", 12222200));
      mint(user2, to_asset("MKR", 1015333));
      mint(user2, to_asset("DAI", 1));
      mint(user2, to_wei_asset("XXX", 51));

      mint(nonadmin, to_wei_asset("WETH", 1));
      mint(nonadmin, to_wei_asset("DAI", 200));
   }

   void bindBefore() {
      bind(admin, N(pool), to_wei_asset("WETH", 50), to_wei(5));
      bind(admin, N(pool), to_wei_asset("MKR", 20), to_wei(5));
      bind(admin, N(pool), to_wei_asset("DAI", 10000), to_wei(5));
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

   void mintBefore1() {

      newtoken(admin, to_maximum_supply("WETH"));
      newtoken(admin, to_maximum_supply("DAI"));

      mint(admin, to_wei_asset("WETH", 5));
      mint(admin, to_wei_asset("DAI", 200));

      mint(nonadmin, to_wei_asset("WETH", 1));
      mint(nonadmin, to_wei_asset("DAI", 200));
   }

   void bindBefore1() {
      bind(admin, N(pool), to_wei_asset("WETH", 5), to_wei(5));
      bind(admin, N(pool), to_wei_asset("DAI", 200), to_wei(5));
   }

   void joinpoolBefore1() {
      std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1)};
      joinpool(nonadmin, N(pool), to_wei(10), v);
   }

   void exitpoolBefore1() { exitpool(nonadmin, N(pool), to_wei(10), std::vector<uint_eth>{0, 0}); }
   void before1() {
      newpoolBefore();
      mintBefore1();
      bindBefore1();
      finalizeBefore();
      joinpoolBefore1();
      exitpoolBefore1();
   }

   bool is_auth_token;

   name           admin;
   name           nonadmin;
   name           user1;
   name           user2;
   abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eosdos_tests)
////////////////zoo////////////////////
BOOST_FIXTURE_TEST_CASE(newpool_tests, eosdos_tester) try { newpool(admin, N(pool)); }
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(setblabs_tests, eosdos_tester) try {
   setblabs(N(eosdoseosdos), N(alice));
   std::string b = get_factory_store()["blabs"].as_string();
   BOOST_REQUIRE_EQUAL("alice", b);
}
FC_LOG_AND_RETHROW()

////////////////proxy////////////////////

BOOST_FIXTURE_TEST_CASE(bind_tests, eosdos_tester) try {

   newpoolBefore();
   mintBefore1();
   bind(admin, N(pool), to_wei_asset("WETH", 5), to_wei(5));
   bind(admin, N(pool), to_wei_asset("DAI", 200), to_wei(5));
}
FC_LOG_AND_RETHROW()


////////////////token////////////////////
BOOST_FIXTURE_TEST_CASE(mint_tests, eosdos_tester) try {
   newpool(admin, N(pool));
   newtoken(admin, to_maximum_supply("WETH"));
   newtoken(admin, to_maximum_supply("DAI"));
   mint(N(alice), to_wei_asset("WETH", 5));
   mint(N(alice), to_wei_asset("DAI", 200));

   const auto ab = balanceOf(to_sym("WETH"), N(alice));
   BOOST_REQUIRE_EQUAL(std::to_string(to_wei(5)), ab);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(extransfer_tests, eosdos_tester) try {
   const std::string     token_name = "POOL";
   const extended_asset& max_supply = to_asset(token_name, 1);
   newtoken(admin, to_maximum_supply(token_name));
   mint(N(alice1111111), max_supply);
   const symbol& sym = max_supply.quantity.get_symbol();
   extransfer(N(alice1111111), user2, max_supply);

   BOOST_REQUIRE_EQUAL(eosio::chain::asset::from_string("0.0001 " + token_name), get_balancex(user2, sym));
}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
