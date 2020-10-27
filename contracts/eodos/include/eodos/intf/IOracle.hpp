/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/
#pragma once

 #include <common/defines.hpp>


class IOracle { 
 public:

    virtual uint256  getPrice() = 0;
};
