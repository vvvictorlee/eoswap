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
#include <common/transfer_mgmt.hpp>
#include <eoswap/BFactory.hpp>
#include <eoswap/BPool.hpp>

using eosio::action;
using eosio::asset;
using eosio::name;
using eosio::permission_level;

class [[eosio::contract("eoswap")]] eoswap : public eosio::contract {
private:
  BFactory factory;
  BPool pool;
  BToken _token_;
  transfer_mgmt transfer_mgmts;

public:
  eoswap(name s, name code, eosio::datastream<const char *> ds)
      : contract(s, code, ds), factory(s, pool), pool(s, _token_), _token_(s),
        transfer_mgmts(s) {}

  //////////////////factory////////////////////////
  [[eosio::action]] void setblabs(name msg_sender, name blabs) {
    require_auth(msg_sender);
    factory.setBLabs(msg_sender, blabs);
  }

  [[eosio::action]] void collect(name msg_sender, name pool_name) {
    factory.collect(msg_sender, pool_name);
  }

  [[eosio::action]] void newpool(name msg_sender, name pool_name) {
    factory.newBPool(msg_sender, pool_name);
  }

  //////////////////POOL////////////////////////
  [[eosio::action]] void setswapfee(name msg_sender, name pool_name,
                                    uint swapFee) {
    pool.auth(msg_sender, pool_name);
    pool.setSwapFee(swapFee);
  }

  [[eosio::action]] void setcontroler(name msg_sender, name pool_name,
                                      name manager) {

    pool.auth(msg_sender, pool_name);
    pool.setController(manager);
  }

  [[eosio::action]] void setpubswap(name msg_sender, name pool_name,
                                    bool public_) {

    pool.auth(msg_sender, pool_name);
    pool.setPublicSwap(public_);
  }

  [[eosio::action]] void finalize(name msg_sender, name pool_name) {
    pool.auth(msg_sender, pool_name);
    pool.finalize(msg_sender);
  }
  // _lock_  Bind does not lock because it jumps to `rebind`, which does

  [[eosio::action]] void bind(name msg_sender, name pool_name, name token,
                              uint balance, uint denorm) {

    pool.auth(msg_sender, pool_name);
    pool.bind(token, balance, denorm);
  }
  [[eosio::action]] void rebind(name msg_sender, name pool_name, name token,
                                uint balance, uint denorm) {

    pool.auth(msg_sender, pool_name);
    pool.rebind(token, balance, denorm);
  }

  [[eosio::action]] void unbind(name msg_sender, name pool_name, name token) {

    pool.auth(msg_sender, pool_name);
    pool.unbind(token);
  }

  // Absorb any _token_ that have been sent to this contract into the pool

  [[eosio::action]] void gulp(name msg_sender, name pool_name, name token) {

    pool.auth(msg_sender, pool_name);
    pool.gulp(token);
  }

  [[eosio::action]] void joinpool(name msg_sender, name pool_name,
                                  uint poolAmountOut,
                                  std::vector<uint> maxAmountsIn) {

    pool.auth(msg_sender, pool_name);
    pool.joinPool(poolAmountOut, maxAmountsIn);
  }

  [[eosio::action]] void exitpool(name msg_sender, name pool_name,
                                  uint poolAmountIn,
                                  std::vector<uint> minAmountsOut) {

    pool.auth(msg_sender, pool_name);
    pool.exitPool(poolAmountIn, minAmountsOut);
  }

  [[eosio::action]] void swapamtin(name msg_sender, name pool_name,name tokenIn, uint tokenAmountIn,
                                          name tokenOut, uint minAmountOut,
                                          uint maxPrice) {

    pool.auth(msg_sender, pool_name);
    pool.swapExactAmountIn( tokenIn,  tokenAmountIn,
                                           tokenOut,  minAmountOut,
                                           maxPrice);
  }

  [[eosio::action]] void swapamtout(name msg_sender, name pool_name,name tokenIn, uint maxAmountIn,
                                           name tokenOut, uint tokenAmountOut,
                                           uint maxPrice) {

    pool.auth(msg_sender, pool_name);
    pool.swapExactAmountOut( tokenIn,  maxAmountIn,
                                           tokenOut,  tokenAmountOut,
                                           maxPrice);
  }


  ////////////////// TEST pool TOKEN////////////////////////

  [[eosio::action]] void approve(name msg_sender, name token, name dst,
                                 uint amt) {
    _token_.initToken(msg_sender, token);
    _token_.approve(msg_sender, dst, amt);
  }

  [[eosio::action]] void transfer(name msg_sender, name token, name dst,
                                  uint amt) {
    _token_.initToken(msg_sender, token);
    _token_.transfer(msg_sender, dst, amt);
  }

  [[eosio::action]] void transferfrom(name msg_sender, name token, name src,
                                      name dst, uint amt) {
    _token_.initToken(msg_sender, token);
    _token_.transferFrom(msg_sender, src, dst, amt);
  }

  [[eosio::action]] void incapproval(name msg_sender, name token, name dst,
                                     uint amt) {
    _token_.initToken(msg_sender, token);
    _token_.increaseApproval(msg_sender, dst, amt);
  }

  [[eosio::action]] void decapproval(name msg_sender, name token, name dst,
                                     uint amt) {
    _token_.initToken(msg_sender, token);
    _token_.decreaseApproval(msg_sender, dst, amt);
  }
  /////test interface /////
  [[eosio::action]] void mint(name msg_sender, name token, uint amt) {
    print(msg_sender);
    _token_.initToken(msg_sender, token);
    _token_._mint(amt);
  }

  [[eosio::action]] void burn(name msg_sender, name token, uint amt) {
    _token_.initToken(msg_sender, token);
    _token_._burn(amt);
  }

  [[eosio::action]] void move(name token, name src, name dst, uint amt) {
    _token_.initToken(src, token);
    _token_._move(src, dst, amt);
  }

  ////////////////////on_notify////////////////////
  [[eosio::on_notify("eosio.token::transfer")]] void on_transfer(
      name from, name to, asset quantity, std::string memo) {
    check(get_first_receiver() == "eosio.token"_n, "should be eosio.token");
    print_f("On notify : % % % %", from, to, quantity, memo);
    transfer_mgmts.eosiotoken_transfer(from, to, quantity, memo,
                                       [&](const auto &ad) {
                                         //       if (ad.action.size() == 0) {
                                         //         return;
                                         //       }

                                         //       if (ad.action == ta_knt) {
                                         //         int type =
                                         //         atoi(ad.param.c_str());
                                         //         knight_controller.hireknight(ad.from,
                                         //         type, ad.quantity);
                                         //         admin_controller.add_revenue(ad.quantity,
                                         //         rv_knight);
                                         //       } 
                                       });
  }

  [[eosio::on_notify("*::transfer")]] void on_transfer_by_non(
      name from, name to, asset quantity, std::string memo) {
    check(get_first_receiver() != "eosio.token"_n, "should not be eosio.token");
    print_f("On notify 2 : % % % %", from, to, quantity, memo);
    transfer_mgmts.non_eosiotoken_transfer(from, to, quantity, memo,
                                           [&](const auto &ad) {
                                             //       if (ad.action.size() == 0)
                                             //       {
                                             //         return;
                                             //       }

                                             //       if (ad.action == ta_knt) {
                                             //         int type =
                                             //         atoi(ad.param.c_str());
                                             //         knight_controller.hireknight(ad.from,
                                             //         type, ad.quantity);
                                             //         admin_controller.add_revenue(ad.quantity,
                                             //         rv_knight);
                                             //       } 
                                           

                                           });
  }
};
