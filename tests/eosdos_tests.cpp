#include "eosio.system_tester.hpp"
#include <boost/test/unit_test.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/testing/tester.hpp>

#include "Runtime/Runtime.h"

#include <boost/multiprecision/cpp_int.hpp>
#include <fc/variant_object.hpp>
#include <sstream>
// #define EOSDOS_DEBUG
#ifdef EOSDOS_DEBUG
#define LINE_DEBUG BOOST_TEST_CHECK(__LINE__ == 0);
#else
#define LINE_DEBUG
#endif
using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo      = fc::mutable_variant_object;
using uint_eth = uint64_t;
using namesym  = eosio::chain::uint128_t;
using address  = name;
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

using std::string;
using checksum256 = fc::sha256;

template <typename T>
void push(T&) {}

template <typename Stream, typename T, typename... Types>
void push(Stream& s, T arg, Types... args) {
   s << arg;
   push(s, args...);
}

template <class... Types>
checksum256 get_checksum256(const Types&... args) {
   datastream<size_t> ps;
   push(ps, args...);
   size_t size = ps.tellp();

   std::vector<char> result;
   result.resize(size);

   datastream<char*> ds(result.data(), result.size());
   push(ds, args...);
   checksum256 digest = sha256::hash(result.data(), result.size());
   return digest;
}

// inline bool is_equal_capi_checksum256( checksum256 a, checksum256 b ){
//    return std::memcmp( a.hash, b.hash, 32 ) == 0;
// }

uint64_t get_hash_key(checksum256 hash) {
   const uint64_t* p64 = reinterpret_cast<const uint64_t*>(&hash);
   return p64[0] ^ p64[1] ^ p64[2] ^ p64[3];
}

