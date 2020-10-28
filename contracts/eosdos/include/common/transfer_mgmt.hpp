#pragma once
#include <common/defines.hpp>
#include <common/eosio.token.hpp>
class transfer_mgmt {
 private:
   name self;

 public:
   static constexpr eosio::name token_account{"eosio.token"_n};
   static constexpr eosio::name active_permission{"active"_n};

   //-------------------------------------------------------------------------
   transfer_mgmt(name _self)
       : self(_self) {}

   template <typename T>
   void eosiotoken_transfer(name from, name to, asset quantity, std::string memo, T func) {
      if (from == self || to != self || quantity.symbol != core_symbol() || memo.empty()) {
         // print("memo is empty on trasfer");
         return;
      }
      // eosio::check(quantity.symbol == eosio::symbol("EOS", 4),
      //              "only accepts EOS for deposits");
      eosio::check(quantity.is_valid(), "Invalid token transfer");
      eosio::check(quantity.amount > 0, "Quantity must be positive");

      // system account could transfer eos to contract
      // eg) unstake, sellram, etc
      if (is_system_account(from)) {
         return;
      }

      std::vector<std::string> action_parameters = parse_string(memo, ";");
      const int                memo_size         = 1;
      check(action_parameters.size() > memo_size, "parse memo failed ");

      transfer_data data;
      data.msg_sender = from;
      data.action     = action_parameters[0];
      data.param      = action_parameters[1];

      func(data);
   }

   template <typename T>
   void non_eosiotoken_transfer(name from, name to, asset quantity, std::string memo, T func) {
      if (from == self || to != self || quantity.symbol == core_symbol() || memo.empty()) {
         return;
      }

      std::vector<std::string> action_parameters = parse_string(memo, ";");
      const int                memo_size         = 1;
      check(action_parameters.size() > memo_size, "parse memo failed ");

      transfer_data data;
      data.msg_sender = from;
      data.action     = action_parameters[0];
      data.param      = action_parameters[1];

      func(data);
   }

   bool is_system_account(name name) {
      if (name == "eosio.bpay"_n || name == "eosio.msig"_n || name == "eosio.names"_n || name == "eosio.ram"_n ||
          name == "eosio.ramfee"_n || name == "eosio.saving"_n || name == "eosio.stake"_n || name == "eosio.token"_n ||
          name == "eosio.vpay"_n) {
         return true;
      }
      return false;
   }

   symbol core_symbol() const {
      symbol _core_symbol = symbol(symbol_code("EOS"), 4);
      return _core_symbol;
   }

   static asset get_balance(const name& owner, const extended_symbol& exsym) {
      return eosio::token::get_balance(exsym.get_contract(), owner, exsym.get_symbol().code());
   }

   static name get_issuer(const extended_symbol& exsym) {
      return eosio::token::get_issuer(exsym.get_contract(), exsym.get_symbol().code());
   }

   /**
    * @brief
    *
    * @param from
    * @param to
    * @param quantity
    * @param memo
    */
   void transfer(name from, name to, extended_asset quantity, std::string memo) {
      if (from == to) {
         return;
      }
      inner_transfer(from, to, quantity, memo);
   }

   void inner_transfer(name from, name to, extended_asset quantity, std::string memo, bool is_deferred = false) {
      check(from != to, "cannot transfer to self");
      //  require_auth( from );
      check(is_account(to), "to account does not exist");
      check(quantity.quantity.is_valid(), "invalid quantity");
      check(quantity.quantity.amount > 0, "must transfer positive quantity");
      check(memo.size() <= 256, "memo has more than 256 bytes");

      if (!is_deferred) {
         action(
             permission_level{from, "active"_n}, quantity.contract, "transfer"_n,
             std::make_tuple(from, to, quantity.quantity, memo))
             .send();
      } else {
         transaction t;
         t.actions.emplace_back(
             permission_level{from, active_permission}, quantity.contract, "transfer"_n,
             std::make_tuple(from, to, quantity.quantity, memo));
         t.delay_sec           = 0;
         uint128_t deferred_id = (uint128_t(to.value) << 64) | current_time_point_sec().sec_since_epoch();
         cancel_deferred(deferred_id);
         t.send(deferred_id, self, true);
      }

      // INLINE_ACTION_SENDER(eosio::token, transfer)(token_account, {{from,
      // active_permission}, {to, active_permission}},{from, to, quantity, memo});
   }

   void create(name issuer, const extended_asset& maximum_supply) {
      require_auth(issuer);
      check(is_account(issuer), "issuer account does not exist");

      check(maximum_supply.quantity.is_valid(), "invalid quantity");
      check(maximum_supply.quantity.amount > 0, "must transfer positive quantity");
      action(
          permission_level{"eosdosxtoken"_n, "active"_n}, maximum_supply.contract, "create"_n,
          std::make_tuple(issuer, maximum_supply.quantity))
          .send();
   }

   void issue(name to, const extended_asset& quantity, const string& memo) {
      check(is_account(to), "to account does not exist");
      check(quantity.quantity.is_valid(), "invalid quantity");
      check(quantity.quantity.amount > 0, "must transfer positive quantity");
      check(memo.size() <= 256, "memo has more than 256 bytes");
      auto issuer = get_issuer(quantity.get_extended_symbol());
      action(
          permission_level{issuer, "active"_n}, quantity.contract, "issue"_n,
          std::make_tuple(issuer, quantity.quantity, memo))
          .send();
      if (to != issuer) {
         transfer(issuer, to, quantity, memo);
      }
   }

   void burn(name issuer, const extended_asset& quantity, const string& memo) {
      check(is_account(issuer), "issuer account does not exist");

      check(quantity.quantity.is_valid(), "invalid quantity");
      check(quantity.quantity.amount > 0, "must transfer positive quantity");
      check(memo.size() <= 256, "memo has more than 256 bytes");

      action(
          permission_level{issuer, "active"_n}, quantity.contract, "retire"_n, std::make_tuple(quantity.quantity, memo))
          .send();
   }

   static std::vector<std::string> parse_string(const std::string& source, const std::string& delimiter = ",") {
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

   static uint64_t to_int(const std::string& str) {
      bool        isOK   = false;
      const char* nptr   = str.c_str();
      char*       endptr = NULL;
      errno              = 0;
      uint64_t val       = std::strtoull(nptr, &endptr, 10);
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
};