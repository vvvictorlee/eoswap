#pragma once
#include <common/BType.hpp>
#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>

#include <string>
#include <vector>

// #include <eosio.system/eosio.system.hpp>

// namespace roxesystem {
//    class system_contract;
//}

namespace eosio {

using std::find;
using std::string;
using std::vector;

// using eosiosystem::system_contract;

/**
 * roxe.tokenize contract defines the structures and actions that allow users to create, issue, and manage
 * tokens on eosio based blockchains.
 */
// class [[eosio::contract("roxe.tokenize")]] tokenize : public contract {
class tokenize {
   name self;

 public:
   tokenize(name _self)
       : self(_self) {}
   name get_self() { return self; }

#ifdef TOKENIZE_FEE
   using contract::contract;
   /**
    * Allows `issuer` account to create a token in supply of `maximum_supply`. If validation is successful a new entry
    * in statstable for token symbol scope gets created.
    *
    * @param issuer - the account that creates the token,
    * @param maximum_supply - the maximum supply set for the token created.
    *
    * @pre Token symbol has to be valid,
    * @pre Token symbol must not be already created,
    * @pre maximum_supply has to be smaller than the maximum supply allowed by the system: 1^62 - 1.
    * @pre Maximum supply must be positive;
    */
   [[eosio::action]] void create(const name& issuer, const asset& maximum_supply);

   /**
    *  This action issues to `to` account a `quantity` of tokens.
    *
    * @param from - the account to be authorized to issue tokens,
    * @param to - the account to issue tokens to,
    * @param quntity - the amount of tokens to be issued,
    * @memo - the memo string that accompanies the token issue transaction.
    */
   [[eosio::action]] void issue(const name& from, const name& to, const asset& quantity, const string& memo);

   /**
    * The opposite for create action, if all validations succeed,
    * it debits the statstable.supply amount.
    *
    * @param from - the account of tokens to retire,
    * @param quantity - the quantity of tokens to retire,
    * @param memo - the memo string to accompany the transaction.
    */
   [[eosio::action]] void retire(const name& from, const asset& quantity, const string& memo);

   /**
    * add author for issuer
    *
    * @param author - the account of authorization ,
    * @param sym - the symbol of tokens,
    */
   [[eosio::action]] void addauthor(const symbol& sym, const name& author);

   /**
    * delete author for issuer
    * @param author - the account of authorization ,
    * @param sym - the symbol of tokens,
    */
   [[eosio::action]] void delauthor(const symbol& sym, const name& from);

   /**
    * Allows `from` account to transfer to `to` account the `quantity` tokens.
    * One account is debited and the other is credited with quantity tokens.
    *
    * @param from - the account to transfer from,
    * @param to - the account to be transferred to,
    * @param quantity - the quantity of tokens to be transferred,
    * @param memo - the memo string to accompany the transaction.
    */
   [[eosio::action]] void transfer(const name& from, const name& to, const asset& quantity, const string& memo);

   /**
    * Allows `ram_payer` to create an account `owner` with zero balance for
    * token `symbol` at the expense of `ram_payer`.
    *
    * @param owner - the account to be created,
    * @param symbol - the token to be payed with by `ram_payer`,
    * @param ram_payer - the account that supports the cost of this action.
    *
    * More information can be read [here](https://github.com/ROXE/eosio.contracts/issues/62)
    * and [here](https://github.com/ROXE/eosio.contracts/issues/61).
    */
   [[eosio::action]] void open(const name& owner, const symbol& symbol, const name& ram_payer);

   /**
    * This action is the opposite for open, it closes the account `owner`
    * for token `symbol`.
    *
    * @param owner - the owner account to execute the close action for,
    * @param symbol - the symbol of the token to execute the close action for.
    *
    * @pre The pair of owner plus symbol has to exist otherwise no action is executed,
    * @pre If the pair of owner plus symbol exists, the balance has to be zero.
    */
   [[eosio::action]] void close(const name& owner, const symbol& symbol);

   /**
    * Set transaction fee `fee` for token `symbol`.
    *
    * @param symbol - the token to be payed,
    * @param fee - the number that supports the cost of each transaction.
    *
    */
   [[eosio::action]] /// FIXME add transaction fee
   void
   setfee(const symbol& symbol, const int64_t fee);

