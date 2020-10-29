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
   instance_mgmt _instance_mgmt;
   DODOZoo       zoo;
   DODOEthProxy  proxy;

 public:
   static constexpr eosio::name     oracle_account{"eosdoseosdos"_n};
   static constexpr extended_symbol weth_symbol = {symbol(symbol_code("WETH"), 4), "eosdosxtoken"_n};

   eosdos(name s, name code, eosio::datastream<const char*> ds)
       : contract(s, code, ds)
       , _instance_mgmt(s)
       , zoo(s, _instance_mgmt)
       , proxy(s, _instance_mgmt, zoo) {
      zoo.init(_self, _self, _self);
   }

   //////////////////zoo////////////////////////
   [[eosio::action]] void newdodo(
       name msg_sender, name dodo_name, address owner, address supervisor, address maintainer,
       const extended_symbol& baseToken, const extended_symbol& quoteToken, const extended_symbol& oracle,
       uint256 lpFeeRate, uint256 mtFeeRate, uint256 k, uint256 gasPriceLimit) {
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.newDODO(
          dodo_name, owner, supervisor, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k,
          gasPriceLimit);
   }

   [[eosio::action]] void adddodo(name msg_sender, address _dodo) {
      proxy.setMsgSender(msg_sender);
      zoo.addDODO(_dodo);
   }

   [[eosio::action]] void removedodo(name msg_sender, address _dodo) {
      proxy.setMsgSender(msg_sender);
      zoo.removeDODO(_dodo);
   }

   [[eosio::action]] void breeddodo(
       name msg_sender, name dodo_name, address maintainer, const extended_symbol& baseToken,
       const extended_symbol& quoteToken, const extended_symbol& oracle, uint256 lpFeeRate, uint256 mtFeeRate,
       uint256 k, uint256 gasPriceLimit) {
      proxy.setMsgSender(msg_sender);
      zoo.breedDODO(dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
   }

   //////////////////proxy////////////////////////

   [[eosio::action]] void init(name msg_sender, address dodoZoo, const extended_symbol& weth) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      proxy.init(dodoZoo, weth);
   }

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
   ////////////////////  admin dodo////////////////////////
   [[eosio::action]] void enabletradin(name msg_sender, name dodo_name) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_dodo(dodo_name, [&](auto& dodo) { dodo.enableTrading(); });
   }

   [[eosio::action]] void enablequodep(name msg_sender, name dodo_name) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_dodo(dodo_name, [&](auto& dodo) { dodo.enableQuoteDeposit(); });
   }

   [[eosio::action]] void enablebasdep(name msg_sender, name dodo_name) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_dodo(dodo_name, [&](auto& dodo) { dodo.enableBaseDeposit(); });
   }

   ////////////////////  LiquidityProvider dodo////////////////////////
   [[eosio::action]] void depositquote(name msg_sender, name dodo_name, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      //   DODOStore& dodoStore = _instance_mgmt.get_storage_mgmt().get_dodo_store(dodo_name);
      //   DODO       dodo(dodoStore, zoo);
      //   dodo.setMsgSender(msg_sender);
      //   dodo.depositQuote(amt.quantity.amount);
      _instance_mgmt.get_dodo(dodo_name, [&](auto& dodo) { (void)dodo.depositQuote(amt.quantity.amount); });
   }

   ////////////////////   Oracle////////////////////////
   [[eosio::action]] void neworacle(name msg_sender, const extended_symbol& token) {
      check(oracle_account == msg_sender, "no oracle admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.newOracle(token);
   }

   [[eosio::action]] void setprice(name msg_sender, const extended_asset& amt) {
      check(oracle_account == msg_sender, "no oracle admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_oracle(amt.get_extended_symbol(), [&](auto& oracle) { oracle.setPrice(amt); });
   }

   ////////////////////  TOKEN////////////////////////
   [[eosio::action]] void extransfer(name from, name to, extended_asset quantity, std::string memo) {
      _instance_mgmt.get_transfer_mgmt().transfer(from, to, quantity, memo);
   }

   [[eosio::action]] void newtoken(name msg_sender, const extended_asset& token) {
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.newToken(token);
      _instance_mgmt.get_transfer_mgmt().create(msg_sender, token);
   }

   [[eosio::action]] void newethtoken(name msg_sender, const extended_asset& token) {
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.newEthToken(token);
      _instance_mgmt.get_transfer_mgmt().create(msg_sender, token);
   }

   //    /////test interface /////
   [[eosio::action]] void mint(name msg_sender, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_token<TestERC20>(amt.get_extended_symbol(), [&](auto& _token_) {
         _token_.mint(msg_sender, amt.quantity.amount);
         _instance_mgmt.get_transfer_mgmt().issue(msg_sender, amt, "");
      });
   }

   [[eosio::action]] void mintweth(name msg_sender, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_token<WETH9>(amt.get_extended_symbol(), [&](auto& _token_) {
         _token_.mint(msg_sender, amt.quantity.amount);
         _instance_mgmt.get_transfer_mgmt().issue(msg_sender, amt, "");
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
      _instance_mgmt.get_transfer_mgmt().eosiotoken_transfer(from, to, quantity, memo, [&](const auto& action_event) {
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
      _instance_mgmt.get_transfer_mgmt().non_eosiotoken_transfer(
          from, to, quantity, memo, [&](const auto& action_event) {

          });
   }
};
