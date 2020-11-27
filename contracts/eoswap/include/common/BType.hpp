#pragma once
#include <cmath>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/symbol.hpp>
#include <eosio/system.hpp>

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
using namesym =uint128_t;

static const std::string default_core_symbol = "BPT";
static const uint8_t default_precision = 4;
static const std::string chain_token = "eth";
static const std::string address_zero = "0";

static const uint8_t current_bridge_version = 1;
// namespace eoswap
// {
void require(bool test, const char *cstr) { eosio::check(test, cstr); }

namesym to_namesym(const extended_symbol& exsym)
{
    namesym ns = exsym.get_contract().value;
    ns = ns << 64| exsym.get_symbol().raw();
    return ns;
}
///static compile time
constexpr double my_pow(double x, int exp) {
  int sign = 1;
  if (exp < 0) {
    sign = -1;
    exp = -exp;
  }
  if (exp == 0)
    return x < 0 ? -1.0 : 1.0;
  double ret = x;
  while (--exp)
    ret *= x;
  return sign > 0 ? ret : 1.0 / ret;
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
struct transfer_data {
  name msg_sender;
  std::string action;
  std::string param;
  extended_symbol ext_sym;
  extended_asset ext_asset;
};

static const std::string bind_action_string = "bind";
static const std::string joinpool_action_string = "joinpool";
static const std::string swapin_action_string = "swapin";
static const std::string swapout_action_string = "swapout";
// }