/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#prama once 
 #include <common/defines.hpp>

#include <eodos/intf/IDODO.hpp>
#include <eodos/intf/IERC20.hpp>
#include <eodos/lib/SafeMath.hpp>

class IDODOMine { 
 public:

   virtual uint256 getUserLpBalance(address _lpToken, address _user) = 0;
};

#ifdef DODOMINEREADER
class DODOMineReader { 
 public:
    std::tuple<uint256, uint256> getUserStakedBalance(address _dodoMine, address _dodo, address _user) {
       uint256 baseBalance;
       uint256 quoteBalance;
       address baseLpToken  = IDODO(_dodo)._BASE_CAPITAL_TOKEN_();
       address quoteLpToken = IDODO(_dodo)._QUOTE_CAPITAL_TOKEN_();

       uint256 baseLpBalance  = IDODOMine(_dodoMine).getUserLpBalance(baseLpToken, _user);
       uint256 quoteLpBalance = IDODOMine(_dodoMine).getUserLpBalance(quoteLpToken, _user);

       uint256 baseLpTotalSupply  = IERC20(baseLpToken).totalSupply();
       uint256 quoteLpTotalSupply = IERC20(quoteLpToken).totalSupply();

       (uint256 baseTarget, uint256 quoteTarget) = IDODO(_dodo).getExpectedTarget();
       baseBalance                               = baseTarget.mul(baseLpBalance).div(baseLpTotalSupply);
       quoteBalance                              = quoteTarget.mul(quoteLpBalance).div(quoteLpTotalSupply);

       return (baseBalance, quoteBalance);
    }
};
#endif