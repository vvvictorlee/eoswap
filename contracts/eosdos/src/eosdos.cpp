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
      _instance_mgmt.newDODO(msg_sender,
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

   [[eosio::action]] void init(name msg_sender, address dodoZoo, const extended_symbol& weth, const extended_symbol& core_symbol) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      proxy.init(dodoZoo, weth,core_symbol);
   }

   [[eosio::action]] void sellethtoken(
       name msg_sender, const extended_asset& ethToken, const extended_asset& minReceiveToken) {
      proxy.setMsgSender(msg_sender);
      proxy.sellEthToToken(ethToken, minReceiveToken);

   }
/////////////////////////////////////////
   [[eosio::action]] void buyethtoken(
       name msg_sender, const extended_asset& ethToken, const extended_asset& maxPayTokenAmount) {
      proxy.setMsgSender(msg_sender);
    //   proxy.buyEthWithToken(ethToken, maxPayTokenAmount);
      buyethtoken1_action buyethtoken1_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      buyethtoken1_act.send( msg_sender, ethToken,maxPayTokenAmount );
      buyethtoken2_action buyethtoken2_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      buyethtoken2_act.send( msg_sender, ethToken,maxPayTokenAmount );
   }

   [[eosio::action]] void buyethtoken1(
       name msg_sender, const extended_asset& ethToken, const extended_asset& maxPayTokenAmount) {
      proxy.setMsgSender(msg_sender);
      proxy.buyEthWithToken(ethToken, maxPayTokenAmount,true);
   }

   [[eosio::action]] void buyethtoken2(
       name msg_sender, const extended_asset& ethToken, const extended_asset& maxPayTokenAmount) {
      proxy.setMsgSender(msg_sender);
      proxy.buyEthWithToken(ethToken, maxPayTokenAmount);
   }

    using buyethtoken1_action = eosio::action_wrapper<"buyethtoken1"_n, &eosdos::buyethtoken1>;
    using buyethtoken2_action = eosio::action_wrapper<"buyethtoken2"_n, &eosdos::buyethtoken2>;
