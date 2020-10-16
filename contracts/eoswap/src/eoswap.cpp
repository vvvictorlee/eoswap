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
#include <storage/BFactoryTable.hpp>
#include <storage/BPoolTable.hpp>
#include <storage/BTokenTable.hpp>
#include <eoswap/BFactory.hpp>
#include <eoswap/BPool.hpp>

using eosio::action;
using eosio::asset;
using eosio::name;
using eosio::permission_level;

class [[eosio::contract("eoswap")]] eoswap : public eosio::contract {
 private:
   BFactory factory;

 public:
   eoswap(name s, name code, eosio::datastream<const char*> ds)
       : contract(s, code, ds)
       , factory(s) {}

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
         factory.get_transfer_mgmt().issue(msg_sender, amt,"");
      });
   }

   [[eosio::action]] void burn(name msg_sender, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_._burn(amt.quantity.amount);
         factory.get_transfer_mgmt().burn(msg_sender, amt,"");
      });
   }

   [[eosio::action]] void move(name msg_sender, name dst, const extended_asset& amt) {
      factory.setMsgSender(msg_sender);
      factory.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_._move(msg_sender, dst, amt.quantity.amount);
      });
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
