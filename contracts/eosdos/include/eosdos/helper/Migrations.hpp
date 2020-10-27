/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

class Migrations {
 public:
   void restricted() {
      if (getMsgSender() == owner) {
      }
   }

   Migrations() { owner = getMsgSender(); }

   void setCompleted(uint256 completed) {
      restricted();
      last_completed_migration = completed;
   }

   void upgrade(address newAddress) {
      restricted();
      Migrations upgraded = Migrations(newAddress);
      upgraded.setCompleted(last_completed_migration);
   }
};
