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

class eoswap_tester : public tester {
public:
  eoswap_tester() {
    produce_blocks(2);

    create_accounts({N(alice), N(bob), N(carol), N(eoswapeoswap)});
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

  action_result newpool(account_name msg_sender,account_name pool_name) {
    return push_action(msg_sender, N(newpool), mvo()("msg_sender", msg_sender)("pool_name",pool_name));
  }

  // msg_sender, name dst, uint amt
  action_result mint(account_name msg_sender, uint64_t amt,account_name pool_name) {
    return push_action(msg_sender, N(mint),
                       mvo()("msg_sender", msg_sender)("amt", amt)("pool_name",pool_name));
  }

  action_result transfer(account_name msg_sender, account_name dst,
                         uint64_t amt,account_name pool_name) {
    return push_action(msg_sender, N(transfer),
                       mvo()("msg_sender", msg_sender)("dst", dst)("amt", amt)("pool_name",pool_name));
  }

  action_result approve(account_name msg_sender, account_name dst,
                        uint64_t amt,account_name pool_name) {
    return push_action(msg_sender, N(approve),
                       mvo()("msg_sender", msg_sender)("dst", dst)("amt", amt)("pool_name",pool_name));
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

  abi_serializer abi_ser;
};

BOOST_AUTO_TEST_SUITE(eoswap_tests)

BOOST_FIXTURE_TEST_CASE(newpoool_tests, eoswap_tester) try {

  newpool(N(eoswapeoswap),N(pool));
}
FC_LOG_AND_RETHROW()


BOOST_FIXTURE_TEST_CASE(transfer5_tests, eoswap_tester) try {

  //   newpool(N(eoswapeoswap));
  mint(N(alice), 300,N(pool));
  const auto ts = get_token_store()["balance"];
  const auto kv = array_to_kv(ts);

  const auto m = kv.find(std::string("eoswapeoswap"));
  bool flag = m != kv.end();

}
FC_LOG_AND_RETHROW()

BOOST_FIXTURE_TEST_CASE(approve_tests, eoswap_tester) try {

  //   newpool(N(eoswapeoswap));
  approve(N(alice), N(bob), 300,N(pool));

  BOOST_REQUIRE_EQUAL(1 , get_token_store()["allowance"].get_array().size());
  const auto b = get_token_store()["allowance"].get_array();
  for (int i = 0; i < b.size(); i++) {
    const auto a = b.at(i);

    // BOOST_REQUIRE_NO_THROW(a["key"] == "eoswapeoswap");
    const auto aa = a["value"]["a2amap"].get_array();
    const auto aaa = aa.at(0);
     BOOST_REQUIRE_EQUAL(aaa["value"] ,300);
  }
}
FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()
