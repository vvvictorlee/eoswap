#pragma once
#include <cmath>
#include <common/dos.functions.hpp>
#include <common/dos.types.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>
#include <eosio/eosio.hpp>
#include <eosio/singleton.hpp>
#include <eosio/symbol.hpp>
#include <eosio/system.hpp>
// #define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %d: "format"/n", __LINE__, ##__VA_ARGS__)
// #define EOSDOS_CONTRACT_DEBUG

#define _DODO_DEBUG
#if defined(EOSDOS_CONTRACT_DEBUG) && defined(_DODO_DEBUG)
#define DODO_DEBUG(...)                                                                                                \
   do {                                                                                                                \
      my_print_f("        <% % %>         ", __FILE__, __LINE__, __FUNCTION__);                                          \
      my_print_f(__VA_ARGS__);    \
      my_print_f("                      ");                                                                           \
   } while (0);
#else
#define DODO_DEBUG(...)
#endif

#define _TRADER_DEBUG
#ifdef _TRADER_DEBUG
#define TRADER_DEBUG(args...) DODO_DEBUG(##args)
#else
#define TRADER_DEBUG
#endif

#define _PRICING_DEBUG
#ifdef _PRICING_DEBUG
#define PRICING_DEBUG(args...) DODO_DEBUG(##args)
#else
#define PRICING_DEBUG
#endif

#define _DODOMATH_DEBUG
#ifdef _DODOMATH_DEBUG
#define DODOMATH_DEBUG(args...) DODO_DEBUG(##args)
#else
#define DODOMATH_DEBUG
#endif

#ifdef EOSDOS_CONTRACT_DEBUG
#define contract_debug(args...) print(" | ", ##args)
#else
#define contract_debug
#endif

using namespace eosio;

using bytes   = std::vector<char>;
using bytes32 = std::vector<char>;
using address = name;
using uint256 = uint128_t;
using uint112 = uint64_t;
using uint32  = uint32_t;
using namesym = uint128_t;

template <typename Arg, typename... Args>
inline void my_print_f(const char* s, Arg val, Args... rest) {
#ifdef EOSDOS_CONTRACT_DEBUG
   print_f(s, val, rest...);
#endif
}

inline void my_print_f(const char* s) {
#ifdef EOSDOS_CONTRACT_DEBUG
   print(s);
#endif
}

void require(bool test, const std::string& cstr) { eosio::check(test, cstr); }

namesym to_namesym(const extended_symbol& exsym) {
   namesym ns = exsym.get_contract().value;
   ns         = ns << 64 | exsym.get_symbol().raw();
   return ns;
}

namesym to_namesymcode(const extended_symbol& exsym) {
   namesym ns = exsym.get_contract().value;
   ns         = ns << 64 | exsym.get_symbol().code().raw();
   return ns;
}

constexpr double my_pow(double x, int exp) {
   int sign = 1;
   if (exp < 0) {
      sign = -1;
      exp  = -exp;
   }
   if (exp == 0)
      return x < 0 ? -1.0 : 1.0;
   double ret = x;
   while (--exp)
      ret *= x;
   return sign > 0 ? ret : 1.0 / ret;
}

std::vector<std::string> split(const std::string& str, const std::string& delims = ":") {
   std::vector<std::string> output;
   auto                     first = std::cbegin(str);
   while (first != std::cend(str)) {
      const auto second = std::find_first_of(first, std::cend(str), std::cbegin(delims), std::cend(delims));
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
   name            msg_sender;
   std::string     action;
   std::string     param;
   extended_symbol ext_sym;
   extended_asset  ext_asset;
};

static const std::string bind_action_string     = "bind";
static const std::string joinpool_action_string = "joinpool";
static const std::string swapin_action_string   = "swapin";
static const std::string swapout_action_string  = "swapout";