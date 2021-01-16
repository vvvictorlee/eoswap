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

const std::string TOKEN_DECIMALS_STR    = "6,";
const int         TOKEN_DECIMALS        = 6;
const std::string LP_TOKEN_DECIMALS_STR = "6,";
const int         LP_TOKEN_DECIMALS     = 6;

using namespace eosio::testing;
using namespace eosio;
using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo      = fc::mutable_variant_object;
using uint256m = uint64_t;
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
      create_accounts({N(weth), N(dai), N(mkr), N(xxx), N(roxe.ro)});
      produce_blocks(2);
      admin       = N(eosdoseosdos);
      doowner     = N(dodoowner111);
      tokenissuer = N(tokenissuer1);
      maintainer  = N(maintainer11);
      //   admin          = N(eosdosoracle);
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
          N(eosio.token), config::system_account_name, eosio::chain::asset::from_string("1000000000.000000 EOS"));

      issue(config::system_account_name, eosio::chain::asset::from_string("1000000000.000000 EOS"));

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

      set_code(N(roxe.ro), contracts::token_wasm());
      set_abi(N(roxe.ro), contracts::token_abi().data());

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
      std::string         amt       = "100000.000000 ";
      std::string         memo      = "";
      //   for (auto& acc_name : accounts) {
      //      std::string token     = tokens[j++];
      //      std::string amount    = "1000000000.000000 " + token;
      //      std::string tamount   = amt + token;
      //      asset       maxsupply = eosio::chain::asset::from_string(amount.c_str());
      //      name        acc       = name(acc_name.c_str());

      //      create_currency(N(roxe.ro), acc, maxsupply);
      //      issuex(N(roxe.ro), acc, maxsupply, acc);
      //      produce_blocks(1);
      //      for (auto& to : taccounts) {
      //         transferex(N(roxe.ro), acc, to, tamount, acc, memo);
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
         push_permission_update_auth_action(N(roxe.ro));
         push_permission_update_auth_action(lp);
         push_permission_update_auth_action(trader);
         push_permission_update_auth_action(N(alice1111111));
         push_permission_update_auth_action(dodo_ethbase_name);
         push_permission_update_auth_action(dodo_ethquote_name);
         push_permission_update_auth_action(dodo_stablecoin_name);
         push_permission_update_auth_action(doowner);
         push_permission_update_auth_action(tokenissuer);
         push_permission_update_auth_action(maintainer);
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
   // msg_sender, name dst, uint256m amt

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
   action_result setadmin(name msg_sender, name dodo_name, name admin_name, name admin) {
      return push_action(
          msg_sender, N(setadmin),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("admin_name", admin_name)("admin", admin));
   }

   action_result setparameter(name msg_sender, name dodo_name, name para_name, uint64_t para_value) {
      return push_action(
          msg_sender, N(setparameter),
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

   action_result moveoracle(name msg_sender) {
      return push_action(msg_sender, N(moveoracle), mvo()("msg_sender", msg_sender));
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

   //////////////////TEST BUY& SELL//////////////
   action_result sellbasetest(
       name msg_sender, name dodo_name, const extended_asset& amount, const extended_asset& minReceiveQuote,
       const std::vector<int64_t>& params) {
      return push_action(
          msg_sender, N(sellbasetest),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amount", amount)("minReceiveQuote", minReceiveQuote)(
              "params", params));
   }

   action_result buybasetest(
       name msg_sender, name dodo_name, const extended_asset& amount, const extended_asset& maxPayQuote,
       const std::vector<int64_t>& params) {
      return push_action(
          msg_sender, N(buybasetest),
          mvo()("msg_sender", msg_sender)("dodo_name", dodo_name)("amount", amount)("maxPayQuote", maxPayQuote)(
              "params", params));
   }

   action_result setparametera(const std::string& symbol, const std::vector<int64_t> params) {
      return push_action(N(eosdoseosdos), N(setparametera), mvo()("symbol", symbol)("params", params));
   }

   ////////////////get table//////////////

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

   std::string get_account(account_name acc, const string& symbolname, name contract_name = N(extendxtoken)) {
      //   auto         symb        = eosio::chain::symbol::from_string(symbolname);
      //   auto         symbol_code = symb.to_symbol_code().value;
      //   vector<char> data        = get_row_by_account(N(eoswapeoswap), acc, N(accounts), account_name(symbol_code));
      //   return data.empty() ? fc::variant() : abi_ser.binary_to_variant("account", data, abi_serializer_max_time);
      auto a = get_accountx(acc, symbolname);
      return (a.is_null() ? "" : a.as_string());
   }

   fc::variant get_accountx(account_name acc, const string& symbolname) {
      vector<char> data;
      const auto&  db  = control->db();
      namespace chain  = eosio::chain;
      const auto* t_id = db.find<eosio::chain::table_id_object, chain::by_code_scope_table>(
          boost::make_tuple(N(eosdoseosdos), acc, N(accounts)));
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
#ifdef EOSDOS_DEBUG
            BOOST_TEST_CHECK(nullptr == d);
#endif
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

   fc::variant get_dodo_table(name dodo_name) {
      name         table_name = N(dodos);
      vector<char> data       = get_row_by_account(N(eosdoseosdos), N(eosdoseosdos), table_name, dodo_name);
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("dodo_storage", data, abi_serializer_max_time);
   }

   fc::variant get_oracle_table(const extended_symbol& basetoken, const extended_symbol& quotetoken) {
      uint64_t    key = get_hash_key(get_checksum256(
          basetoken.contract.to_uint64_t(), basetoken.sym.value(), quotetoken.contract.to_uint64_t(),
          quotetoken.sym.value()));
      const auto& db  = control->db();
      const auto* tbl = db.find<table_id_object, by_code_scope_table>(
          boost::make_tuple(N(eosdoseosdos), N(eosdoseosdos), N(oracleprices)));
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
      return data.empty() ? fc::variant() : abi_ser.binary_to_variant("oracle_prices", data, abi_serializer_max_time);
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

   uint256m to_wei(uint256m value) { return value * pow(10, TOKEN_DECIMALS); }

   extended_asset to_pl_asset(int64_t value, const std::string& sym, name lp_contract_name) {
      return extended_asset{asset{value, symbol{TOKEN_DECIMALS, sym.c_str()}}, lp_contract_name};
   }

   extended_symbol to_lp_esym(const std::string& sym, name lp_contract_name) {
      return extended_symbol{symbol{TOKEN_DECIMALS, sym.c_str()}, lp_contract_name};
   }

   symbol to_lp_sym(const std::string& sym) { return symbol{TOKEN_DECIMALS, sym.c_str()}; }

   extended_asset to_asset(int64_t value, const std::string& sym) {
      return extended_asset{asset{value, symbol{TOKEN_DECIMALS, sym.c_str()}}, name{"roxe.ro"}};
   }
   extended_asset to_wei_asset(uint256m value, const std::string& sym) {
      return to_asset(static_cast<int64_t>(to_wei(value)), sym);
   }

   extended_symbol get_core_symbol() { return extended_symbol{symbol{CORE_SYM}, name{"eosio.token"}}; }

   symbol to_sym_from_string(const std::string& sym) { return symbol{TOKEN_DECIMALS, sym.c_str()}; }

   extended_symbol to_sym(const std::string& sym) {
      return extended_symbol{symbol{TOKEN_DECIMALS, sym.c_str()}, name{"roxe.ro"}};
   }

   extended_asset to_maximum_supply(const std::string& sym) {
      return extended_asset{asset{1000000000000000, symbol{TOKEN_DECIMALS, sym.c_str()}}, name{"roxe.ro"}};
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
      std::vector<std::string> tokens{"WETH", "DAI", "MKR"};
      LINE_DEBUG;
      for (auto token : tokens) {
         newtoken(tokenissuer, to_maximum_supply(token));
      }
   }

   void mintBefore() {
      mint(lp, to_wei_asset(1000, "MKR"));
      LINE_DEBUG;

      mint(trader, to_wei_asset(1000, "MKR"));
      LINE_DEBUG;
   }

   void setPriceBaseBefore() { setprice(admin, to_sym("WETH"), to_wei_asset(100, "MKR")); }

   void setPriceQuoteBefore() { setprice(admin, to_sym("MKR"), to_asset(100, "WETH")); }

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

   void enableBaseBefore() {
      LINE_DEBUG;
      name dodo_name = dodo_ethbase_name;
      setparameter(admin, dodo_name, N(trading), 1);
      setparameter(admin, dodo_name, N(quotedeposit), 1);
      setparameter(admin, dodo_name, N(basedeposit), 1);
   }

   void enableQuoteBefore() {
      LINE_DEBUG;
      name dodo_name = dodo_ethquote_name;
      setparameter(admin, dodo_name, N(trading), 1);
      setparameter(admin, dodo_name, N(quotedeposit), 1);
      setparameter(admin, dodo_name, N(basedeposit), 1);
   }

   void depositQuoteBefore() {
      LINE_DEBUG;
      depositquote(lp, dodo_ethbase_name, to_wei_asset(1000, "MKR"));
   }

   void depositBaseBefore() {
      LINE_DEBUG;
      depositbase(lp, dodo_ethquote_name, to_wei_asset(1000, "MKR"));
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

   void setPriceStableBefore() { setprice(admin, to_sym("DAI"), to_wei_asset(1, "MKR")); }

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

   void enableStableBefore() {
      LINE_DEBUG;
      name dodo_name = dodo_stablecoin_name;
      setparameter(admin, dodo_name, N(trading), 1);
      setparameter(admin, dodo_name, N(quotedeposit), 1);
      setparameter(admin, dodo_name, N(basedeposit), 1);
   }

   void depositQuoteStableBefore() {
      LINE_DEBUG;
      depositquote(lp, dodo_stablecoin_name, to_wei_asset(10000, "MKR"));
   }

   void depositBaseStableBefore() {
      LINE_DEBUG;
      depositbase(lp, dodo_stablecoin_name, to_wei_asset(10000, "DAI"));
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

   void stableCoinBefore2() {
      std::vector<name>        users{lp, trader};
      std::vector<std::string> tokens{"USD", "GBP"};
      LINE_DEBUG;
      for (auto token : tokens) {
         newtoken(tokenissuer, to_maximum_supply(token));
         for (auto user : users) {
            mint(user, to_wei_asset(10000000, token));
         }
      }

      setprice(admin, to_sym("USD"), to_asset(740000, "GBP"));

      LINE_DEBUG;
      name            msg_sender    = admin;
      name            dodo_name     = dodo_stablecoin_name;
      address         maintainer    = doowner;
      extended_symbol baseToken     = to_sym("USD");
      extended_symbol quoteToken    = to_sym("GBP");
      extended_symbol oracle        = to_sym("USD");
      uint64_t        lpFeeRate     = 595;
      uint64_t        mtFeeRate     = 105;
      uint64_t        k             = 1;
      uint64_t        gasPriceLimit = 0; // gweiStr("100")
                                         //   lpFeeRate: decimalStr("0.0001"),
                                         //   mtFeeRate: decimalStr("0"),
                                         //   k: gweiStr("1"), // nearly zero
                                         //   gasPriceLimit: gweiStr("100"),

      LINE_DEBUG;
      breeddodo(
          msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);

      LINE_DEBUG;
      std::vector<name> enableparas{N(trading), N(quotedeposit), N(basedeposit)};
      for (auto para : enableparas) {
         setparameter(admin, dodo_name, para, 1);
      }

      LINE_DEBUG;
      depositbase(lp, dodo_stablecoin_name, to_wei_asset(1000000, "USD"));
      LINE_DEBUG;
      depositquote(lp, dodo_stablecoin_name, to_wei_asset(740000, "GBP"));
   }

   void stableCoinBefore3() {
      std::vector<name>        users{lp, trader};
      std::vector<std::string> tokens{"USD", "GBP"};
      LINE_DEBUG;
      for (auto token : tokens) {
         newtoken(tokenissuer, to_maximum_supply(token));
         for (auto user : users) {
            mint(user, to_wei_asset(10000000, token));
         }
      }

      setprice(admin, to_sym("USD"), to_asset(740000, "GBP"));

      LINE_DEBUG;
      name            msg_sender    = admin;
      name            dodo_name     = dodo_stablecoin_name;
      address         maintainer    = doowner;
      extended_symbol baseToken     = to_sym("USD");
      extended_symbol quoteToken    = to_sym("GBP");
      extended_symbol oracle        = to_sym("USD");
      uint64_t        lpFeeRate     = 595;
      uint64_t        mtFeeRate     = 105;
      uint64_t        k             = 100;
      uint64_t        gasPriceLimit = 0; // gweiStr("100")
                                         //   lpFeeRate: decimalStr("0.0001"),
                                         //   mtFeeRate: decimalStr("0"),
                                         //   k: gweiStr("1"), // nearly zero
                                         //   gasPriceLimit: gweiStr("100"),

      LINE_DEBUG;
      breeddodo(
          msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);

      LINE_DEBUG;
      std::vector<name> enableparas{N(trading), N(quotedeposit), N(basedeposit)};
      for (auto para : enableparas) {
         setparameter(admin, dodo_name, para, 1);
      }

      LINE_DEBUG;
      depositbase(lp, dodo_stablecoin_name, to_wei_asset(1000000, "USD"));
      LINE_DEBUG;
      depositquote(lp, dodo_stablecoin_name, to_wei_asset(740000, "GBP"));
   }

   void check_dodos(name dodo_name) {
#ifdef EOSDOS_DEBUG
      LINE_DEBUG;
      auto store = dodos(dodo_name);
      BOOST_TEST_CHECK(nullptr == store);
#endif
   }

   void check_balance(std::string token_name, std::string amount) {
      auto sym = to_sym_from_string(token_name);
      auto c   = eosio::chain::asset::from_string(amount + " " + token_name);
      auto b   = get_balancex(trader, sym);
      BOOST_TEST_CHECK(c == b);
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
         setparametera(p.first, p.second);
      }
   }

   bool is_auth_token;

   name           admin;
   name           doowner;
   name           tokenissuer;
   name           maintainer;
   name           lp;
   name           trader;
   name           dodo_ethbase_name;
   name           dodo_ethquote_name;
   name           dodo_stablecoin_name;
   abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eosdos_tests)
////////////////dodo admin////////////////////

BOOST_FIXTURE_TEST_CASE(setadmin_tests, eosdos_tester) try {
   stableCoinBefore();
   name dodo_name = dodo_stablecoin_name;
   setadmin(admin, dodo_name, N(supervisor), N(alice));
   setadmin(admin, dodo_name, N(maintainer), N(bob));
   auto store = dodos(dodo_name);
   //    BOOST_TEST_CHECK(nullptr==store);
   BOOST_TEST_CHECK("alice" == store["_SUPERVISOR_"].as_string());
   BOOST_TEST_CHECK("bob" == store["_MAINTAINER_"].as_string());
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(setparameter_tests, eosdos_tester) try {
   stableCoinBefore();
   name dodo_name = dodo_stablecoin_name;
   setparameter(admin, dodo_name, N(k), 100);
   setparameter(admin, dodo_name, N(lpfeerate), 3);
   setparameter(admin, dodo_name, N(mtfeerate), 2);
   setparameter(admin, dodo_name, N(trading), 1);
   setparameter(admin, dodo_name, N(quotedeposit), 1);
   setparameter(admin, dodo_name, N(basedeposit), 1);
   setparameter(admin, dodo_name, N(buying), 1);
   setparameter(admin, dodo_name, N(selling), 1);
   setparameter(admin, dodo_name, N(basebalimit), 4);
   setparameter(admin, dodo_name, N(quotebalimit), 5);
   auto store = dodos(dodo_name);
   //    BOOST_TEST_CHECK(nullptr==store);

   BOOST_TEST_CHECK(3 == store["_LP_FEE_RATE_"].as<uint64_t>());
   BOOST_TEST_CHECK(2 == store["_MT_FEE_RATE_"].as<uint64_t>());
   BOOST_TEST_CHECK(100 == store["_K_"].as<uint64_t>());
   BOOST_TEST_CHECK(true == store["_TRADE_ALLOWED_"].as<bool>());
   BOOST_TEST_CHECK(true == store["_DEPOSIT_QUOTE_ALLOWED_"].as<bool>());
   BOOST_TEST_CHECK(true == store["_DEPOSIT_BASE_ALLOWED_"].as<bool>());
   BOOST_TEST_CHECK(true == store["_BUYING_ALLOWED_"].as<bool>());
   BOOST_TEST_CHECK(true == store["_SELLING_ALLOWED_"].as<bool>());
   BOOST_TEST_CHECK(4 == store["_BASE_BALANCE_LIMIT_"].as<uint64_t>());
   BOOST_TEST_CHECK(5 == store["_QUOTE_BALANCE_LIMIT_"].as<uint64_t>());
}
FC_LOG_AND_RETHROW()

/////////////////////admin dodo///////////////////////
BOOST_FIXTURE_TEST_CASE(enable_trading_tests, eosdos_tester) try {
   newTokenBefore();
   mintStableBefore();

   setPriceStableBefore();
   breedStableBefore();
   LINE_DEBUG;
   name dodo_name = dodo_stablecoin_name;
   //   setparameter(admin, dodo_name,N(trading),1);
   setparameter(admin, dodo_name, N(quotedeposit), 1);
   setparameter(admin, dodo_name, N(basedeposit), 1);

   depositBaseStableBefore();
   depositQuoteStableBefore();

   //    buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "DAI"), to_wei_asset(1001, "MKR"));

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("TRADE_NOT_ALLOWED"),
       buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "DAI"), to_wei_asset(1001, "MKR")));
}
FC_LOG_AND_RETHROW()

/////////////////////admin dodo///////////////////////
BOOST_FIXTURE_TEST_CASE(no_token_trading_tests, eosdos_tester) try {
   stableCoinBefore();

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("no base token symbol in the pair"),
       buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "WETH"), to_wei_asset(1001, "MKR")));

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("no quote token symbol in the pair"),
       buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "DAI"), to_wei_asset(1001, "WETH")));

   auto base_with_dec_one = extended_asset{asset{1000, symbol{TOKEN_DECIMALS + 1, "DAI"}}, name{"roxe.ro"}};

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("mismatch precision of the base token in the pair"),
       buybasetoken(trader, dodo_stablecoin_name, base_with_dec_one, to_wei_asset(1001, "MKR")));

   auto quote_with_dec_one = extended_asset{asset{1000, symbol{TOKEN_DECIMALS + 1, "MKR"}}, name{"roxe.ro"}};

   BOOST_REQUIRE_EQUAL(
       wasm_assert_msg("mismatch precision of the quote token in the pair"),
       buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "DAI"), quote_with_dec_one));
}
FC_LOG_AND_RETHROW()

