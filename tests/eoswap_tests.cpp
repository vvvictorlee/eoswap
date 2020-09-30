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

class findx {
public:
  findx(const string str) { test = str; }
  bool operator()(const fc::variant &v) {
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

    create_accounts({N(alice), N(bob), N(carol), N(eoswapeoswap), N(pool)});
    produce_blocks(2);
    admin = N(eoswapeoswap);
    nonadmin = N(alice);
    user1 = N(bob);
    user2 = N(carol);
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

  action_result swapamtin(name msg_sender, name pool_name, name tokenIn,
                          uint tokenAmountIn, name tokenOut, uint minAmountOut,
                          uint maxPrice) {
    return push_action(msg_sender, N(swapamtin),
                       mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
                           "tokenIn", tokenIn)("tokenAmountIn", tokenAmountIn)(
                           "tokenOut", tokenOut)("minAmountOut", minAmountOut)(
                           "maxPrice", maxPrice));
  }

  action_result swapamtout(name msg_sender, name pool_name, name tokenIn,
                           uint maxAmountIn, name tokenOut, uint tokenAmountOut,
                           uint maxPrice) {
    return push_action(
        msg_sender, N(swapamtout),
        mvo()("msg_sender", msg_sender)("pool_name", pool_name)(
            "tokenIn", tokenIn)("maxAmountIn", maxAmountIn)(
            "tokenOut", tokenOut)("tokenAmountOut", tokenAmountOut)("maxPrice",
                                                                    maxPrice));
  }

  ////////////////TOKEN//////////////

  action_result approve(account_name msg_sender, account_name token,
                        account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(approve),
                       mvo()("msg_sender", msg_sender)("token", token)(
                           "dst", dst)("amt", amt));
  }

  action_result transfer(account_name msg_sender, account_name token,
                         account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(transfer),
                       mvo()("msg_sender", msg_sender)("token", token)(
                           "dst", dst)("amt", amt));
  }

  action_result transferfrom(account_name msg_sender, account_name token,
                             account_name src, account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(transferfrom),
                       mvo()("msg_sender", msg_sender)("token", token)(
                           "src", src)("dst", dst)("amt", amt));
  }

  action_result incapproval(account_name msg_sender, account_name token,
                            account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(incapproval),
                       mvo()("msg_sender", msg_sender)("token", token)(
                           "dst", dst)("amt", amt));
  }

  action_result decapproval(account_name msg_sender, account_name token,
                            account_name dst, uint_eth amt) {
    return push_action(msg_sender, N(decapproval),
                       mvo()("msg_sender", msg_sender)("token", token)(
                           "dst", dst)("amt", amt));
  }

  action_result mint(account_name msg_sender, account_name token,
                     uint_eth amt) {
    return push_action(
        msg_sender, N(mint),
        mvo()("msg_sender", msg_sender)("token", token)("amt", amt));
  }

  action_result burn(account_name msg_sender, account_name token,
                     uint_eth amt) {
    return push_action(
        msg_sender, N(burn),
        mvo()("msg_sender", msg_sender)("token", token)("amt", amt));
  }

  action_result move(account_name token, account_name src, name dst,
                     uint_eth amt) {
    return push_action(
        src, N(move),
        mvo()("token", token)("src", src)("dst", dst)("amt", amt));
  }

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

  std::string find_value(const fc::variant &v, const std::string &key) {
    auto a = v.get_array();
    auto i = std::find_if(a.begin(), a.end(), findx(key));
    if (i != a.end()) {
      fc::variant vv = *i;
      return vv["value"].as_string();
    }
    return std::string("");
  }

  fc::variant find_variant(const fc::variant &v, const std::string &key) {
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
    const auto s = get_token_store();
    const auto t = find_variant(s["tokens"], token.to_string());
    const auto a = find_variant(t["allowance"], src.to_string());
    const auto amt = find_value(a["dst2amt"], dst.to_string());
    return amt;
  }

  fc::variant records(name pool_name, name token) {
    const auto ps = get_pool_store();
    const auto p = find_variant(ps["pools"], pool_name.to_string());
    const auto r = find_value(p["records"], token.to_string());
    return r;
  }

  uint_eth to_wei(uint_eth value) { return value * pow(10, 6); }

  void newpoolBefore() { newpool(admin, N(pool)); }

  void setswapfeeBefore() {
    // await pool.setSwapFee(toWei('0.003'));
  }

  void mintBefore() {

    uint_eth eamount = to_wei(5);
    uint_eth damount = to_wei(200);
    uint_eth neamount = to_wei(1);
    uint_eth ndamount = to_wei(200);
    uint_eth jamount = to_wei(10);

    // mint(admin, N(weth), to_wei(5));
    // mint(admin, N(dai), to_wei(200));

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

  name admin;
  name nonadmin;
  name user1;
  name user2;
  abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eoswap_tests)