class eosdos_tester : public tester {
 public:
   eosdos_tester() {
      produce_blocks(2);

      create_accounts({N(eosio.token), N(eosio.ram), N(eosio.ramfee), N(eosio.stake), N(eosio.bpay), N(eosio.vpay),
                       N(eosio.saving), N(eosio.names), N(eosio.rex)});

      create_accounts({N(alice), N(bob), N(carol), N(eosdoseosdos), N(eosdosoracle), N(ethbasemkr11), N(ethquotemkr1),
                       N(daimkrdaimkr), N(maintainer11), N(tokenissuer1), N(dodoowner111)});
      create_accounts({N(weth), N(dai), N(mkr), N(xxx), N(eosdosxtoken)});
      produce_blocks(2);
      admin                = N(eosdoseosdos);
      doowner              = N(dodoowner111);
      tokenissuer          = N(tokenissuer1);
      maintainer           = N(maintainer11);
      oracleadmin          = N(eosdosoracle);
      lp                   = N(alice);
      trader               = N(bob);
      dodo_ethbase_name    = N(ethbasemkr11);
      dodo_ethquote_name   = N(ethquotemkr1);
      dodo_stablecoin_name = N(daimkrdaimkr);

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

      set_code(N(ethbasemkr11), contracts::token_wasm());
      set_abi(N(ethbasemkr11), contracts::token_abi().data());

      set_code(N(ethquotemkr1), contracts::token_wasm());
      set_abi(N(ethquotemkr1), contracts::token_abi().data());

      set_code(N(daimkrdaimkr), contracts::token_wasm());
      set_abi(N(daimkrdaimkr), contracts::token_abi().data());

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

      std::vector<name> test_core_accounts = {N(alice),        N(bob),          N(carol),       N(eosdoseosdos),
                                              N(eosdosoracle), N(ethbasemkr11), N(ethquotemkr1)};
      for (auto& acc_name : test_core_accounts) {
         transfer(config::system_account_name, acc_name, "100000.0000");
         produce_blocks(1);
      }

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
         push_permission_update_auth_action(admin);
         push_permission_update_auth_action(N(eosdosxtoken));
         push_permission_update_auth_action(lp);
         push_permission_update_auth_action(trader);
         push_permission_update_auth_action(N(alice1111111));
         push_permission_update_auth_action(dodo_ethbase_name);
         push_permission_update_auth_action(dodo_ethquote_name);
         push_permission_update_auth_action(dodo_stablecoin_name);
         push_permission_update_auth_action(doowner);
         push_permission_update_auth_action(tokenissuer);
         push_permission_update_auth_action(maintainer);
         push_permission_update_auth_action(oracleadmin);
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
   action_result newdodo(
       name msg_sender, name dodo_name, address owner, address supervisor, address maintainer,
       const extended_symbol& baseToken, const extended_symbol& quoteToken, const extended_symbol& oracle,
       uint64_t lpFeeRate, uint64_t mtFeeRate, uint64_t k, uint64_t gasPriceLimit) {
      return push_action(
          msg_sender, N(newdodo),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("owner", owner)("supervisor", supervisor)(
              "maintainer", maintainer)("baseToken", baseToken)("quoteToken", quoteToken)("oracle", oracle)(
              "lpFeeRate", lpFeeRate)("mtFeeRate", mtFeeRate)("k", k)("gasPriceLimit", gasPriceLimit));
   }
   action_result adddodo(name msg_sender, address _dodo) {
      return push_action(msg_sender, N(adddodo), mvo()("msg_sender", msg_sender)("_dodo", _dodo));
   }

   action_result removedodo(name msg_sender, address _dodo) {
      return push_action(msg_sender, N(removedodo), mvo()("msg_sender", msg_sender)("_dodo", _dodo));
   }

   action_result breeddodo(
       name msg_sender, name dodo_name, address maintainer, const extended_symbol& baseToken,
       const extended_symbol& quoteToken, const extended_symbol& oracle, uint64_t lpFeeRate, uint64_t mtFeeRate,
       uint64_t k, uint64_t gasPriceLimit) {
      return push_action(
          msg_sender, N(breeddodo),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("maintainer", maintainer)("baseToken", baseToken)(
              "quoteToken", quoteToken)("oracle", oracle)("lpFeeRate", lpFeeRate)("mtFeeRate", mtFeeRate)("k", k)(
              "gasPriceLimit", gasPriceLimit));
   }

   ///////////////proxy ////////////////////
   // msg_sender, name dst, uint_eth amt

   action_result
   init(name msg_sender, address dodoZoo, const extended_symbol& weth, const extended_symbol& core_symbol) {
      return push_action(
          msg_sender, N(init),
          mvo()("msg_sender", msg_sender)("dodoZoo", dodoZoo)("weth", weth)("core_symbol", core_symbol));
   }

   action_result sellethtoken(name msg_sender, const extended_asset& ethToken, const extended_asset& minReceiveToken) {
      return push_action(
          msg_sender, N(sellethtoken),
          mvo()("msg_sender", msg_sender)("ethToken", ethToken)("minReceiveToken", minReceiveToken));
   }

   action_result buyethtoken(name msg_sender, const extended_asset& ethToken, const extended_asset& maxPayTokenAmount) {
      return push_action(
          msg_sender, N(buyethtoken),
          mvo()("msg_sender", msg_sender)("ethToken", ethToken)("maxPayTokenAmount", maxPayTokenAmount));
   }

   action_result selltokeneth(name msg_sender, const extended_asset& baseToken, const extended_asset& minReceiveEth) {
      return push_action(
          msg_sender, N(selltokeneth),
          mvo()("msg_sender", msg_sender)("baseToken", baseToken)("minReceiveEth", minReceiveEth));
   }

   action_result buytokeneth(name msg_sender, const extended_asset& baseToken, const extended_asset& maxPayEthAmount) {
      return push_action(
          msg_sender, N(buytokeneth),
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

   //////////////////admin dodo//////////////
   action_result enablequodep(name msg_sender, name dodo_name) {
      return push_action(msg_sender, N(enablequodep), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name));
   }

   action_result enablebasdep(name msg_sender, name dodo_name) {
      return push_action(msg_sender, N(enablebasdep), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name));
   }

   action_result enabletradin(name msg_sender, name dodo_name) {
      return push_action(msg_sender, N(enabletradin), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name));
   }

   action_result setparameter(name msg_sender, name dodo_name, name para_name, uint64_t para_value) {
      return push_action(
          msg_sender, N(enablequodep),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("para_name", para_name)("para_value", para_value));
   }