   /**
    * Set transaction fee percent `percent` for token `symbol`.
    *
    * @param symbol - the token to be payed,
    * @param percent - the percent that supports the fee percent of each transaction.
    *
    */
   [[eosio::action]] /// FIXME add transaction fee
   void
   setfeeper(const symbol& symbol, const int64_t percent);

   /**
    * Set transaction fee `max fee` for token `symbol`.
    *
    * @param symbol - the token to be payed,
    * @param maxfee - the percent that supports the max fee of each transaction.
    *
    */
   [[eosio::action]] /// FIXME add transaction fee
   void
   setmaxfee(const symbol& symbol, const int64_t maxfee);

   /**
    * Set transaction fee `min fee` for token `symbol`.
    *
    * @param symbol - the token to be payed,
    * @param minfee - the percent that supports the min fee of each transaction.
    *
    */
   [[eosio::action]] /// FIXME add transaction fee
   void
   setminfee(const symbol& symbol, const int64_t minfee);

   /**
    * Set transaction fee token `roc` for token `symbol`.
    *
    * @param symbol - the token to be payed,
    * @param roc - the useroc that supports the roc token fee of each transaction.
    *
    */
   [[eosio::action]] /// FIXME add transaction fee
   void
   useroc(const symbol& symbol, const bool roc);

   /**
    * Set transaction fee fix or not for token `symbol`.
    *
    * @param symbol - the token to be payed,
    * @param fix - the setfix that supports fix fee of each transaction.
    *
    */
   [[eosio::action]] /// FIXME fix transaction fee
   void
   setfix(const symbol& symbol, const bool fix);

   static asset get_supply(const name& token_contract_account, const symbol_code& sym_code) {
      stats       statstable(token_contract_account, sym_code.raw());
      const auto& st = statstable.get(sym_code.raw());
      return st.supply;
   }

   static asset get_balance(const name& token_contract_account, const name& owner, const symbol_code& sym_code) {
      accounts    accountstable(token_contract_account, owner.value);
      const auto& ac = accountstable.get(sym_code.raw());
      return ac.balance;
   }
#endif
   static constexpr symbol core_symbol = symbol(symbol_code("ROC"), 4);
   static constexpr int    param_count = 6;
   //  int64_t fee;
   // int64_t percent;
   // int64_t maxfee;
   // int64_t minfee;
   // bool fixed;
   // bool useroc;
   void setparameter(const symbol& symbol, const std::vector<int64_t> params) {
      require_auth(get_self());
      check(params.size() >= param_count, "params' count couldn't be less than 6.");
      check(params[0] >= default_tx_fee, "Cannot set fee below default value(0).");
      check(params[2] >= params[3], "maxfee couldn't be less than minfee.");

      stats statstable(get_self(), symbol.code().raw());
      auto  existing = statstable.find(symbol.code().raw());
      //   check(existing != statstable.end(), "token with symbol does not exist, create token before setfee");
      currency_stats s;
      if (existing != statstable.end()) {
         s = *existing;
      }
      s.supply = asset{1, symbol};

      std::map<int64_t, int64_t&> paras = {
          std::pair<int64_t, int64_t&>{0, s.fee}, std::pair<int64_t, int64_t&>{1, s.percent},
          std::pair<int64_t, int64_t&>{2, s.maxfee}, std::pair<int64_t, int64_t&>{3, s.minfee}};
      std::map<int64_t, bool&> boolparas = {std::pair<int64_t, bool&>{4, s.fixed},
                                            std::pair<int64_t, bool&>{5, s.useroc}};

      for (int64_t i = 0; i < (int64_t)params.size(); ++i) {
         if (i < 4) {
            auto it = paras.find(i);
            check(it != paras.end(), "no  parameter");
            it->second = params[i];
         } else {
            auto itb = boolparas.find(i);
            check(itb != boolparas.end(), "no  parameter");
            itb->second = !!params[i];
         }
      }

      if (existing != statstable.end()) {
         statstable.modify(*existing, same_payer, [&](auto& ss) { ss = s; });
      } else {
         statstable.emplace(get_self(), [&](auto& ss) { ss = s; });
      }

       asset in     = asset(1000, symbol);
      asset out    = asset(1000, symbol);
      asset inres  = estimate_fee_given_in(get_self(), in);
      asset outres = estimate_fee_given_out(get_self(), out);
   }

