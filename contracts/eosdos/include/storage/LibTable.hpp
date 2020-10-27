
#pragma once

 #include <common/defines.hpp>

struct OwnableStore {
   address _OWNER_;
   address _NEW_OWNER_;

   EOSLIB_SERIALIZE(OwnableStore, (_OWNER_)(_NEW_OWNER_))
};

struct ReentrancyGuardStore {
   bool _ENTERED_;
   EOSLIB_SERIALIZE(ReentrancyGuardStore, (_ENTERED_))
};

