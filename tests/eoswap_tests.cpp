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

using mvo = fc::mutable_variant_object;
using uint_eth = uint64_t;

class eoswap_tester : public tester {
public:
  eoswap_tester() {
    produce_blocks(2);

    create_accounts({N(alice), N(bob), N(carol), N(eoswapeoswap), N(pool)});
    produce_blocks(2);

    set_code(N(eoswapeoswap), contracts::swap_wasm());
    set_abi(N(eoswapeoswap), contracts::swap_abi().data());

    produce_blocks();

    const auto &accnt =
        control->db().get<account_object, by_name>(N(eoswapeoswap));
    abi_def abi;
    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
    abi_ser.set_abi(abi, abi_serializer_max_time);
  }

  action_result push_action(const account_name &signer, const action_name &name,
                            const variant_object &data) {
    string action_type_name = abi_ser.get_action_type(name);

    action act;
    act.account = N(eoswapeoswap);
    act.name = name;
    act.data = abi_ser.variant_to_binary(action_type_name, data,
                                         abi_serializer_max_time);

    return base_tester::push_action(std::move(act), signer.to_uint64_t());
  }
  /////////////factory///////////
  action_result setblabs(account_name msg_sender, account_name blabs) {
    return push_action(msg_sender, N(setblabs),
                       mvo()("msg_sender", msg_sender)("blabs", blabs));
  }