////////////////factory////////////////////
BOOST_FIXTURE_TEST_CASE(newpool_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(setblabs_tests, eoswap_tester) try {
  setblabs(N(eoswapeoswap), N(alice));
  std::string b = get_factory_store()["blabs"].as_string();
  BOOST_REQUIRE_EQUAL("alice", b);
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////
BOOST_FIXTURE_TEST_CASE(mint_tests, eoswap_tester) try {

  uint_eth eamount = to_wei(5);
  uint_eth damount = to_wei(200);
  uint_eth jamount = to_wei(10);
  newpool(N(alice), N(pool));
  mint(N(alice), N(weth), eamount);
  mint(N(alice), N(dai), damount);

  const auto ab = balanceOf(N(weth), N(alice));
  BOOST_REQUIRE_EQUAL(std::to_string(eamount), ab);
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////
BOOST_FIXTURE_TEST_CASE(bind_tests, eoswap_tester) try {

  uint_eth eamount = to_wei(5);
  uint_eth damount = to_wei(200);
  uint_eth jamount = to_wei(10);
  newpool(N(alice), N(pool));
  mint(N(alice), N(weth), eamount);
  mint(N(alice), N(dai), damount);
  bind(N(alice), N(pool), N(weth), eamount, to_wei(5));
  bind(N(alice), N(pool), N(dai), damount, to_wei(5));
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////
BOOST_FIXTURE_TEST_CASE(finalize_tests, eoswap_tester) try {

  uint_eth eamount = to_wei(5);
  uint_eth damount = to_wei(200);
  uint_eth neamount = to_wei(1);
  uint_eth ndamount = to_wei(200);
  uint_eth jamount = to_wei(10);
  newpool(admin, N(pool));
  mint(admin, N(weth), eamount);
  mint(admin, N(dai), damount);
  mint(nonadmin, N(weth), neamount);
  mint(nonadmin, N(dai), ndamount);

  bind(admin, N(pool), N(weth), eamount, to_wei(5));
  bind(admin, N(pool), N(dai), damount, to_wei(5));
  finalize(admin, N(pool));
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////
BOOST_FIXTURE_TEST_CASE(joinpool_tests, eoswap_tester) try {

  uint_eth eamount = to_wei(5);
  uint_eth damount = to_wei(200);
  uint_eth neamount = to_wei(1);
  uint_eth ndamount = to_wei(200);
  uint_eth jamount = to_wei(10);
  newpool(admin, N(pool));
  mint(admin, N(weth), eamount);
  mint(admin, N(dai), damount);
  mint(nonadmin, N(weth), neamount);
  mint(nonadmin, N(dai), ndamount);

  bind(admin, N(pool), N(weth), eamount, to_wei(5));
  bind(admin, N(pool), N(dai), damount, to_wei(5));
  finalize(admin, N(pool));
  std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1)};
  const auto t = get_token_store();
  joinpool(nonadmin, N(pool), jamount, v);
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////
BOOST_FIXTURE_TEST_CASE(exitpool_tests, eoswap_tester) try {

  uint_eth eamount = to_wei(5);
  uint_eth damount = to_wei(200);
  uint_eth neamount = to_wei(1);
  uint_eth ndamount = to_wei(200);
  uint_eth jamount = to_wei(10);
  newpool(admin, N(pool));
  mint(admin, N(weth), eamount);
  mint(admin, N(dai), damount);
  mint(nonadmin, N(weth), neamount);
  mint(nonadmin, N(dai), ndamount);

  bind(admin, N(pool), N(weth), eamount, to_wei(5));
  bind(admin, N(pool), N(dai), damount, to_wei(5));
  finalize(admin, N(pool));
  std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1)};
  const auto t = get_token_store();
  joinpool(nonadmin, N(pool), jamount, v);
  exitpool(nonadmin, N(pool), jamount, std::vector<uint_eth>{0, 0});
}
FC_LOG_AND_RETHROW()

////////////////pool////////////////////
BOOST_FIXTURE_TEST_CASE(collect_tests, eoswap_tester) try {

  uint_eth eamount = to_wei(5);
  uint_eth damount = to_wei(200);
  uint_eth neamount = to_wei(1);
  uint_eth ndamount = to_wei(200);
  uint_eth jamount = to_wei(10);
  newpool(admin, N(pool));
  mint(admin, N(weth), eamount);
  mint(admin, N(dai), damount);
  mint(nonadmin, N(weth), neamount);
  mint(nonadmin, N(dai), ndamount);

  bind(admin, N(pool), N(weth), eamount, to_wei(5));
  bind(admin, N(pool), N(dai), damount, to_wei(5));
  finalize(admin, N(pool));
  std::vector<uint_eth> v{uint_eth(-1), uint_eth(-1)};
  const auto t = get_token_store();
  joinpool(nonadmin, N(pool), jamount, v);
  exitpool(nonadmin, N(pool), jamount, std::vector<uint_eth>{0, 0});
  collect(admin, N(pool));
  const auto ab = balanceOf(N(pool), admin);

  BOOST_REQUIRE_EQUAL(std::to_string(to_wei(100)), ab);
}
FC_LOG_AND_RETHROW()

////////////////token////////////////////
BOOST_FIXTURE_TEST_CASE(burn_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
  mint(N(alice), N(pool), 300);
  burn(N(alice), N(pool), 300);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(approve_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
  approve(N(alice), N(pool), N(bob), 300);
  const auto b = allowance(N(pool), N(alice), N(bob));

  BOOST_REQUIRE_EQUAL("300", b);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(transfer_tests, eoswap_tester) try {

  newpool(N(alice), N(pool));
  mint(N(alice), N(pool), 300);
  transfer(N(alice), N(pool), N(bob), 300);
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountIn_tests, eoswap_tester) try {

  before();
  swapamtin(N(bob), N(pool), N(weth), 2500000, N(dai), to_wei(475),
            to_wei(200));
}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(swapExactAmountOut_tests, eoswap_tester) try {

  before();
  swapamtout(N(bob), N(pool), N(weth), to_wei(3), N(mkr), to_wei(1),
             to_wei(500));
}
FC_LOG_AND_RETHROW()

// it('swapExactAmountIn', async () => {
// // 2.5 WETH -> DAI
// const expected = calcOutGivenIn(52.5, 5, 10500, 5, 2.5, 0.003);
// const txr = await pool.swapExactAmountIn(
// WETH,
// toWei('2.5'),
// DAI,
// toWei('475'),
// toWei('200'),
// { from: user2 },
// );
// const log = txr.logs[0];
// assert.equal(log.event, 'LOG_SWAP');
// // 475.905805337091423

// const actual = fromWei(log.args[4]);
// const relDif = calcRelativeDiff(expected, actual);
// if (verbose) {
// console.log('swapExactAmountIn');
// console.log(`expected: ${expected})`);
// console.log(`actual  : ${actual})`);
// console.log(`relDif  : ${relDif})`);
// }

// assert.isAtMost(relDif.toNumber(), errorDelta);

// const userDaiBalance = await dai.balanceOf(user2);
// assert.equal(fromWei(userDaiBalance), Number(fromWei(log.args[4])));

// // 182.804672101083406128
// const wethPrice = await pool.getSpotPrice(DAI, WETH);
// const wethPriceFeeCheck = ((10024.094194662908577 / 5) / (55 / 5)) * (1 / (1
// - 0.003)); assert.approximately(Number(fromWei(wethPrice)),
// Number(wethPriceFeeCheck), errorDelta);

// const daiNormWeight = await pool.getNormalizedWeight(DAI);
// assert.equal(0.333333333333333333, fromWei(daiNormWeight));
// });

// it('swapExactAmountOut', async () => {
// // ETH -> 1 MKR
// // const amountIn = (55 * (((21 / (21 - 1)) ** (5 / 5)) - 1)) / (1 - 0.003);
// const expected = calcInGivenOut(55, 5, 21, 5, 1, 0.003);
// const txr = await pool.swapExactAmountOut(
// WETH,
// toWei('3'),
// MKR,
// toWei('1.0'),
// toWei('500'),
// { from: user2 },
// );
// const log = txr.logs[0];
// assert.equal(log.event, 'LOG_SWAP');
// // 2.758274824473420261

// const actual = fromWei(log.args[3]);
// const relDif = calcRelativeDiff(expected, actual);
// if (verbose) {
// console.log('swapExactAmountOut');
// console.log(`expected: ${expected})`);
// console.log(`actual  : ${actual})`);
// console.log(`relDif  : ${relDif})`);
// }

// assert.isAtMost(relDif.toNumber(), errorDelta);
// });

BOOST_AUTO_TEST_SUITE_END()
