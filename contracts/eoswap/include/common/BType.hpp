#pragma once
#include <cmath>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/symbol.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

#define IS_DEBUG true

#if IS_DEBUG
    #define debug(args...) print(" | ", ##args)
#else
    #define debug(args...)
#endif


using namespace eosio;

using bytes = std::vector<char>;

using address = name;
using uint = uint64_t;
using uint8 = uint8_t;


static const std::string default_core_symbol = "BPT";
static const uint8_t default_precision = 4;
static const std::string chain_token = "eth";
static const std::string address_zero = "0";

static const uint8_t current_bridge_version = 1;

void require(bool test, const char *cstr) { eosio::check(test, cstr); }



constexpr double my_pow(double x, int exp)
{
    int sign = 1;
    if (exp < 0)
    {
        sign = -1;
        exp = -exp;
    }
    if (exp == 0)
        return x < 0 ? -1.0 : 1.0;
    double ret = x;
    while (--exp)
        ret *= x;
    return sign > 0 ? ret : 1.0/ret;
}


std::vector<std::string> split(const std::string &str,
                               const std::string &delims = ":") {
  std::vector<std::string> output;
  auto first = std::cbegin(str);
  while (first != std::cend(str)) {
    const auto second = std::find_first_of(
        first, std::cend(str), std::cbegin(delims), std::cend(delims));
    if (first != second)
      output.emplace_back(first, second);
    if (second == std::cend(str))
      break;
    first = std::next(second);
  }
  return output;
}

time_point_sec current_time_point_sec() {
  const static time_point_sec cts{current_time_point()};
  return cts;
}
struct transfer_info {
    name from;
    std::string action;
    std::string param;
    uint32_t type;
    name seller;
    asset quantity;
    uint32_t block;
    uint32_t checksum;
};




enum transfer_type : uint8_t { tt_freeze, tt_delay };
enum transfer_category : uint8_t { tc_service_stake, tc_pay_service, tc_deposit, tc_arbitration_stake_appeal, tc_arbitration_stake_arbitrator, tc_arbitration_stake_resp_case, tc_risk_guarantee };

enum deposit_index : uint8_t { index_category, index_from, index_to, index_notify, deposit_count };

enum appeal_index : uint8_t { index_id = 1, index_evidence, index_info, index_reason, index_provider, appeal_count };

enum arbitrator_index : uint8_t { index_type = 1, arbitrator_count };

const uint8_t resp_case_count = 3;

enum risk_guarantee_index : uint8_t { index_duration = 2, risk_guarantee_case_count };