  action_result collect(account_name msg_sender, account_name pool_name) {
    return push_action(msg_sender, N(collect),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name));
  }

  action_result newpool(account_name msg_sender, account_name pool_name) {
    return push_action(msg_sender, N(newpool),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name));
  }

  ///////////////pool ////////////////////
  // msg_sender, name dst, uint_eth amt
  action_result setswapfee(account_name msg_sender, account_name pool_name,
                           uint_eth swapFee) {
    return push_action(msg_sender, N(setswapfee),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "swapFee", swapFee));
  }

  action_result setcontroler(account_name msg_sender, account_name pool_name,
                             name manager) {
    return push_action(msg_sender, N(setcontroler),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "manager", manager));
  }

  action_result setpubswap(account_name msg_sender, account_name pool_name,
                           bool public_) {
    return push_action(msg_sender, N(setpubswap),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "public_", public_));
  }

  action_result finalize(account_name msg_sender, account_name pool_name) {
    return push_action(msg_sender, N(finalize),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name));
  }

  action_result bind(account_name msg_sender, account_name pool_name,
                     account_name token, uint_eth balance, uint_eth denorm) {
    return push_action(
        msg_sender, N(bind),
        mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token)(
            "balance", balance)("denorm", denorm));
  }

  action_result rebind(account_name msg_sender, account_name pool_name,
                       account_name token, uint_eth balance, uint_eth denorm) {
    return push_action(
        msg_sender, N(rebind),
        mvo()("msg_sender", msg_sender)("pool_name", pool_name)("token", token)(
            "balance", balance)("denorm", denorm));
  }

  action_result unbind(account_name msg_sender, account_name pool_name,
                       account_name token) {
    return push_action(msg_sender, N(unbind),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "token", token));
  }

  action_result gulp(account_name msg_sender, account_name pool_name,
                     account_name token) {
    return push_action(msg_sender, N(gulp),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "token", token));
  }

  action_result joinpool(account_name msg_sender, account_name pool_name,
                         uint_eth poolAmountOut,
                         std::vector<uint_eth> maxAmountsIn) {
    return push_action(
        msg_sender, N(joinpool),
        mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
            "poolAmountOut", poolAmountOut)("maxAmountsIn", maxAmountsIn));
  }

  action_result exitpool(account_name msg_sender, account_name pool_name,
                         uint_eth poolAmountIn,
                         std::vector<uint_eth> minAmountsOut) {
    return push_action(
        msg_sender, N(exitpool),
        mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
            "poolAmountIn", poolAmountIn)("minAmountsOut", minAmountsOut));
  }

  ////////////////pool TOKEN//////////////

  action_result approve(account_name msg_sender, account_name pool_name,
                        account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(approve),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "dst", dst)("amt", amt));
  }

  action_result transfer(account_name msg_sender, account_name pool_name,
                         account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(transfer),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "dst", dst)("amt", amt));
  }

  action_result transferfrom(account_name msg_sender, account_name pool_name,
                             account_name src, account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(transferfrom),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "src", src)("dst", dst)("amt", amt));
  }

  action_result incapproval(account_name msg_sender, account_name pool_name,
                            account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(incapproval),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "dst", dst)("amt", amt));
  }

  action_result decapproval(account_name msg_sender, account_name pool_name,
                            account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(decapproval),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "dst", dst)("amt", amt));
  }

  action_result mint(account_name msg_sender, account_name pool_name,
                     uint_eth amt) {
    return push_action(
        msg_sender, N(mint),
        mvo()("msg_sender", msg_sender)("pool_name", pool_name)("amt", amt));
  }

  action_result burn(account_name msg_sender, account_name pool_name,
                     uint_eth amt) {
    return push_action(
        msg_sender, N(burn),
        mvo()("msg_sender", msg_sender)("pool_name", pool_name)("amt", amt));
  }

  action_result move(account_name pool_name, account_name src, name dst,
                     uint_eth amt) {
    return push_action(
        src, N(move),
        mvo()("pool_name", pool_name)("src", src)("dst", dst)("amt", amt));
  }

  ////////////////TEST TOKEN//////////////

  action_result tmint(account_name msg_sender, account_name token,
                      uint_eth amt) {
    return push_action(
        msg_sender, N(tmint),
        mvo()("msg_sender", msg_sender)("token", token)("amt", amt));
  }

  action_result tmove(account_name token, account_name src, name dst,
                      uint_eth amt) {
    return push_action(
        src, N(tmove),
        mvo()("token", token)("src", src)("dst", dst)("amt", amt));
  }

  action_result ttransfer(account_name msg_sender, account_name token,
                          account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(ttransfer),
                       mvo()("msg_sender", msg_sender)("token", token)(
                           "dst", dst)("amt", amt));
  }

  action_result ttransferfrm(account_name msg_sender, account_name token,
                             account_name src, account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(ttransferfrm),
                       mvo()("msg_sender", msg_sender)("token", token)(
                           "src", src)("dst", dst)("amt", amt));
  }

  //  action_result approve(account_name msg_sender, account_name token,
  //                         account_name dst, uint_eth amt) {
  //     return push_action(msg_sender, N(approve),
  //                        mvo()("msg_sender", msg_sender)("token", token)(
  //                            "dst", dst)("amt", amt));
  //   }

  //   action_result incapproval(account_name msg_sender, account_name token,
  //                             account_name dst, uint_eth amt) {
  //     return push_action(msg_sender, N(incapproval),
  //                        mvo()("msg_sender", msg_sender)("token", token)(
  //                            "dst", dst)("amt", amt));
  //   }

  //   action_result decapproval(account_name msg_sender, account_name token,
  //                             account_name dst, uint_eth amt) {
  //     return push_action(msg_sender, N(decapproval),
  //                        mvo()("msg_sender", msg_sender)("token", token)(
  //                            "dst", dst)("amt", amt));
  //   }

  //   action_result burn(account_name msg_sender, account_name token,
  //                      uint_eth amt) {
  //     return push_action(
  //         msg_sender, N(burn),
  //         mvo()("msg_sender", msg_sender)("token", token)("amt", amt));
  //   }

  ////////////////get table//////////////

  fc::variant get_factory_store() {
    vector<char> data = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap),
                                           N(factorystore), N(factorystore));
    if (data.empty())
      std::cout << "\nData is empty\n" << std::endl;
    return data.empty() ? fc::variant()
                        : abi_ser.binary_to_variant("BFactoryStorage", data,
                                                    abi_serializer_max_time);
  }

  fc::variant get_pool_store() {
    vector<char> data = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap),
                                           N(poolstore), N(poolstore));
    if (data.empty())
      std::cout << "\nData is empty\n" << std::endl;
    return data.empty() ? fc::variant()
                        : abi_ser.binary_to_variant("BPoolStorage", data,
                                                    abi_serializer_max_time);
  }

  fc::variant get_token_store() {
    vector<char> data = get_row_by_account(N(eoswapeoswap), N(eoswapeoswap),
                                           N(tokenstore), N(tokenstore));
    if (data.empty())
      std::cout << "\nData is empty\n" << std::endl;
    return data.empty() ? fc::variant()
                        : abi_ser.binary_to_variant("BTokenStorage", data,
                                                    abi_serializer_max_time);
  }

  void to_kv_helper(
      const fc::variant &v,
      std::function<void(const std::string &, const std::string &)> &&append) {
    if (v.is_object()) {
      const auto &obj = v.get_object();
      static const std::string sep = ".";

      for (const auto &entry : obj) {
        to_kv_helper(entry.value(),
                     [&append, &entry](const std::string &path,
                                       const std::string &value) {
                       append(sep + entry.key() + path, value);
                     });
      }
    } else if (v.is_array()) {
      const auto &arr = v.get_array();
      for (size_t idx = 0; idx < arr.size(); idx++) {
        const auto &entry = arr.at(idx);
        to_kv_helper(entry, [&append, idx](const std::string &path,
                                           const std::string &value) {
          append(std::string("[") + std::to_string(idx) + std::string("]") +
                     path,
                 value);
        });
      }
    } else if (!v.is_null()) {
      append("", v.as_string());
    }
  }

  auto to_kv(const fc::variant &v) {
    std::map<std::string, std::string> result;
    to_kv_helper(v, [&result](const std::string &k, const std::string &v) {
      result.emplace(k, v);
    });
    return result;
  }

  void array_to_kv_helper(
      const fc::variant &v,
      std::function<void(const std::string &, const std::string &)> &&append) {
    if (v.is_array()) {
      const auto &arr = v.get_array();
      for (size_t idx = 0; idx < arr.size(); idx++) {
        const auto &entry = arr.at(idx);
        append(entry["key"].as_string(), entry["value"].as_string());
      }
    }
  }

  auto array_to_kv(const fc::variant &v) {
    std::map<std::string, std::string> result;
    array_to_kv_helper(v,
                       [&result](const std::string &k, const std::string &v) {
                         result.emplace(k, v);
                       });
    return result;
  }

  uint_eth to_wei(uint_eth value) { return value * pow(10, 6); }

  abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eoswap_tests)