/////////////////////////////////////////////
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
    //   proxy.withdrawEthAsBase(ethtokenamount, quoteToken);
      withdraweab1_action withdraweab1_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdraweab1_act.send( msg_sender, ethtokenamount, quoteToken );
      withdraweab2_action withdraweab2_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdraweab2_act.send( msg_sender, ethtokenamount, quoteToken );
      withdraweab3_action withdraweab3_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdraweab3_act.send( msg_sender, ethtokenamount, quoteToken );
   }

   [[eosio::action]] void withdraweab1(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsBase(ethtokenamount, quoteToken,1);
   }

   [[eosio::action]] void withdraweab2(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsBase(ethtokenamount, quoteToken,2);
   }

   [[eosio::action]] void withdraweab3(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsBase(ethtokenamount, quoteToken,3);
   }

    using withdraweab1_action = eosio::action_wrapper<"withdraweab1"_n, &eosdos::withdraweab1>;
    using withdraweab2_action = eosio::action_wrapper<"withdraweab2"_n, &eosdos::withdraweab2>;
    using withdraweab3_action = eosio::action_wrapper<"withdraweab3"_n, &eosdos::withdraweab3>;

   [[eosio::action]] void withdrawaeab(name msg_sender, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
    //   proxy.withdrawAllEthAsBase(quoteToken);
      withdrawaeaa_action withdrawaeaa_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdrawaeaa_act.send( msg_sender,  quoteToken );
      withdrawaeac_action withdrawaeac_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdrawaeac_act.send( msg_sender,  quoteToken );
   }

   [[eosio::action]] void withdrawaeaa(name msg_sender, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawAllEthAsBase(quoteToken,true);
   }
   [[eosio::action]] void withdrawaeac(name msg_sender, const extended_symbol& quoteToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawAllEthAsBase(quoteToken);
   }
    using withdrawaeaa_action = eosio::action_wrapper<"withdrawaeaa"_n, &eosdos::withdrawaeaa>;
    using withdrawaeac_action = eosio::action_wrapper<"withdrawaeac"_n, &eosdos::withdrawaeac>;

   // Absorb any _token_ that have been sent to this contract into the pool
   [[eosio::action]] void depositethaq(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.depositEthAsQuote(ethtokenamount, baseToken);
   }

   [[eosio::action]] void withdraweaq(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
    //   proxy.withdrawEthAsQuote(ethtokenamount, baseToken);
      withdraweaq1_action withdraweaq1_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdraweaq1_act.send( msg_sender, ethtokenamount, baseToken );
      withdraweaq2_action withdraweaq2_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdraweaq2_act.send( msg_sender, ethtokenamount, baseToken );
      withdraweaq3_action withdraweaq3_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdraweaq3_act.send( msg_sender, ethtokenamount, baseToken );
   }

   [[eosio::action]] void withdraweaq1(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsQuote(ethtokenamount, baseToken,1);
   }

   [[eosio::action]] void withdraweaq2(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsQuote(ethtokenamount, baseToken,2);
   }

   [[eosio::action]] void withdraweaq3(
       name msg_sender, const extended_asset& ethtokenamount, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawEthAsQuote(ethtokenamount, baseToken,3);
   }

   using withdraweaq1_action = eosio::action_wrapper<"withdraweaq1"_n, &eosdos::withdraweaq1>;
    using withdraweaq2_action = eosio::action_wrapper<"withdraweaq2"_n, &eosdos::withdraweaq2>;
  using withdraweaq3_action = eosio::action_wrapper<"withdraweaq3"_n, &eosdos::withdraweaq3>;

   [[eosio::action]] void withdrawaeaq(name msg_sender, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
    //   proxy.withdrawAllEthAsQuote(baseToken);
      withdrawaeao_action withdrawaeao_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdrawaeao_act.send( msg_sender,  baseToken );
      withdrawaeap_action withdrawaeap_act{ "eosdoseosdos"_n, { {msg_sender, "active"_n} } };
      withdrawaeap_act.send( msg_sender,  baseToken );
   }
 
   [[eosio::action]] void withdrawaeao(name msg_sender, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawAllEthAsQuote(baseToken,true);
   }
 
   [[eosio::action]] void withdrawaeap(name msg_sender, const extended_symbol& baseToken) {
      proxy.setMsgSender(msg_sender);
      proxy.withdrawAllEthAsQuote(baseToken);
   }
 
   using withdrawaeao_action = eosio::action_wrapper<"withdrawaeao"_n, &eosdos::withdrawaeao>;
    using withdrawaeap_action = eosio::action_wrapper<"withdrawaeap"_n, &eosdos::withdrawaeap>;

   ////////////////////  admin dodo////////////////////////
   [[eosio::action]] void enabletradin(name msg_sender, name dodo_name) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_dodo(msg_sender,dodo_name, [&](auto& dodo) { dodo.enableTrading(); });
   }

   [[eosio::action]] void enablequodep(name msg_sender, name dodo_name) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_dodo(msg_sender,dodo_name, [&](auto& dodo) { dodo.enableQuoteDeposit(); });
   }

   [[eosio::action]] void enablebasdep(name msg_sender, name dodo_name) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_dodo(msg_sender,dodo_name, [&](auto& dodo) { dodo.enableBaseDeposit(); });
   }

   ////////////////////  LiquidityProvider dodo////////////////////////
   [[eosio::action]] void depositquote(name msg_sender, name dodo_name, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      //   DODOStore& dodoStore = _instance_mgmt.get_storage_mgmt().get_dodo_store(dodo_name);
      //   DODO       dodo(dodoStore, zoo);
      //   dodo.setMsgSender(msg_sender);
      //   dodo.depositQuote(amt.quantity.amount);
      _instance_mgmt.get_dodo(msg_sender,dodo_name, [&](auto& dodo) { (void)dodo.depositQuote(amt.quantity.amount); });
   }

   ////////////////////   Oracle////////////////////////
   [[eosio::action]] void neworacle(name msg_sender, const extended_symbol& token) {
      check(oracle_account == msg_sender, "no oracle admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.newOracle(msg_sender,token);
   }

   [[eosio::action]] void setprice(name msg_sender, const extended_asset& amt) {
      check(oracle_account == msg_sender, "no oracle admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_oracle(msg_sender,amt.get_extended_symbol(), [&](auto& oracle) { oracle.setPrice(amt); });
   }

   ////////////////////  TOKEN////////////////////////
   [[eosio::action]] void extransfer(name from, name to, extended_asset quantity, std::string memo) {
      transfer_mgmt::static_transfer(from, to, quantity, memo);
   }

   [[eosio::action]] void newtoken(name msg_sender, const extended_asset& token) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.newToken<TestERC20>(msg_sender,token);
   }

   [[eosio::action]] void newethtoken(name msg_sender, const extended_asset& token) {
      check(_self == msg_sender, "no  admin");
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.newToken<WETH9>(msg_sender,token);
   }

   //    /////test interface /////
   [[eosio::action]] void mint(name msg_sender, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_token<TestERC20>(msg_sender,amt.get_extended_symbol(), [&](auto& _token_) {
         _token_.mint(msg_sender, amt.quantity.amount);
      });
   }

   [[eosio::action]] void mintweth(name msg_sender, const extended_asset& amt) {
      proxy.setMsgSender(msg_sender);
      _instance_mgmt.get_token<WETH9>(msg_sender,amt.get_extended_symbol(), [&](auto& _token_) {
         _token_.mint(msg_sender, amt.quantity.amount);
      });
   }




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
        print_f("On notify 2 :% % % % %", get_first_receiver(),from, to, quantity, memo);
      _instance_mgmt.get_transfer_mgmt().non_eosiotoken_transfer(
          from, to, quantity, memo, [&](const auto& action_event) {

          });
   }

};
