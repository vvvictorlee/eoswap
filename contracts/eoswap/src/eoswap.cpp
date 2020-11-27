#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/symbol.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <string>
#include <vector>

#include <cmath>
#include <common/BType.hpp>
#include <common/extended_token.hpp>
#include <eoswap/BFactory.hpp>
#include <eoswap/BPool.hpp>
#include <storage/BFactoryTable.hpp>
#include <storage/BPoolTable.hpp>
#include <storage/BTokenTable.hpp>
#include "extended_token.cpp"

using eosio::action;
using eosio::asset;
using eosio::name;
using eosio::permission_level;
using std::string;

class [[eosio::contract("eoswap")]] eoswap : public eosio::contract {
 private:
   BFactory       factory;
   extendedtoken::extended_token extoken;

 public:
   eoswap(name s, name code, eosio::datastream<const char*> ds)
       : contract(s, code, ds)
       , factory(s)
       , extoken(s) {}

   //////////////////factory////////////////////////
   [[eosio::action]] void setblabs(name msg_sender, name blabs) {
      factory.setMsgSender(msg_sender);
      factory.setBLabs(blabs);
   }

   [[eosio::action]] void collect(name msg_sender, name pool_name) {
      factory.setMsgSender(msg_sender);
      factory.collect(pool_name);
   }

   [[eosio::action]] void newpool(name msg_sender, name pool_name) {
      factory.setMsgSender(msg_sender);
      factory.newBPool(pool_name);
   }

