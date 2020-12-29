/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#pragma once
#include <common/defines.hpp>

/**
 * @title SafeMath
 * @author DODO Breeder
 *
 * @notice Math operations with safety checks that revert on error
 */
namespace SafeMath {
uint256 mul(uint256 a, uint256 b) {
   if (a == 0) {
      return 0;
   }

   uint256 c = a * b;
   require(c / a == b, "MUL_ERROR");

   return c;
}

uint256 div(uint256 a, uint256 b) {
   require(b > 0, "DIVIDING_ERROR");
   return a / b;
}

uint256 divCeil(uint256 a, uint256 b) {
   uint256 quotient  = div(a, b);
   uint256 remainder = a - quotient * b;
   if (remainder > 0) {
      return quotient + 1;
   } else {
      return quotient;
   }
}

uint256 sub(uint256 a, uint256 b) {
   require(b <= a, "SUB_ERROR");
   return a - b;
}

uint256 add(uint256 a, uint256 b) {
   uint256 c = a + b;
   require(c >= a, "ADD_ERROR");
   return c;
}

uint256 sqrt(uint256 x) {
   uint256 z = x / 2 + 1;
   uint256 y = x;
   while (z < y) {
      y = z;
      z = (x / z + z) / 2;
   }

   return y;
}
} // namespace SafeMath


///////////////////DSafeMath///////////////////
namespace DSafeMath {
double mul(double a, double b) {
   if (a == 0) {
      return 0;
   }

   double c = a * b;
//    require(c / a == b, "MUL_ERROR");

   return c;
}

double div(double a, double b) {
   require(b > 0, "DIVIDING_ERROR");
   return a / b;
}

double divCeil(double a, double b) {
   double quotient  = div(a, b);
   double remainder = a - quotient * b;
   if (remainder > 0) {
      return quotient + 1;
   } else {
      return quotient;
   }
}

double sub(double a, double b) {
   require(b <= a, "SUB_ERROR");
   return a - b;
}

double add(double a, double b) {
   double c = a + b;
   require(c >= a, "ADD_ERROR");
   return c;
}

double sqrt(double x) {
   double z = x / 2 + 1;
   double y = x;
   while (z < y) {
      y = z;
      z = (x / z + z) / 2;
   }

   return y;
}
} // namespace SafeMath
