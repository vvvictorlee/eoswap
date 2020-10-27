/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
#include <common/defines.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>

class IStorage {
 private:
   IStorage& subStorage;

 public:
   IStorage(IStorage& _subStorage)
       : subStorage(_subStorage) {}
   virtual name                   get_self()= 0;
   virtual storage_mgmt&  get_storage_mgmt()  = 0;
   virtual transfer_mgmt& get_transfer_mgmt() = 0;
   template <typename T>
   void get_dodo(name dodo_name, T func);

   template <typename T>
   void get_lptoken(const extended_symbol& lptoken, T func);

   template <typename T>
   void get_oracle(const extended_symbol& oracle, T func);
};