////////////////factory////////////////////
BOOST_FIXTURE_TEST_CASE(newpoool_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
}
FC_LOG_AND_RETHROW()
////////////////pool////////////////////
BOOST_FIXTURE_TEST_CASE(bind_tests, eoswap_tester) try {
 
// // admin balances
// await weth.mint(admin, toWei('5'));
// await dai.mint(admin, toWei('200'));

// // nonAdmin balances
// await weth.mint(nonAdmin, toWei('1'), { from: admin });
// await dai.mint(nonAdmin, toWei('50'), { from: admin });

// await pool.bind(WETH, toWei('5'), toWei('5'));
// await pool.bind(DAI, toWei('200'), toWei('5'));

// await pool.finalize();

// await pool.joinPool(toWei('10'), [MAX, MAX], { from: nonAdmin });
// await pool.exitPool(toWei('10'), [toWei('0'), toWei('0')], { from: nonAdmin });


  uint_eth eamount = to_wei(5);
  uint_eth damount = to_wei(200);
  uint_eth jamount = to_wei(10);
  newpool(N(alice), N(pool));
  tmint(N(alice), N(weth), eamount);
  tmint(N(alice), N(dai), damount);
  bind(N(alice), N(pool), N(weth), eamount, to_wei(5));
  bind(N(alice), N(pool), N(dai), damount, to_wei(5));
  finalize(N(alice), N(pool));
  std::vector<uint_eth> v{uint_eth(-1),uint_eth(-1)};
  joinpool(N(alice), N(pool), jamount, v);
  exitpool(N(alice), N(pool), jamount, std::vector<uint_eth>{0, 0});
  collect(N(eoswapeoswap), N(pool));

  BOOST_REQUIRE_EQUAL(1, get_token_store()["allowance"].get_array().size());
  const auto ts = get_token_store()["allowance"];
}
FC_LOG_AND_RETHROW()

////////////////token////////////////////
BOOST_FIXTURE_TEST_CASE(mint_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
  mint(N(alice), N(pool), 300);
  const auto ts = get_token_store()["balance"];
  const auto kv = array_to_kv(ts);

  const auto m = kv.find(std::string("eoswapeoswap"));
  bool flag = m != kv.end();
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(approve_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
  approve(N(alice), N(pool), N(bob), 300);

  BOOST_REQUIRE_EQUAL(1, get_token_store()["allowance"].get_array().size());
  const auto ts = get_token_store()["allowance"];

  //  const auto b = get_token_store()["allowance"].get_array();
  //   for (int i = 0; i < b.size(); i++) {
  //     const auto a = b.at(i);

  //     // BOOST_REQUIRE_NO_THROW(a["key"] == "eoswapeoswap");
  //     const auto aa = a["value"]["a2amap"].get_array();
  //     const auto aaa = aa.at(0);
  //     BOOST_REQUIRE_EQUAL(aaa["value"], 300);
  //   }
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(transfer_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
  approve(N(alice), N(pool), N(bob), 300);
  const auto ts = get_token_store()["balance"];
  const auto kv = array_to_kv(ts);

  const auto m = kv.find(std::string("eoswapeoswap"));
  bool flag = m != kv.end();
}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
