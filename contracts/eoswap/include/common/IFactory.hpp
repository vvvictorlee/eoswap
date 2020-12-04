/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once
#include <common/BType.hpp>
#include <common/storage_mgmt.hpp>
#include <common/transfer_mgmt.hpp>
class instance_mgmt;
class IFactory {
 public:
   virtual void           setMsgSender(name _msg_sender) = 0;
   virtual name           get_self()                     = 0;
   virtual storage_mgmt&  get_storage_mgmt()             = 0;
   virtual transfer_mgmt& get_transfer_mgmt()            = 0;
   virtual instance_mgmt& get_instance_mgmt()            = 0;
   virtual void           newBPool(name pool_name)       = 0;
   template <typename T>
   void pool(name pool_name, T func);
};
