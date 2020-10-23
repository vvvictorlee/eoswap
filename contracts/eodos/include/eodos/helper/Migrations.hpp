/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>

class Migrations {
 public:

   void restricted() {
      if (msg.sender == owner) {
      }
   }

   Migrations() { owner = msg.sender; }

   void setCompleted(uint256 completed) {
      restricted();
      last_completed_migration = completed;
   }

   void upgrade(address newAddress) {
      restricted();
      Migrations upgraded = Migrations(newAddress);
      upgraded.setCompleted(last_completed_migration);
   }
}