/////////////////////dodo///////////////////////
BOOST_FIXTURE_TEST_CASE(buy_tiny_base_token_tests, eosdos_tester) try {
   stableCoinBefore();
   buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1, "DAI"), to_wei_asset(1001, "MKR"));

   check_balance("DAI", "10001.000000");
   check_balance("MKR", "9998.999999");

   sellbastoken(trader, dodo_stablecoin_name, to_wei_asset(1, "DAI"), to_asset(1, "MKR"));
   check_balance("DAI", "10000.000000");
   check_balance("MKR", "10000.999998");
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(simulation_formula_tests, eosdos_tester) try {
   std::vector<name>        users{lp, trader};
   std::vector<std::string> tokens{"USD", "GBP"};
   LINE_DEBUG;
   for (auto token : tokens) {
      newtoken(tokenissuer, to_maximum_supply(token));
      for (auto user : users) {
         mint(user, to_wei_asset(10000000, token));
      }
   }

   setprice(admin, to_sym("USD"), to_asset(740000, "GBP"));

   LINE_DEBUG;
   name            msg_sender    = admin;
   name            dodo_name     = dodo_stablecoin_name;
   address         maintainer    = doowner;
   extended_symbol baseToken     = to_sym("USD");
   extended_symbol quoteToken    = to_sym("GBP");
   extended_symbol oracle        = to_sym("USD");
   uint64_t        lpFeeRate     = 595;
   uint64_t        mtFeeRate     = 105;
   uint64_t        k             = 1;
   uint64_t        gasPriceLimit = 0; // gweiStr("100")
                                      //   lpFeeRate: decimalStr("0.0001"),
                                      //   mtFeeRate: decimalStr("0"),
                                      //   k: gweiStr("1"), // nearly zero
                                      //   gasPriceLimit: gweiStr("100"),

   LINE_DEBUG;
   breeddodo(msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);

   LINE_DEBUG;
   std::vector<name> enableparas{N(trading), N(quotedeposit), N(basedeposit)};
   for (auto para : enableparas) {
      setparameter(admin, dodo_name, para, 1);
   }

   LINE_DEBUG;
   depositquote(lp, dodo_name, to_wei_asset(740000, "GBP"));

   LINE_DEBUG;
   depositbase(lp, dodo_name, to_wei_asset(1000000, "USD"));

   LINE_DEBUG;
   depositquote(lp, dodo_name, to_wei_asset(740000, "GBP"));

   LINE_DEBUG;
   depositquote(lp, dodo_name, to_wei_asset(740000, "GBP"));

   LINE_DEBUG;
   withdrawquote(lp, dodo_name, to_wei_asset(740000, "GBP"));
   LINE_DEBUG;
   withdrawquote(lp, dodo_name, to_wei_asset(740000, "GBP"));
   //    {0, 1000000, 100000000}, {1, 1000000, 1000000},   {1, 1000000, 10000},
   std::vector<std::vector<int64_t>> testpara{
       {0, 1000000, 100000000}, {1, 1000000, 100000},    {1, 1000000, 10000},
       {1, 100000000, 10000},   {0, 50000000, 50000000}, {0, 1287998, 10000000},
       {0, 1287869, 10000000},  {0, 1288385, 10000000},  {0, 1294179, 10000000},
   };

   std::vector<std::vector<int64_t>> testpara1{
       {1, 998500000, 728900000},  {0, 1323603731, 999000000}, {0, 1000000000, 1000000000}, {0, 1323603731, 1000000000},
       {0, 50000000, 50000000},    {0, 50000000, 40000000},    {0, 312220000, 233000000},   {0, 1340943400, 999000000},
       {0, 1340943393, 999000000}, {1, 998500000, 728900000},  {1, 998500000, 728900000},   {1, 998500000, 728900000},
       {1, 98500000, 72830000},
   };

   LINE_DEBUG;

   auto testbuysell = [&](auto testparas) {
      for (auto p : testparas) {
         check_dodos(dodo_name);
         if (0 == p[0]) {
            buybasetoken(trader, dodo_name, to_asset(p[1], "USD"), to_asset(p[2], "GBP"));
         } else {
            sellbastoken(trader, dodo_name, to_asset(p[1], "USD"), to_asset(p[2], "GBP"));
         }
         check_dodos(dodo_name);
         check_balance("USD", "10001.000000");
         check_balance("GBP", "9998.999999");
      }
   };

   testbuysell(testpara);
   setparameter(admin, dodo_name, N(lpfeerate), 595);
   setparameter(admin, dodo_name, N(mtfeerate), 105);
   testbuysell(testpara1);

   //    setparameter(admin, dodo_name, N(k), 100);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(buy1_base_token_formula_tests, eosdos_tester) try {
   stableCoinBefore3();
   name dodo_name = dodo_stablecoin_name;

   check_dodos(dodo_name);

   buybasetoken(trader, dodo_name, to_asset(1000000, "USD"), to_asset(999000000, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10001.000000");
   check_balance("GBP", "9998.999999");

   sellbastoken(trader, dodo_name, to_asset(1000000, "USD"), to_asset(10, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10000.000000");
   check_balance("GBP", "10000.999998");
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(transfer_fee_tests, eosdos_tester) try {
   stableCoinBefore3();
   initparam();
   name dodo_name = dodo_stablecoin_name;

   check_dodos(dodo_name);

   buybasetoken(trader, dodo_name, to_asset(1000000, "USD"), to_asset(999000000, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10001.000000");
   check_balance("GBP", "9998.999999");

   sellbastoken(trader, dodo_name, to_asset(1000000, "USD"), to_asset(10, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10000.000000");
   check_balance("GBP", "10000.999998");
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(buysell_base_testparam_formula_tests, eosdos_tester) try {
   stableCoinBefore3();
   name                 dodo_name = dodo_stablecoin_name;
   std::vector<int64_t> params    = {1, 994984389360, 739567295754, 699279538448, 958398324877};

   // _R_STATUS_: 1,
   // _TARGET_BASE_TOKEN_AMOUNT_: '994984389360',
   // _TARGET_QUOTE_TOKEN_AMOUNT_: '739567295754',
   // _BASE_BALANCE_: '699279538448',
   // _QUOTE_BALANCE_: '958398324877'

   check_dodos(dodo_name);
   check_balance("USD", "10000.000000");
   check_balance("GBP", "10000.999998");
   buybasetest(trader, dodo_name, to_asset(698649560000, "USD"), to_wei_asset(99999999, "GBP"), params);
   //    sellbasetest(trader, dodo_name, to_asset(199998500000, "USD"), to_asset(147858890000, "GBP"),params);
   //    sellbasetest(trader, dodo_name, to_asset(698649560000, "USD"), to_asset(0, "GBP"),params);
   check_dodos(dodo_name);
   check_balance("USD", "10000.000000");
   check_balance("GBP", "10000.999998");
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(buy_base_token_formula_tests, eosdos_tester) try {
   stableCoinBefore2();
   name dodo_name = dodo_stablecoin_name;
   check_dodos(dodo_name);

   buybasetoken(trader, dodo_name, to_asset(1323603731, "USD"), to_asset(999000000, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10001.000000");
   check_balance("GBP", "9998.999999");

   sellbastoken(trader, dodo_name, to_asset(998500000, "USD"), to_asset(728900000, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10000.000000");
   check_balance("GBP", "10000.999998");

   buybasetoken(trader, dodo_name, to_asset(1323603731, "USD"), to_asset(999000000, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10001.000000");
   check_balance("GBP", "9998.999999");

   //    int i = 0;
   //    for (i = 0; i < 10; ++i) {
   //       BOOST_TEST_CHECK(i == -1);
   //       {
   //          LINE_DEBUG;
   //          auto store = dodos(dodo_name);
   //          BOOST_TEST_CHECK(nullptr == store);
   //       }
   //       buybasetoken(trader, dodo_name, to_asset(1323603731, "USD"), to_asset(999000000, "GBP"));
   //       {
   //          LINE_DEBUG;
   //          auto store = dodos(dodo_name);
   //          BOOST_TEST_CHECK(nullptr == store);
   //       }
   //       check_balance("USD", "10001.000000");
   //       check_balance("GBP", "9998.999999");
   //    }
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(sell_base_token_formula_tests, eosdos_tester) try {
   stableCoinBefore2();
   name dodo_name = dodo_stablecoin_name;
   check_dodos(dodo_name);
   sellbastoken(trader, dodo_name, to_asset(998500000, "USD"), to_asset(728900000, "GBP"));
   check_balance("USD", "10000.000000");
   check_balance("GBP", "10000.999998");
   check_dodos(dodo_name);
   buybasetoken(trader, dodo_name, to_asset(1323603731, "USD"), to_asset(999000000, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10001.000000");
   check_balance("GBP", "9998.999999");

   sellbastoken(trader, dodo_name, to_asset(998500000, "USD"), to_asset(728900000, "GBP"));
   check_dodos(dodo_name);
   check_balance("USD", "10000.000000");
   check_balance("GBP", "10000.999998");

   //    int i = 0;
   //    for (i = 0; i < 20; ++i) {
   //       BOOST_TEST_CHECK(i == -1);
   //       {
   //          LINE_DEBUG;
   //          auto store = dodos(dodo_name);
   //          BOOST_TEST_CHECK(nullptr == store);
   //       }
   //       sellbastoken(trader, dodo_name, to_asset(99850000000, "USD"), to_asset(728900000, "GBP"));
   //       {
   //          LINE_DEBUG;
   //          auto store = dodos(dodo_name);
   //          BOOST_TEST_CHECK(nullptr == store);
   //       }

   //       check_balance("USD", "10000.000000");
   //       check_balance("GBP", "10000.999998");
   //    }
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(buy_base_token_tests, eosdos_tester) try {
   stableCoinBefore();
   buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "DAI"), to_wei_asset(1001, "MKR"));

   check_balance("DAI", "11000.000000");
   check_balance("MKR", "8999.999000");

   buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(8906, "DAI"), to_wei_asset(9000, "MKR"));
   check_balance("DAI", "19906.000000");
   check_balance("MKR", "93.152930");

   sellbastoken(trader, dodo_stablecoin_name, to_wei_asset(19900, "DAI"), to_asset(19970, "MKR"));
   check_balance("DAI", "6.000000");
   check_balance("MKR", "19996.954019");
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
   check_balance("DAI", "0.000000");
   check_balance("MKR", "19999.980001");

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
   check_balance("DAI", "19990.000000");
   check_balance("MKR", "10000.009991");

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
   check_balance("DAI", "19900.000000");
   check_balance("MKR", "99.010000");

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
   uint64_t mint_amount = 10000;
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
   BOOST_TEST_CHECK("8999999" == store["_BASE_BALANCE_"].as_string());
   std::string quote_token_name = "MKR";
   auto        sym              = to_sym_from_string(quote_token_name);
   auto        c                = eosio::chain::asset::from_string("899.999700 " + quote_token_name);
   auto        b                = get_balancex(trader, sym);
   BOOST_TEST_CHECK(c == b);
   // 898581839502056240973
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(sell_eth_to_token_tests, eosdos_tester) try {
   ethBaseBefore();
   sellethtoken(trader, to_wei_asset(1, "WETH"), to_wei_asset(50, "MKR"));
   auto store = dodos(dodo_ethbase_name);
   BOOST_TEST_CHECK("11000000" == store["_BASE_BALANCE_"].as_string());

   std::string token_name = "MKR";
   auto        sym        = to_sym_from_string(token_name);
   auto        c          = eosio::chain::asset::from_string("1099.999690 " + token_name);
   auto        b          = get_balancex(trader, sym);
   BOOST_TEST_CHECK(c == b);
   // 1098617454226610630663
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(withdraw_eth_as_base_tests, eosdos_tester) try {
   ethBaseBefore();
   withdraweab(lp, to_wei_asset(5, "WETH"), to_sym("MKR"));
   std::string token_name = "WETH";
   //    auto        sym        = to_lp_esym(token_name, dodo_ethbase_name);
   //    auto        c          = eosio::chain::asset::from_string("5.000000 " + token_name);
   //    auto        b          = get_balancex(lp, sym.sym, sym.contract);
   //    BOOST_TEST_CHECK(c == b);
   const auto b = get_account(lp, LP_TOKEN_DECIMALS_STR + token_name, dodo_ethbase_name);
   auto       c = "5.000000 " + token_name;
   BOOST_REQUIRE_EQUAL(c, b);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(withdraw_all_eth_as_base_tests, eosdos_tester) try {
   ethBaseBefore();
   withdrawaeab(lp, to_sym("MKR"));
   std::string token_name = "WETH";
   //    auto        sym        = to_lp_esym(token_name, dodo_ethbase_name);
   //    auto        c          = eosio::chain::asset::from_string("0.000000 " + token_name);
   //    auto        b          = get_balancex(lp, sym.sym, sym.contract);
   //    BOOST_TEST_CHECK(c == b);
   const auto b = get_account(lp, LP_TOKEN_DECIMALS_STR + token_name, dodo_ethbase_name);
   auto       c = "0.000000 " + token_name;
   BOOST_REQUIRE_EQUAL(c, b);
}
FC_LOG_AND_RETHROW()

////////////////proxy eth quote////////////////////
BOOST_FIXTURE_TEST_CASE(buy_token_with_eth_tests, eosdos_tester) try {
   ethQuoteBefore();

   buytokeneth(trader, to_wei_asset(200, "MKR"), to_asset(21000, "WETH"));

   auto store = dodos(dodo_ethquote_name);
   BOOST_TEST_CHECK("10020000" == store["_QUOTE_BALANCE_"].as_string());
   std::string quote_token_name = "MKR";
   auto        sym              = to_sym_from_string(quote_token_name);
   auto        c                = eosio::chain::asset::from_string("1200.000000 " + quote_token_name);
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
   BOOST_TEST_CHECK("9995000" == store["_QUOTE_BALANCE_"].as_string());

   std::string token_name = "MKR";
   auto        sym        = to_sym_from_string(token_name);
   auto        c          = eosio::chain::asset::from_string("950.000000 " + token_name);
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
   //    auto        sym        = to_lp_esym(token_name, dodo_ethquote_name);
   //    auto        c          = eosio::chain::asset::from_string("5.000000 " + token_name);
   //    auto        b          = get_balancex(lp, sym.sym, sym.contract);
   //    BOOST_TEST_CHECK(c == b);
   const auto b = get_account(lp, LP_TOKEN_DECIMALS_STR + token_name, dodo_ethquote_name);
   auto       c = "5.000000 " + token_name;
   BOOST_REQUIRE_EQUAL(c, b);
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
   LINE_DEBUG;
   withdrawaeaq(lp, to_sym("MKR"));
   LINE_DEBUG;
   std::string token_name = "WETH";
   //    auto        sym        = to_lp_esym(token_name, dodo_ethquote_name);
   //    auto        c          = eosio::chain::asset::from_string("0.000000 " + token_name);
   //    auto        b          = get_balancex(lp, sym.sym, sym.contract);
   //    BOOST_TEST_CHECK(c == b);
   const auto b = get_account(lp, LP_TOKEN_DECIMALS_STR + token_name, dodo_ethquote_name);
   auto       c = "0.000000 " + token_name;
   BOOST_REQUIRE_EQUAL(c, b);
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
   setprice(admin, to_sym("WETH"), ea);
   setprice(admin, to_sym("DAI"), to_wei_asset(10, "MKR"));
   setprice(admin, to_sym("MKR"), to_wei_asset(20, "WETH"));
   auto bb = get_oracle_table(to_sym("WETH"), to_sym("MKR"));
   {
      auto a = TOKEN_DECIMALS_STR + "WETH"; // to_sym("WETH");
      auto b = bb["basetoken"]["sym"].as_string();
      BOOST_REQUIRE_EQUAL(b, a);
   }
   {
      auto a = "5.000000 MKR"; // to_sym("WETH");
      auto b = bb["quotetoken"]["quantity"].as_string();
      BOOST_REQUIRE_EQUAL(b, a);
   }

   moveoracle(admin);
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
   auto        a                = eosio::chain::asset::from_string("5.000000 " + quote_token_name);
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

   BOOST_REQUIRE_EQUAL(eosio::chain::asset::from_string("0.000001 " + token_name), get_balancex(maintainer, sym));
}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
