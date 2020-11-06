/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once 
 #include <common/defines.hpp>

#include <eosdos/intf/IDODO.hpp>
#include <eosdos/intf/IERC20.hpp>
#include <eosdos/lib/SafeMath.hpp>

class IDODOMine { 
 public:

   virtual uint64_t getUserLpBalance(address _lpToken, address _user) = 0;
};

#ifdef DODOMINEREADER
class DODOMineReader { 
 public:
    std::tuple<uint64_t, uint64_t> getUserStakedBalance(address _dodoMine, address _dodo, address _user) {
       uint64_t baseBalance;
       uint64_t quoteBalance;
       address baseLpToken  = IDODO(_dodo)._BASE_CAPITAL_TOKEN_();
       address quoteLpToken = IDODO(_dodo)._QUOTE_CAPITAL_TOKEN_();

       uint64_t baseLpBalance  = IDODOMine(_dodoMine).getUserLpBalance(baseLpToken, _user);
       uint64_t quoteLpBalance = IDODOMine(_dodoMine).getUserLpBalance(quoteLpToken, _user);

       uint64_t baseLpTotalSupply  = IERC20(baseLpToken).totalSupply();
       uint64_t quoteLpTotalSupply = IERC20(quoteLpToken).totalSupply();

       (uint64_t baseTarget, uint64_t quoteTarget) = IDODO(_dodo).getExpectedTarget();
       baseBalance                               = baseTarget.mul(baseLpBalance).div(baseLpTotalSupply);
       quoteBalance                              = quoteTarget.mul(quoteLpBalance).div(quoteLpTotalSupply);

       return (baseBalance, quoteBalance);
    }
};
#endif