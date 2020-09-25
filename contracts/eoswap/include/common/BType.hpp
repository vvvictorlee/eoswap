#pragma once
#include <cmath>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/symbol.hpp>
#include <eosio/system.hpp>
#include <eosio/singleton.hpp>

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