   //////////////////LiquidityProvider dodo//////////////
   action_result depositquote(name msg_sender, name dodo_name, const extended_asset& amt) {
      return push_action(
          msg_sender, N(depositquote), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amt", amt));
   }
   action_result depositbase(name msg_sender, name dodo_name, const extended_asset& amt) {
      return push_action(
          msg_sender, N(depositbase), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amt", amt));
   }

   action_result withdrawquote(name msg_sender, name dodo_name, const extended_asset& amt) {
      return push_action(
          msg_sender, N(withdrawquote), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amt", amt));
   }
   action_result withdrawbase(name msg_sender, name dodo_name, const extended_asset& amt) {
      return push_action(
          msg_sender, N(withdrawbase), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amt", amt));
   }
   action_result withdrawallq(name msg_sender, name dodo_name) {
      return push_action(msg_sender, N(withdrawallq), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name));
   }
   action_result withdrawallb(name msg_sender, name dodo_name) {
      return push_action(msg_sender, N(withdrawallb), mvo()("msg_sender", msg_sender)("dodo_name", dodo_name));
   }

   action_result
   sellbastoken(name msg_sender, name dodo_name, const extended_asset& amount, const extended_asset& minReceiveQuote) {
      return push_action(
          msg_sender, N(sellbastoken),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amount", amount)(
              "minReceiveQuote", minReceiveQuote));
   }
   action_result
   buybasetoken(name msg_sender, name dodo_name, const extended_asset& amount, const extended_asset& maxPayQuote) {
      return push_action(
          msg_sender, N(buybasetoken),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amount", amount)("maxPayQuote", maxPayQuote));
   }
   //////////////////Oracle//////////////
   action_result setprice(name msg_sender, const extended_symbol& basetoken, const extended_asset& quotetoken) {
      return push_action(
          msg_sender, N(setprice), mvo()("msg_sender", msg_sender)("basetoken", basetoken)("quotetoken", quotetoken));
   }

   //////////////////TOKEN//////////////
   action_result extransfer(name from, name to, const extended_asset& quantity, const std::string& memo = "") {
      return push_action(from, N(extransfer), mvo()("from", from)("to", to)("quantity", quantity)("memo", memo));
   }

   action_result newtoken(account_name msg_sender, const extended_asset& token) {
      return push_action(msg_sender, N(newtoken), mvo()("msg_sender", msg_sender)("token", token));
   }

   action_result mint(account_name msg_sender, const extended_asset& amt) {
      return push_action(msg_sender, N(mint), mvo()("msg_sender", msg_sender)("amt", amt));
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

   fc::variant get_dodo_table(name dodo_name) {
      name         table_name = N(dodos);
      vector<char> data       = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), table_name, dodo_name);
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("dodo_storage", data, abi_serializer_max_time);
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

   fc::variant get_oracle_table(const extended_symbol& basetoken, const extended_symbol& quotetoken) {
      uint64_t    key = get_hash_key(get_checksum256(
          basetoken.contract.to_uint64_t(), basetoken.sym.value(), quotetoken.contract.to_uint64_t(),
          quotetoken.sym.value()));
      const auto& db  = control->db();
      const auto* tbl = db.find<table_id_object, by_code_scope_table>(
          boost::make_tuple(N(eosdoseosdos), N(eosdoseosdos), N(oracles)));
      vector<char> data;
      // the balance is implied to be 0 if either the table or row does not exist
      if (tbl) {
         const auto* obj = db.find<key_value_object, by_scope_primary>(boost::make_tuple(tbl->id, key));
         if (obj) {
            data.resize(obj->value.size());
            memcpy(data.data(), obj->value.data(), data.size());
         }
      }
      if (data.empty())
         std::cout << "\nData is empty\n" << std::endl;
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("oracle_storage", data, abi_serializer_max_time);
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

   fc::variant dodos(name dodo_name) {
      const auto f = get_dodo_table(dodo_name);
      return f["dodos"];
      //   const auto f = get_dodo_store();
      //   const auto p = find_variant(f["dodos"], dodo_name.to_string());
      //   return p;
   }

   uint_eth to_wei(uint_eth value) { return value * pow(10, 4); }

   extended_asset to_pl_asset(int64_t value, const std::string& sym, name lp_contract_name) {
      return extended_asset{asset{value, symbol{4, sym.c_str()}}, lp_contract_name};
   }

   extended_symbol to_lp_esym(const std::string& sym, name lp_contract_name) {
      return extended_symbol{symbol{4, sym.c_str()}, lp_contract_name};
   }

   symbol to_lp_sym(const std::string& sym) { return symbol{4, sym.c_str()}; }

   extended_asset to_asset(int64_t value, const std::string& sym) {
      return extended_asset{asset{value, symbol{4, sym.c_str()}}, name{"eosdosxtoken"}};
   }
   extended_asset to_wei_asset(uint_eth value, const std::string& sym) {
      return to_asset(static_cast<int64_t>(to_wei(value)), sym);
   }

   extended_symbol get_core_symbol() { return extended_symbol{symbol{CORE_SYM}, name{"eosio.token"}}; }

   symbol to_sym_from_string(const std::string& sym) { return symbol{4, sym.c_str()}; }

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

   void initProxyBefore() {
      LINE_DEBUG;
      init(admin, maintainer, to_sym("WETH"), get_core_symbol());
   }

   void newTokenBefore() {
      LINE_DEBUG;
      newtoken(tokenissuer, to_maximum_supply("WETH"));
      LINE_DEBUG;
      newtoken(tokenissuer, to_maximum_supply("DAI"));
      LINE_DEBUG;
      newtoken(tokenissuer, to_maximum_supply("MKR"));
   }

   void mintBefore() {
      mint(lp, to_wei_asset(1000, "MKR"));
      LINE_DEBUG;

      mint(trader, to_wei_asset(1000, "MKR"));
      LINE_DEBUG;
   }

   void mintStableBefore() {
      mint(lp, to_wei_asset(10000, "DAI"));
      LINE_DEBUG;

      mint(trader, to_wei_asset(10000, "DAI"));
      LINE_DEBUG;
      mint(lp, to_wei_asset(10000, "MKR"));
      LINE_DEBUG;

      mint(trader, to_wei_asset(10000, "MKR"));
      LINE_DEBUG;
   }

   void setPriceBaseBefore() { setprice(oracleadmin, to_sym("WETH"), to_wei_asset(100, "MKR")); }

   void setPriceQuoteBefore() { setprice(oracleadmin, to_sym("MKR"), to_asset(100, "WETH")); }

   void setPriceStableBefore() { setprice(oracleadmin, to_sym("DAI"), to_wei_asset(1, "MKR")); }

   void breedBaseBefore() {

      LINE_DEBUG;
      name            msg_sender    = admin;
      name            dodo_name     = dodo_ethbase_name;
      address         maintainer    = doowner;
      extended_symbol baseToken     = to_sym("WETH");
      extended_symbol quoteToken    = to_sym("MKR");
      extended_symbol oracle        = to_sym("WETH");
      uint64_t        lpFeeRate     = 2;
      uint64_t        mtFeeRate     = 1;
      uint64_t        k             = 1;
      uint64_t        gasPriceLimit = 0; // gweiStr("100")
      //    auto oracle_store = get_oracle_store();
      //    BOOST_TEST_CHECK(nullptr == oracle_store);
      //   lpFeeRate: decimalStr("0.002"),
      //   mtFeeRate: decimalStr("0.001"),
      //   k: decimalStr("0.1"),
      //   gasPriceLimit: gweiStr("100"),

      LINE_DEBUG;
      breeddodo(
          msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
   }

   void breedQuoteBefore() {

      LINE_DEBUG;
      name            msg_sender    = admin;
      name            dodo_name     = dodo_ethquote_name;
      address         maintainer    = doowner;
      extended_symbol baseToken     = to_sym("WETH");
      extended_symbol quoteToken    = to_sym("MKR");
      extended_symbol oracle        = to_sym("MKR");
      uint64_t        lpFeeRate     = 2;
      uint64_t        mtFeeRate     = 2;
      uint64_t        k             = 1;
      uint64_t        gasPriceLimit = 0; // gweiStr("100")
      //    auto oracle_store = get_oracle_store();
      //    BOOST_TEST_CHECK(nullptr == oracle_store);
      //   lpFeeRate: decimalStr("0.002"),
      //   mtFeeRate: decimalStr("0.001"),
      //   k: decimalStr("0.1"),
      //   gasPriceLimit: gweiStr("100"),

      LINE_DEBUG;
      breeddodo(
          msg_sender, dodo_name, maintainer, quoteToken, baseToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
   }

   void breedStableBefore() {

      LINE_DEBUG;
      name            msg_sender    = admin;
      name            dodo_name     = dodo_stablecoin_name;
      address         maintainer    = doowner;
      extended_symbol baseToken     = to_sym("DAI");
      extended_symbol quoteToken    = to_sym("MKR");
      extended_symbol oracle        = to_sym("DAI");
      uint64_t        lpFeeRate     = 1;
      uint64_t        mtFeeRate     = 0;
      uint64_t        k             = 1;
      uint64_t        gasPriceLimit = 0; // gweiStr("100")
                                         //   lpFeeRate: decimalStr("0.0001"),
                                         //   mtFeeRate: decimalStr("0"),
                                         //   k: gweiStr("1"), // nearly zero
                                         //   gasPriceLimit: gweiStr("100"),

      LINE_DEBUG;
      breeddodo(
          msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
   }

   void enableBaseBefore() {
      LINE_DEBUG;
      name dodo_name = dodo_ethbase_name;
      enabletradin(admin, dodo_name);
      enablequodep(admin, dodo_name);
      enablebasdep(admin, dodo_name);
   }

   void enableQuoteBefore() {
      LINE_DEBUG;
      name dodo_name = dodo_ethquote_name;
      enabletradin(admin, dodo_name);
      enablequodep(admin, dodo_name);
      enablebasdep(admin, dodo_name);
   }

   void enableStableBefore() {
      LINE_DEBUG;
      name dodo_name = dodo_stablecoin_name;
      enabletradin(admin, dodo_name);
      enablequodep(admin, dodo_name);
      enablebasdep(admin, dodo_name);
   }

   void depositQuoteBefore() {
      LINE_DEBUG;
      depositquote(lp, dodo_ethbase_name, to_wei_asset(1000, "MKR"));
   }

   void depositBaseBefore() {
      LINE_DEBUG;
      depositbase(lp, dodo_ethquote_name, to_wei_asset(1000, "MKR"));
   }

   void depositQuoteStableBefore() {
      LINE_DEBUG;
      depositquote(lp, dodo_stablecoin_name, to_wei_asset(10000, "MKR"));
   }

   void depositBaseStableBefore() {
      LINE_DEBUG;
      depositbase(lp, dodo_stablecoin_name, to_wei_asset(10000, "DAI"));
   }

   void depositEthAsBaseBefore() {
      LINE_DEBUG;
      depositethab(lp, to_wei_asset(10, "WETH"), to_sym("MKR"));
   }

   void depositEthAsQuoteBefore() {
      LINE_DEBUG;
      depositethaq(lp, to_wei_asset(10, "WETH"), to_sym("MKR"));
   }
   void ethBaseBefore() {
      initProxyBefore();
      newTokenBefore();
      mintBefore();
      setPriceBaseBefore();
      breedBaseBefore();
      enableBaseBefore();

      depositQuoteBefore();
      depositEthAsBaseBefore();
   }

   void ethQuoteBefore() {
      initProxyBefore();
      newTokenBefore();
      mintBefore();

      setPriceQuoteBefore();
      breedQuoteBefore();
      enableQuoteBefore();

      depositBaseBefore();
      depositEthAsQuoteBefore();
   }

   void stableCoinBefore() {
      newTokenBefore();
      mintStableBefore();

      setPriceStableBefore();
      breedStableBefore();
      enableStableBefore();

      depositBaseStableBefore();
      depositQuoteStableBefore();
   }

   void check_balance(std::string token_name, std::string amount) {
      auto sym = to_sym_from_string(token_name);
      auto c   = eosio::chain::asset::from_string(amount + " " + token_name);
      auto b   = get_balancex(trader, sym);
      BOOST_TEST_CHECK(c == b);
   }

   bool is_auth_token;

   name           admin;
   name           doowner;
   name           tokenissuer;
   name           maintainer;
   name           oracleadmin;
   name           lp;
   name           trader;
   name           dodo_ethbase_name;
   name           dodo_ethquote_name;
   name           dodo_stablecoin_name;
   abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eosdos_tests)
////////////////dodo admin////////////////////

BOOST_FIXTURE_TEST_CASE(setparameter_tests, eosdos_tester) try {
   stableCoinBefore();
   name dodo_name = dodo_stablecoin_name;
   setparameter(admin, dodo_name, N(k), 100);
   setparameter(admin, dodo_name, N(feerate), 1);
}
FC_LOG_AND_RETHROW()

/////////////////////dodo///////////////////////
BOOST_FIXTURE_TEST_CASE(buy_base_token_tests, eosdos_tester) try {
   stableCoinBefore();
   buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "DAI"), to_wei_asset(1001, "MKR"));

   check_balance("DAI", "11000.0000");
   check_balance("MKR", "8999.9000");

   buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(8906, "DAI"), to_wei_asset(9000, "MKR"));
   check_balance("DAI", "19906.0000");
   check_balance("MKR", "8.3940");

   sellbastoken(trader, dodo_stablecoin_name, to_wei_asset(19900, "DAI"), to_asset(19970, "MKR"));
   check_balance("DAI", "6.0000");
   check_balance("MKR", "19912.7339");
   //   // 10% depth avg price 1.000100000111135
   //   await ctx.DODO.methods.buyBaseToken(decimalStr("1000"), decimalStr("1001"), "0x").send(ctx.sendParam(trader))
   //   assert.equal(await ctx.BASE.methods.balanceOf(trader).call(), decimalStr("11000"))
   //   assert.equal(await ctx.QUOTE.methods.balanceOf(trader).call(), "8999 89 9999 8888 6543 1655")

   //   // 99.9% depth avg price 1.00010109
   //   await ctx.DODO.methods.buyBaseToken(decimalStr("8990"), decimalStr("10000"), "0x").send(ctx.sendParam(trader))
   //   assert.equal(await ctx.BASE.methods.balanceOf(trader).call(), decimalStr("19990"))
   //   assert.equal(await ctx.QUOTE.methods.balanceOf(trader).call(), "8 99 0031 9678 0692 1648")

   //   // sell to 99.9% depth avg price 0.9999
   //   await ctx.DODO.methods.sellBaseToken(decimalStr("19980"), decimalStr("19970"), "0x").send(ctx.sendParam(trader))
   //   assert.equal(await ctx.BASE.methods.balanceOf(trader).call(), decimalStr("10"))
   //   assert.equal(await ctx.QUOTE.methods.balanceOf(trader).call(), "19986992950440794518402")
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(huge_sell_base_token_tests, eosdos_tester) try {
   stableCoinBefore();

   mint(trader, to_wei_asset(10000, "DAI"));
   LINE_DEBUG;

   sellbastoken(trader, dodo_stablecoin_name, to_wei_asset(20000, "DAI"), to_asset(0, "MKR"));
   check_balance("DAI", "0.0000");
   check_balance("MKR", "19998.0004");

   //   it("huge sell trading amount", async () => {
   //       // trader could sell any number of base token
   //       // but the price will drop quickly
   //       await ctx.mintTestToken(trader, decimalStr("10000"), decimalStr("0"))
   //       await ctx.DODO.methods.sellBaseToken(decimalStr("20000"), decimalStr("0"), "0x").send(ctx.sendParam(trader))

   //       assert.equal(await ctx.BASE.methods.balanceOf(trader).call(), decimalStr("0"))
   //       assert.equal(await ctx.QUOTE.methods.balanceOf(trader).call(), "19998999990001000029997")
   //     })
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(huge_buy_base_token_tests, eosdos_tester) try {
   stableCoinBefore();

   mint(trader, to_wei_asset(10000, "MKR"));
   LINE_DEBUG;

   buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(9990, "DAI"), to_wei_asset(20000, "MKR"));
   check_balance("DAI", "19990.0000");
   check_balance("MKR", "8900.9993");

   //     it("huge buy trading amount", async () => {
   //       // could not buy all base balance
   //       await assert.rejects(
   //         ctx.DODO.methods.buyBaseToken(decimalStr("10000"), decimalStr("10010"), "0x").send(ctx.sendParam(trader)),
   //         /DODO_BASE_BALANCE_NOT_ENOUGH/
   //       )

   //       // when buy amount close to base balance, price will increase quickly
   //       await ctx.mintTestToken(trader, decimalStr("0"), decimalStr("10000"))
   //       await ctx.DODO.methods.buyBaseToken(decimalStr("9999"), decimalStr("20000"),
   //       "0x").send(ctx.sendParam(trader))
   // assert.equal(await ctx.BASE.methods.balanceOf(trader).call(),
   //       decimalStr("19999"))
   // assert.equal(await ctx.QUOTE.methods.balanceOf(trader).call(),
   //       "9000 00 0119 9999 9990 0000")
   //     })
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(tiny_withdraw_base_token_tests, eosdos_tester) try {
   stableCoinBefore();

   buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(9900, "DAI"), to_wei_asset(10000, "MKR"));
   check_balance("DAI", "19900.0000");
   check_balance("MKR", "0.9902");

   //     it("tiny withdraw penalty", async () => {
   //       await ctx.DODO.methods.buyBaseToken(decimalStr("9990"), decimalStr("10000"),
   //       "0x").send(ctx.sendParam(trader))

   //       // penalty only 0.2% even if withdraw make pool utilization rate raise to 99.5%
   //       assert.equal(await ctx.DODO.methods.getWithdrawBasePenalty(decimalStr("5")).call(), "9981967500000000")
   //     })
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(withdraw_dodo_tests, eosdos_tester) try {
   stableCoinBefore();
   uint64_t mint_amount = 10000000000;
   mint(lp, to_wei_asset(mint_amount, "DAI"));
   mint(lp, to_wei_asset(mint_amount, "MKR"));
   depositquote(lp, dodo_stablecoin_name, to_wei_asset(mint_amount, "MKR"));
   depositbase(lp, dodo_stablecoin_name, to_wei_asset(mint_amount, "DAI"));
   withdrawbase(lp, dodo_stablecoin_name, to_wei_asset(1000, "DAI"));
   withdrawquote(lp, dodo_stablecoin_name, to_wei_asset(9000, "MKR"));

   withdrawallb(lp, dodo_stablecoin_name);
   withdrawallq(lp, dodo_stablecoin_name);
}
FC_LOG_AND_RETHROW()

////////////////proxy////eth base////////////////
BOOST_FIXTURE_TEST_CASE(buy_eth_with_token_tests, eosdos_tester) try {
   ethBaseBefore();

   buyethtoken(trader, to_wei_asset(1, "WETH"), to_wei_asset(200, "MKR"));

   auto store = dodos(dodo_ethbase_name);
   //    BOOST_TEST_CHECK(nullptr==store);
   BOOST_TEST_CHECK("89999" == store["_BASE_BALANCE_"].as_string());
   std::string quote_token_name = "MKR";
   auto        sym              = to_sym_from_string(quote_token_name);
   auto        c                = eosio::chain::asset::from_string("899.9700 " + quote_token_name);
   auto        b                = get_balancex(trader, sym);
   BOOST_TEST_CHECK(c == b);
   // 898581839502056240973
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(sell_eth_to_token_tests, eosdos_tester) try {
   ethBaseBefore();
   sellethtoken(trader, to_wei_asset(1, "WETH"), to_wei_asset(50, "MKR"));
   auto store = dodos(dodo_ethbase_name);
   BOOST_TEST_CHECK("110000" == store["_BASE_BALANCE_"].as_string());

   std::string token_name = "MKR";
   auto        sym        = to_sym_from_string(token_name);
   auto        c          = eosio::chain::asset::from_string("1099.9690 " + token_name);
   auto        b          = get_balancex(trader, sym);
   BOOST_TEST_CHECK(c == b);
   // 1098617454226610630663
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(withdraw_eth_as_base_tests, eosdos_tester) try {
   ethBaseBefore();
   withdraweab(lp, to_wei_asset(5, "WETH"), to_sym("MKR"));
   std::string token_name = "WETH";
   auto        sym        = to_lp_esym(token_name, dodo_ethbase_name);
   auto        c          = eosio::chain::asset::from_string("5.0000 " + token_name);
   auto        b          = get_balancex(lp, sym.sym, sym.contract);
   BOOST_TEST_CHECK(c == b);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(withdraw_all_eth_as_base_tests, eosdos_tester) try {
   ethBaseBefore();
   withdrawaeab(lp, to_sym("MKR"));
   std::string token_name = "WETH";
   auto        sym        = to_lp_esym(token_name, dodo_ethbase_name);
   auto        c          = eosio::chain::asset::from_string("0.0000 " + token_name);
   auto        b          = get_balancex(lp, sym.sym, sym.contract);
   BOOST_TEST_CHECK(c == b);
}
FC_LOG_AND_RETHROW()

////////////////proxy eth quote////////////////////
BOOST_FIXTURE_TEST_CASE(buy_token_with_eth_tests, eosdos_tester) try {
   ethQuoteBefore();

   buytokeneth(trader, to_wei_asset(200, "MKR"), to_asset(21000, "WETH"));

   auto store = dodos(dodo_ethquote_name);
   BOOST_TEST_CHECK("120008" == store["_QUOTE_BALANCE_"].as_string());
   std::string quote_token_name = "MKR";
   auto        sym              = to_sym_from_string(quote_token_name);
   auto        c                = eosio::chain::asset::from_string("1200.0000 " + quote_token_name);
   auto        b                = get_balancex(trader, sym);
   BOOST_TEST_CHECK(c == b);

   //  const maxPayEthAmount = "2.1";
   //       const ethInPoolBefore = decimalStr("10");

   //         DODOEthProxy.methods.buyTokenWithEth(
   //           ctx.BASE.options.address,
   //           decimalStr("200"),
   //           decimalStr(maxPayEthAmount)
   //       const ethInPoolAfter = "12056338203652739553";
   //       assert.strictEqual(
   //         await ctx.DODO.methods._QUOTE_BALANCE_().call(),
   //         ethInPoolAfter
   //       );
   //       assert.strictEqual(
   //         await ctx.BASE.methods.balanceOf(trader).call(),
   //         decimalStr("1200")
   //       );
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(sell_token_to_eth_tests, eosdos_tester) try {
   ethQuoteBefore();
   selltokeneth(trader, to_wei_asset(50, "MKR"), to_asset(4500, "WETH"));
   auto store = dodos(dodo_ethquote_name);
   BOOST_TEST_CHECK("95001" == store["_QUOTE_BALANCE_"].as_string());

   std::string token_name = "MKR";
   auto        sym        = to_sym_from_string(token_name);
   auto        c          = eosio::chain::asset::from_string("950.0000 " + token_name);
   auto        b          = get_balancex(trader, sym);
   BOOST_TEST_CHECK(c == b);

   //  const minReceiveEthAmount = "0.45";

   //         DODOEthProxy.methods.sellTokenToEth(
   //           ctx.BASE.options.address,
   //           decimalStr("50"),
   //           decimalStr(minReceiveEthAmount)

   //       assert.strictEqual(
   //         await ctx.DODO.methods._QUOTE_BALANCE_().call(),
   //         "9503598324131652490"
   //       );
   //         await ctx.BASE.methods.balanceOf(trader).call(),
   //         decimalStr("950")
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(withdraw_eth_as_quote_tests, eosdos_tester) try {
   ethQuoteBefore();
   withdraweaq(lp, to_wei_asset(5, "WETH"), to_sym("MKR"));
   std::string token_name = "WETH";
   auto        sym        = to_lp_esym(token_name, dodo_ethquote_name);
   auto        c          = eosio::chain::asset::from_string("5.0000 " + token_name);
   auto        b          = get_balancex(lp, sym.sym, sym.contract);
   BOOST_TEST_CHECK(c == b);
   //    const withdrawAmount = decimalStr("5");

   //         .withdrawEthAsQuote(withdrawAmount, ctx.BASE.options.address)
   //         .send(ctx.sendParam(lp));

   //       assert.strictEqual(
   //         await ctx.DODO.methods.getLpQuoteBalance(lp).call(),
   // withdrawAmount
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(withdraw_all_eth_as_quote_tests, eosdos_tester) try {
   ethQuoteBefore();
   withdrawaeaq(lp, to_sym("MKR"));
   std::string token_name = "WETH";
   auto        sym        = to_lp_esym(token_name, dodo_ethquote_name);
   auto        c          = eosio::chain::asset::from_string("0.0000 " + token_name);
   auto        b          = get_balancex(lp, sym.sym, sym.contract);
   BOOST_TEST_CHECK(c == b);

   //    const withdrawAmount = decimalStr("10");
   //         .withdrawAllEthAsQuote(ctx.BASE.options.address)
   //         .send(ctx.sendParam(lp));
   //       assert.strictEqual(
   //         await ctx.DODO.methods.getLpQuoteBalance(lp).call(),
   //         "0"
}
FC_LOG_AND_RETHROW()

////////////////oracle////////////////////

BOOST_FIXTURE_TEST_CASE(setprice_tests, eosdos_tester) try {

   extended_asset ea = to_wei_asset(5, "MKR");
   setprice(oracleadmin, to_sym("WETH"), ea);
   setprice(oracleadmin, to_sym("DAI"), to_wei_asset(10, "MKR"));
   setprice(oracleadmin, to_sym("MKR"), to_wei_asset(20, "WETH"));
   auto bb = get_oracle_table(to_sym("WETH"), to_sym("MKR"));
   {
      auto a = "4,WETH"; // to_sym("WETH");
      auto b = bb["basetoken"]["sym"].as_string();
      BOOST_REQUIRE_EQUAL(b, a);
   }
   {
      auto a = "5.0000 MKR"; // to_sym("WETH");
      auto b = bb["quotetoken"]["quantity"].as_string();
      BOOST_REQUIRE_EQUAL(b, a);
   }
   // BOOST_TEST_CHECK(b["basetoken"]==to_sym("WETH"));
   //    REQUIRE_MATCHING_OBJECT( b, mvo()
   //       ("basetoken", to_sym("WETH"))
   //       ("quotetoken", to_wei_asset(5,"MKR"))
   //       ("_OWNER_", "")
   //       ("_NEW_OWNER_",  "" )
   //    );

   //  {
   //   "basetoken": {
   //     "sym": "4,WETH",
   //     "contract": "eosdosxtoken"
   //   },
   //   "quotetoken": {
   //     "quantity": "5.0000 MKR",
   //     "contract": "eosdosxtoken"
   //   },
   //   "_OWNER_": "",
   //   "_NEW_OWNER_": ""
   // }
}
FC_LOG_AND_RETHROW()

////////////////token////////////////////
BOOST_FIXTURE_TEST_CASE(mint_tests, eosdos_tester) try {
   //    newpool(admin, N(pool));
   newtoken(tokenissuer, to_maximum_supply("WETH"));
   newtoken(tokenissuer, to_maximum_supply("DAI"));
   mint(trader, to_wei_asset(5, "WETH"));
   mint(N(alice), to_wei_asset(200, "DAI"));

   std::string quote_token_name = "WETH";
   auto        sym              = to_sym_from_string(quote_token_name);
   auto        a                = eosio::chain::asset::from_string("5.0000 " + quote_token_name);
   auto        b                = get_balancex(trader, sym);
   BOOST_REQUIRE_EQUAL(a, b);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(extransfer_tests, eosdos_tester) try {
   const std::string     token_name = "POOL";
   const extended_asset& max_supply = to_asset(1, token_name);
   newtoken(tokenissuer, to_maximum_supply(token_name));
   mint(N(alice1111111), max_supply);
   const symbol& sym = max_supply.quantity.get_symbol();
   extransfer(N(alice1111111), maintainer, max_supply);

   BOOST_REQUIRE_EQUAL(eosio::chain::asset::from_string("0.0001 " + token_name), get_balancex(maintainer, sym));
}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
