#include <eosio/asset.hpp>
#include <eosio/eosio.hpp>
#include <eosio/name.hpp>
#include <eosio/symbol.hpp>
#include <eosio/system.hpp>
#include <eosio/transaction.hpp>
#include <string>
#include <vector>

#include <cmath>
#include <common/defines.hpp>
#include <eosdos/DODOEthProxy.hpp>

using eosio::action;
using eosio::asset;
using eosio::name;
using eosio::permission_level;

class [[eosio::contract("eosdos")]] eosdos : public eosio::contract {
 private:
   DODOEthProxy proxy;

 public:
   eosdos(name s, name code, eosio::datastream<const char*> ds)
       : contract(s, code, ds)
       , proxy(s) {}

   //////////////////zoo////////////////////////
   [[eosio::action]] void adddodo(name msg_sender, address _dodo) {
      proxy.setMsgSender(msg_sender);
      proxy.getZoo().addDODO(_dodo);
   }

   [[eosio::action]] void removedodo(name msg_sender, address _dodo) {
      proxy.setMsgSender(msg_sender);
      proxy.getZoo().removeDODO(_dodo);
   }

   [[eosio::action]] void breeddodo(
       name msg_sender, address maintainer, const extended_symbol& baseToken, const extended_symbol& quoteToken,
       const extended_symbol& oracle, uint256 lpFeeRate, uint256 mtFeeRate, uint256 k, uint256 gasPriceLimit) {
      proxy.setMsgSender(msg_sender);
      proxy.getZoo().breedDODO(maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
   }

   //////////////////proxy////////////////////////
   [[eosio::action]] void sellethtoken(
       name msg_sender, const extended_asset& ethToken, const extended_asset& minReceiveToken) {
      proxy.setMsgSender(msg_sender);
      proxy.sellEthToToken(ethToken, minReceiveToken);
   }

   [[eosio::action]] void buyeth1token(
       name msg_sender, const extended_asset& ethToken, const extended_asset& maxPayTokenAmount) {
      proxy.setMsgSender(msg_sender);
      proxy.buyEthWithToken(ethToken, maxPayTokenAmount);
   }

   [[eosio::action]] void selltokeneth(
       name msg_sender, const extended_asset& baseToken, const extended_asset& minReceiveEth) {

      proxy.setMsgSender(msg_sender);
      proxy.sellTokenToEth(baseToken, minReceiveEth);
   }

   [[eosio::action]] void buytoken1eth(
       name msg_sender, const extended_asset& baseToken, const extended_asset& maxPayEthAmount) {
      proxy.setMsgSender(msg_sender);
      proxy.buyTokenWithEth(baseToken, maxPayEthAmount);
   }
   // _lock_  Bind does not lock because it jumps to `rebind`, which does

   [[eosio::action]] void depositethab(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {

      proxy.setMsgSender(msg_sender);
      proxy.depositEthAsBase(ethtokenamount, quoteToken);
   }

   [[eosio::action]] void withdraweab(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsBase(ethtokenamount, quoteToken);
   }

   [[eosio::action]] void withdrawaeab(name msg_sender, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawAllEthAsBase(quoteToken);
   }

   // Absorb any _token_ that have been sent to this contract into the pool
   [[eosio::action]] void depositethaq(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.depositEthAsQuote(ethtokenamount, baseToken);
   }

   [[eosio::action]] void withdraweaq(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsQuote(ethtokenamount, baseToken);
   }

   [[eosio::action]] void withdrawaeaq(name msg_sender, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawAllEthAsQuote(baseToken);
   }

   ////////////////////   Oracle////////////////////////
   [[eosio::action]] void neworacle(name msg_sender, const extended_symbol& token) {
      proxy.setMsgSender(msg_sender);
      proxy.getZoo().get_storage_mgmt().newOracleStore(token);
   }

   [[eosio::action]] void setprice(name msg_sender, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      proxy.getZoo().get_oracle(amt.get_extended_symbol(), [&](auto& oracle) { oracle.setPrice(amt); });
   }

   ////////////////////  TOKEN////////////////////////
   [[eosio::action]] void extransfer(name from, name to, extended_asset quantity, std::string memo) {
      proxy.getZoo().get_transfer_mgmt().transfer(from, to, quantity, memo);
   }

   [[eosio::action]] void newtoken(name msg_sender, const extended_asset& token) {
      proxy.setMsgSender(msg_sender);
      proxy.getZoo().get_transfer_mgmt().create(msg_sender, token);
   }

   [[eosio::action]] void transfer(name msg_sender, name dst, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      proxy.getZoo().token(
          to_namesym(amt.get_extended_symbol()), [&](auto& _token_) { _token_.transfer(dst, amt.quantity.amount); });
   }

   //    /////test interface /////
   [[eosio::action]] void mint(name msg_sender, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      proxy.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
         _token_._mint(amt.quantity.amount);
         proxy.get_transfer_mgmt().issue(msg_sender, amt, "");
      });
   }

   //    [[eosio::action]] void burn(name msg_sender, const extended_asset& amt) {
   //       proxy.setMsgSender(msg_sender);
   //       proxy.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
   //          _token_._burn(amt.quantity.amount);
   //          proxy.get_transfer_mgmt().burn(msg_sender, amt,"");
   //       });
   //    }

   //    [[eosio::action]] void move(name msg_sender, name dst, const extended_asset& amt)
   //    {
   //       proxy.setMsgSender(msg_sender);
   //       proxy.token(to_namesym(amt.get_extended_symbol()), [&](auto& _token_) {
   //          _token_._move(msg_sender, dst, amt.quantity.amount);
   //       });
   //    }

   ////////////////////on_notify////////////////////
   [[eosio::on_notify("eosio.token::transfer")]] void on_transfer(
       name from, name to, asset quantity, std::string memo) {
      check(get_first_receiver() == "eosio.token"_n, "should be eosio.token");
      print_f("On notify : % % % %", from, to, quantity, memo);
      proxy.getZoo().get_transfer_mgmt().eosiotoken_transfer(from, to, quantity, memo, [&](const auto& action_event) {
         if (action_event.action.empty()) {
            return;
         }

         if (action_event.action.compare(bind_action_string) == 0) {
            auto           paras = transfer_mgmt::parse_string(action_event.param);
            name           pool_name;
            extended_asset balance;
            uint256        denorm;
            proxy.setMsgSender(action_event.msg_sender);
            // proxy.pool(pool_name, [&](auto& pool) { pool.bind(balance, denorm); });
         }
      });
   }

   [[eosio::on_notify("*::transfer")]] void on_transfer_by_non(name from, name to, asset quantity, std::string memo) {
      check(get_first_receiver() != "eosio.token"_n, "should not be eosio.token");
      print_f("On notify 2 : % % % %", from, to, quantity, memo);
      proxy.getZoo().get_transfer_mgmt().non_eosiotoken_transfer(
          from, to, quantity, memo, [&](const auto& action_event) {

          });
   }
};
