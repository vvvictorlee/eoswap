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
  BToken ttoken;

public:
  eoswap(name s, name code, eosio::datastream<const char *> ds)
      : contract(s, code, ds), factory(s), pool(s), ttoken(s) {}

  //////////////////factory////////////////////////
  [[eosio::action]] void setblabs(name msg_sender, name blabs) {
    require_auth(msg_sender);
    factory.setBLabs(msg_sender, blabs);
  }

  [[eosio::action]] void collect(name msg_sender, name pool_name) {
    require_auth(msg_sender);
    factory.collect(msg_sender, pool_name);
  }

  [[eosio::action]] void newpool(name msg_sender, name pool_name) {
    require_auth(msg_sender);
    factory.newBPool(msg_sender, pool_name, pool);
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

  // Absorb any tokens that have been sent to this contract into the pool

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

  ////////////////// TEST pool TOKEN////////////////////////

  [[eosio::action]] void approve(name msg_sender, name pool_name, name dst,
                                 uint amt) {
    pool.auth(msg_sender, pool_name);
    pool.approve(msg_sender, dst, amt);
  }

  [[eosio::action]] void transfer(name msg_sender, name pool_name, name dst,
                                  uint amt) {
    pool.auth(msg_sender, pool_name);
    pool.transfer(msg_sender, dst, amt);
  }

  [[eosio::action]] void transferfrom(name msg_sender, name pool_name, name src,
                                      name dst, uint amt) {
    pool.auth(msg_sender, pool_name);
    pool.transferFrom(msg_sender, src, dst, amt);
  }

  [[eosio::action]] void incapproval(name msg_sender, name pool_name, name dst,
                                     uint amt) {
    pool.auth(msg_sender, pool_name);
    pool.increaseApproval(msg_sender, dst, amt);
  }

  [[eosio::action]] void decapproval(name msg_sender, name pool_name, name dst,
                                     uint amt) {
    pool.auth(msg_sender, pool_name);
    pool.decreaseApproval(msg_sender, dst, amt);
  }
  /////test interface /////
  [[eosio::action]] void mint(name msg_sender, name pool_name, uint amt) {
    pool.auth(msg_sender, pool_name);
    pool._mint(amt);
  }

  [[eosio::action]] void burn(name msg_sender, name pool_name, uint amt) {
    pool.auth(msg_sender, pool_name);
    pool._burn(amt);
  }

  [[eosio::action]] void move(name pool_name, name src, name dst, uint amt) {
    pool.auth(src, pool_name);
    pool._move(src, dst, amt);
  }

  ////////////////// TEST TOKEN////////////////////////

  //   [[eosio::action]] void approve(name msg_sender, name pool_name, name dst,
  //                                  uint amt) {
  //     pool.auth(msg_sender, pool_name);
  //     pool.approve(msg_sender, dst, amt);
  //   }

  [[eosio::action]] void ttransfer(name msg_sender, name token, name dst,
                                   uint amt) {
    require_auth(msg_sender);
    ttoken.setToken(token);
    ttoken.transfer(msg_sender, dst, amt);
  }

  [[eosio::action]] void ttransferfrm(name msg_sender, name token, name src,
                                      name dst, uint amt) {
    require_auth(msg_sender);
    ttoken.setToken(token);
    ttoken.transferFrom(msg_sender, src, dst, amt);
  }

  //   [[eosio::action]] void incapproval(name msg_sender, name pool_name, name
  //   dst,
  //                                      uint amt) {
  //     pool.auth(msg_sender, pool_name);
  //     pool.increaseApproval(msg_sender, dst, amt);
  //   }

  //   [[eosio::action]] void decapproval(name msg_sender, name pool_name, name
  //   dst,
  //                                      uint amt) {
  //     pool.auth(msg_sender, pool_name);
  //     pool.decreaseApproval(msg_sender, dst, amt);
  //   }
  [[eosio::action]] void tmint(name msg_sender, name token, uint amt) {
    uint128_t tmp  = 0;
    require_auth(msg_sender);
    ttoken.setToken(token);
    ttoken._mint(amt);
  }

  //   [[eosio::action]] void burn(name msg_sender, name pool_name, uint amt) {
  //     pool.auth(msg_sender, pool_name);
  //     pool._burn(amt);
  //   }

  [[eosio::action]] void tmove(name token, name src, name dst, uint amt) {
    require_auth(src);
    ttoken.setToken(token);
    ttoken._move(src, dst, amt);
  }

  ////////////////////on_notify////////////////////
  [[eosio::on_notify("eosio.token::transfer")]] void transfer(
      name from, name to, asset quantity, std::string memo) {
    // system_controller.eosiotoken_transfer(
    //     from, to, quantity, memo, [&](const auto &ad) {
    //       if (ad.action.size() == 0) {
    //         return;
    //       }

    //       if (ad.action == ta_knt) {
    //         int type = atoi(ad.param.c_str());
    //         knight_controller.hireknight(ad.from, type, ad.quantity);
    //         admin_controller.add_revenue(ad.quantity, rv_knight);
    //       } else if (ad.action == ta_mw) {
    //         int pid = atoi(ad.param.c_str());
    //         player_controller.buymp(ad.from, pid, ad.quantity);
    //         admin_controller.add_revenue(ad.quantity, rv_mp);

    //       } else if (ad.action == ta_item) {
    //         asset tax = market_controller.buyitem(ad.from, ad,
    //         &item_controller,
    //                                               ig_count);
    //         admin_controller.add_revenue(tax, rv_item_tax);
    //         admin_controller.add_tradingvol(ad.quantity);
    //          } else if (ad.action == ta_mat) {
    //         asset tax = market_controller.buymat(
    //             ad.from, ad, &material_controller, ig_count);
    //         admin_controller.add_revenue(tax, rv_material_tax);
    //         admin_controller.add_tradingvol(ad.quantity);
    //          } else if (ad.action == ta_skin) {
    //         asset tax = skin_controller.skbuy(ad.from, ad);
    //         admin_controller.add_revenue(tax, rv_skin);
    //         admin_controller.add_tradingvol(ad.quantity);
    //       } else if (ad.action == ta_ivn) {
    //         if (ad.param == tp_item) {
    //           system_controller.itemivnup(ad.from, ad.quantity);
    //           admin_controller.add_revenue(ad.quantity,rv_item_iventory_up);
    //         } else if (ad.param == tp_mat) {
    //           system_controller.mativnup(ad.from, ad.quantity);
    //           admin_controller.add_revenue(ad.quantity,rv_mat_iventory_up);
    //         } else {
    //           assert_true(false, "invalid inventory");
    //         }
    //       } else {
    //         assert_true(false, "invalid transfer");
    //       }

    //       admin_controller.autodividend();
    //     });
  }
};