   static bool is_exist_symbol(const symbol_code& sym_code, const name contract = "roxe.ro"_n) {
      stats statstable(contract, sym_code.raw());
      auto  existing = statstable.find(sym_code.raw());
      return existing != statstable.end();
   }

   static asset estimate_fee_given_in(const name& token_contract_account, const asset& amount_in) {
      auto sym = amount_in.symbol;
      check(sym.is_valid(), "invalid symbol name");
      check(amount_in.is_valid(), "invalid supply");
      check(amount_in.amount > 0, "amount_in must be positive");

      stats statstable(token_contract_account, sym.code().raw());
      auto  existing = statstable.find(sym.code().raw());
      check(existing != statstable.end(), "token with symbol not exists");
      const auto& st      = *existing; // statstable.get(sym.code().raw());
      symbol      fee_sym = st.useroc ? core_symbol : st.supply.symbol;
      int64_t fee_amount  = (st.fee * percent_decimal + amount_in.amount * st.percent) / (st.percent + percent_decimal);

      if (fee_amount < st.minfee)
         fee_amount = st.minfee;
      if (fee_amount > st.maxfee)
         fee_amount = st.maxfee;

      return asset(fee_amount, fee_sym);
   }

   static asset estimate_fee_given_out(const name& token_contract_account, const asset& amount_out) {
      auto sym = amount_out.symbol;
      check(sym.is_valid(), "invalid symbol name");
      check(amount_out.is_valid(), "invalid supply");
      check(amount_out.amount > 0, "amount_out must be positive");

      stats statstable(token_contract_account, sym.code().raw());
      auto  existing = statstable.find(sym.code().raw());
      check(existing != statstable.end(), "token with symbol not exists");
      const auto& st         = *existing; // statstable.get(sym.code().raw());
      symbol      fee_sym    = st.useroc ? core_symbol : st.supply.symbol;
      int64_t     fee_amount = st.fee + amount_out.amount * st.percent / percent_decimal;

      if (fee_amount < st.minfee)
         fee_amount = st.minfee;
      if (fee_amount > st.maxfee)
         fee_amount = st.maxfee;

      return asset(fee_amount, fee_sym);
   }

   // using create_action = eosio::action_wrapper<"create"_n, &tokenize::create>;
   // using issue_action = eosio::action_wrapper<"issue"_n, &tokenize::issue>;
   // using retire_action = eosio::action_wrapper<"retire"_n, &tokenize::retire>;
   // using transfer_action = eosio::action_wrapper<"transfer"_n, &tokenize::transfer>;
   // using open_action = eosio::action_wrapper<"open"_n, &tokenize::open>;
   // using close_action = eosio::action_wrapper<"close"_n, &tokenize::close>;
   // using setfee_action = eosio::action_wrapper<"setfee"_n, &tokenize::setfee>;
 private:
   struct [[eosio::table("accounts")]] account {
      asset balance;

      uint64_t primary_key() const { return balance.symbol.code().raw(); }
   };

   struct [[eosio::table("stat")]] currency_stats {
      asset        supply;
      asset        max_supply;
      name         issuer;
      vector<name> authors;

      int64_t fee;
      bool    fixed;
      int64_t percent;
      int64_t maxfee;
      int64_t minfee;
      bool    useroc;

      uint64_t primary_key() const { return supply.symbol.code().raw(); }
      EOSLIB_SERIALIZE(
          currency_stats, (supply)(max_supply)(issuer)(authors)(fee)(fixed)(percent)(maxfee)(minfee)(useroc))
   };

   typedef eosio::multi_index<"accounts"_n, account>    accounts;
   typedef eosio::multi_index<"stat"_n, currency_stats> stats;

   void sub_balance(const name& owner, const asset& value);

   void add_balance(const name& owner, const asset& value, const name& ram_payer);

 public:
   /**
    * default transaction fee
    */
   static constexpr int64_t default_tx_fee  = 0; // actural fee = tx_fee / precision =1/10000 (0.0001)
   static constexpr int64_t percent_decimal = 1000000;
};

} // namespace eosio