   //////////////////POOL////////////////////////
   [[eosio::action]] void setswapfee(name msg_sender, name pool_name, uint swapFee) {
      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.setSwapFee(swapFee); });
   }

   [[eosio::action]] void setcontroler(name msg_sender, name pool_name, name manager) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.setController(manager); });
   }

   [[eosio::action]] void setpubswap(name msg_sender, name pool_name, bool public_) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.setPublicSwap(public_); });
   }

   [[eosio::action]] void finalize(name msg_sender, name pool_name) {
      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.finalize(); });
   }
   // _lock_  Bind does not lock because it jumps to `rebind`, which does

   [[eosio::action]] void bind(name msg_sender, name pool_name, const extended_asset& balance, uint denorm) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.bind(balance, denorm); });
   }

   [[eosio::action]] void rebind(name msg_sender, name pool_name, const extended_asset& balance, uint denorm) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.rebind(balance, denorm); });
   }

   [[eosio::action]] void unbind(name msg_sender, name pool_name, const extended_symbol& token) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.unbind(token); });
   }

   // Absorb any _token_ that have been sent to this contract into the pool

   [[eosio::action]] void gulp(name msg_sender, name pool_name, const extended_symbol& token) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.gulp(token); });
   }

   [[eosio::action]] void joinpool(
       name msg_sender, name pool_name, uint poolAmountOut, std::vector<uint> maxAmountsIn) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.joinPool(poolAmountOut, maxAmountsIn); });
   }

   [[eosio::action]] void exitpool(
       name msg_sender, name pool_name, uint poolAmountIn, std::vector<uint> minAmountsOut) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.exitPool(poolAmountIn, minAmountsOut); });
   }

   [[eosio::action]] void swapamtin(
       name msg_sender, name pool_name, const extended_asset& tokenAmountIn, const extended_asset& minAmountOut,
       uint maxPrice) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.swapExactAmountIn(tokenAmountIn, minAmountOut, maxPrice); });
   }

   [[eosio::action]] void swapamtout(
       name msg_sender, name pool_name, const extended_asset& maxAmountIn, const extended_asset& tokenAmountOut,
       uint maxPrice) {

      factory.setMsgSender(msg_sender);
      factory.pool(pool_name, [&](auto& pool) { pool.swapExactAmountOut(maxAmountIn, tokenAmountOut, maxPrice); });
   }

   ////////////////// TEST pool TOKEN////////////////////////
   [[eosio::action]] void extransfer(name from, name to, extended_asset quantity, std::string memo) {
      factory.get_transfer_mgmt().transfer(from, to, quantity, memo);
   }

   [[eosio::action]] void newtoken(name msg_sender, const extended_asset& token) {
      factory.setMsgSender(msg_sender);
      factory.newToken(token);
      factory.get_transfer_mgmt().create(msg_sender, token);
   }

   [[eosio::action]] void approve(name msg_sender, name dst, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(
          to_namesym(amt.get_extended_symbol()), [&](auto& _token_) { _token_.approve(dst, amt.quantity.amount); });
   }

   [[eosio::action]] void transfer(name msg_sender, name dst, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(
          to_namesym(amt.get_extended_symbol()), [&](auto& _token_) { _token_.transfer(dst, amt.quantity.amount); });
   }

   [[eosio::action]] void transferfrom(name msg_sender, name src, name dst, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_.transferFrom(src, dst, amt.quantity.amount);
      });
   }

   [[eosio::action]] void incapproval(name msg_sender, name dst, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_.increaseApproval(dst, amt.quantity.amount);
      });
   }

   [[eosio::action]] void decapproval(name msg_sender, name dst, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_.decreaseApproval(dst, amt.quantity.amount);
      });
   }
   /////test interface /////
   [[eosio::action]] void mint(name msg_sender, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_._mint(amt.quantity.amount);
         factory.get_transfer_mgmt().issue(msg_sender, amt, "");
      });
   }

   [[eosio::action]] void burn(name msg_sender, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_._burn(amt.quantity.amount);
         factory.get_transfer_mgmt().burn(msg_sender, amt, "");
      });
   }

   [[eosio::action]] void move(name msg_sender, name dst, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_._move(msg_sender, dst, amt.quantity.amount);
      });
   }
   ////////////////////extended_token////////////////////

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
   [[eosio::action]] void create(const name& issuer, const extended_asset& maximum_supply) {
      extoken.create( issuer,  maximum_supply);
   }
   /**
    *  This action issues to `to` account a `quantity` of tokens.
    *
    * @param to - the account to issue tokens to, it must be the same as the issuer,
    * @param quntity - the amount of tokens to be issued,
    * @memo - the memo string that accompanies the token issue transaction.
    */
   [[eosio::action]] void issue(const name& to, const extended_asset& quantity, const string& memo) {
      extoken.issue( to,  quantity, memo);
   }

   /**
    * The opposite for create action, if all validations succeed,
    * it debits the statstable.supply amount.
    *
    * @param quantity - the quantity of tokens to retire,
    * @param memo - the memo string to accompany the transaction.
    */
   [[eosio::action]] void retire(const extended_asset& quantity, const string& memo) {
      extoken.retire( quantity,  memo);
   }

   /**
    * Allows `from` account to transfer to `to` account the `quantity` tokens.
    * One account is debited and the other is credited with quantity tokens.
    *
    * @param from - the account to transfer from,
    * @param to - the account to be transferred to,
    * @param quantity - the quantity of tokens to be transferred,
    * @param memo - the memo string to accompany the transaction.
    */
   [[eosio::action]] void transfer(
       const name& from, const name& to, const extended_asset& quantity, const string& memo) {
      extoken.transfer( from, to,  quantity,  memo);
   }
   /**
    * Allows `ram_payer` to create an account `owner` with zero balance for
    * token `symbol` at the expense of `ram_payer`.
    *
    * @param owner - the account to be created,
    * @param symbol - the token to be payed with by `ram_payer`,
    * @param ram_payer - the account that supports the cost of this action.
    *
    * More information can be read [here](https://github.com/EOSIO/eosio.contracts/issues/62)
    * and [here](https://github.com/EOSIO/eosio.contracts/issues/61).
    */
   [[eosio::action]] void open(const name& owner, const extended_symbol& symbol, const name& ram_payer) {
      extoken.open(owner, symbol,  ram_payer);
   }

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
   [[eosio::action]] void close(const name& owner, const extended_symbol& symbol) {
      extoken.close( owner,  symbol);
   }

   ////////////////////on_notify////////////////////
   [[eosio::on_notify("eosio.token::transfer")]] void on_transfer(
       name from, name to, asset quantity, std::string memo) {
      check(get_first_receiver() == "eosio.token"_n, "should be eosio.token");
      print_f("On notify : % % % %", from, to, quantity, memo);
      factory.get_transfer_mgmt().eosiotoken_transfer(from, to, quantity, memo, [&](const auto& action_event) {
         if (action_event.action.empty()) {
            return;
         }

         if (action_event.action.compare(bind_action_string) == 0) {
            auto           paras = transfer_mgmt::parse_string(action_event.param);
            name           pool_name;
            extended_asset balance;
            uint           denorm;
            factory.setMsgSender(action_event.msg_sender);
            factory.pool(pool_name, [&](auto& pool) { pool.bind(balance, denorm); });
         }
      });
   }

   [[eosio::on_notify("*::transfer")]] void on_transfer_by_non(name from, name to, asset quantity, std::string memo) {
      check(get_first_receiver() != "eosio.token"_n, "should not be eosio.token");
      print_f("On notify 2 : % % % %", from, to, quantity, memo);
      factory.get_transfer_mgmt().non_eosiotoken_transfer(from, to, quantity, memo, [&](const auto& action_event) {

      });
   }
};
