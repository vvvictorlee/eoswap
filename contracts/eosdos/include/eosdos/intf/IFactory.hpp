/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
#include <common/defines.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>

class IFactory {
 private:
   IFactory& factory;

 public:
   IFactory(IFactory& _factory)
       : factory(_factory) {}
   virtual name                   get_self()                               = 0;
   virtual storage_mgmt&          get_storage_mgmt()                       = 0;
   virtual transfer_mgmt&         get_transfer_mgmt()                      = 0;
   virtual  extended_symbol newLpToken(const extended_symbol& tokenx) = 0;

//    template <typename T>
//    void get_dodo(name dodo_name, T func) {
//       factory.get_dodo(dodo_name, func);
//    }

   template <typename T>
   void get_lptoken(const extended_symbol& lptoken, T func) {
      factory.get_lptoken(lptoken, func);
   }

   template <typename T>
   void get_oracle(const extended_symbol& oracle, T func) {
      factory.get_oracle(oracle, func);
   }
};
