/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
#include <common/defines.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
class instance_mgmt;
class IFactory {
 public:
   IFactory()
        {}
   virtual name            get_self()                                = 0;
   virtual storage_mgmt&   get_storage_mgmt()                        = 0;
   virtual transfer_mgmt&  get_transfer_mgmt()                       = 0;
   virtual instance_mgmt&  get_instance_mgmt()                       = 0;
   virtual extended_symbol newLpToken(name _msg_sender,name dodo_name,const extended_symbol& tokenx) = 0;

   //    template <typename T>
   //    void get_dodo(name dodo_name, T func) {
   //       _instance_mgmt.get_dodo(dodo_name, func);
   //    }

   template <typename T>
   void get_lptoken(name _msg_sender,const extended_symbol& lptoken, T func);
//  {
//       _instance_mgmt.get_lptoken(lptoken, func);
//    }

//    template <typename T>
//    void get_oracle(name _msg_sender,const extended_symbol& oracle, T func);
// //  {
// //       _instance_mgmt.get_oracle(oracle, func);
// //    }
};
