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

public:
  eoswap(name s, name code, eosio::datastream<const char *> ds)
      : contract(s, code, ds), factory(s), pool(s) {}

  //////////////////factory////////////////////////

  [[eosio::action]] void setblabs(name msg_sender, name blabs) {
    require_auth(msg_sender);
    factory.setBLabs(msg_sender, blabs);
  }

  [[eosio::action]] void collect(name msg_sender, name pool) {
    require_auth(msg_sender);
    factory.collect(msg_sender, pool);
  }

  //////////////////POOL////////////////////////
  [[eosio::action]] void setswapfee(name msg_sender, uint64_t swapFee) {
    require_auth(msg_sender);
    pool.setSwapFee(msg_sender, swapFee);
  }

  [[eosio::action]] void setcontroler(name msg_sender, name manager) {
    require_auth(msg_sender);
    pool.setController(msg_sender, manager);
  }

  [[eosio::action]] void setpubswap(name msg_sender, bool public_) {
    require_auth(msg_sender);
    pool.setPublicSwap(msg_sender, public_);
  }

  [[eosio::action]] void finalize(name msg_sender) {
    require_auth(msg_sender);
    pool.finalize(msg_sender);
  }
  // _lock_  Bind does not lock because it jumps to `rebind`, which does

  [[eosio::action]] void bind(name msg_sender, name token, uint64_t balance,
                              uint64_t denorm) {
    require_auth(msg_sender);
    pool.bind(msg_sender, token, balance, denorm);
  }
  [[eosio::action]] void rebind(name msg_sender, name token, uint64_t balance,
                                uint64_t denorm) {
    require_auth(msg_sender);
    pool.rebind(msg_sender, token, balance, denorm);
  }

  [[eosio::action]] void unbind(name msg_sender, name token) {
    require_auth(msg_sender);
    pool.unbind(msg_sender, token);
  }

  // Absorb any tokens that have been sent to this contract into the pool

  [[eosio::action]] void gulp(name msg_sender,name token) {
    require_auth(msg_sender);
    pool.gulp(token);
  }

  [[eosio::action]] void joinpool(name msg_sender,uint64_t poolAmountOut,
                                  std::vector<uint64_t> maxAmountsIn) {
    require_auth(msg_sender);
    pool.joinPool(msg_sender,poolAmountOut, maxAmountsIn);
  }
  [[eosio::action]] void exitpool(name msg_sender,uint64_t poolAmountIn,
                                  std::vector<uint64_t> minAmountsOut) {
    require_auth(msg_sender);
    pool.exitPool(msg_sender,poolAmountIn, minAmountsOut);
  }

  //////////////////TOKEN////////////////////////

  [[eosio::action]] void approve(name msg_sender, name dst, uint amt) {
    require_auth(msg_sender);
    pool.approve(msg_sender, dst, amt);
  }

  [[eosio::action]] void transfer(name msg_sender, name dst, uint amt) {
    require_auth(msg_sender);
    pool.transfer(msg_sender, dst, amt);
  }

  [[eosio::action]] void transferfrom(name msg_sender, name src, name dst,
                                      uint amt) {
    require_auth(msg_sender);
    pool.transferFrom(msg_sender, src, dst, amt);
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
