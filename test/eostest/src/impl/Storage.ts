/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/


import { SafeMath } from "../lib/SafeMath";
import { DecimalMath } from "../lib/DecimalMath";

import { Types_RStatus } from "../lib/Types";


/**
 * @title Storage
 * @author DODO Breeder
 *
 * @notice Local Variables
 */
export class Storage {

    // ============ Variables for Control ============

    _INITIALIZED_: boolean;
    _CLOSED_: boolean;
    _DEPOSIT_QUOTE_ALLOWED_: boolean;
    _DEPOSIT_BASE_ALLOWED_: boolean;
    _TRADE_ALLOWED_: boolean;
    _GAS_PRICE_LIMIT_: number;

    // ============ Advanced Controls ============
    _BUYING_ALLOWED_: boolean;
    _SELLING_ALLOWED_: boolean;
    _BASE_BALANCE_LIMIT_: number;
    _QUOTE_BALANCE_LIMIT_: number;

    // ============ Core Address ============

    _SUPERVISOR_: string; // could freeze system in emergency
    _MAINTAINER_: string; // collect maintainer fee to buy food for DODO

    _BASE_TOKEN_: string;
    _QUOTE_TOKEN_: string;
    _ORACLE_: string;

    // ============ Variables for PMM Algorithm ============

    _LP_FEE_RATE_: number;
    _MT_FEE_RATE_: number;
    _K_: number;

    _R_STATUS_:Types_RStatus;
    _TARGET_BASE_TOKEN_AMOUNT_: number;
    _TARGET_QUOTE_TOKEN_AMOUNT_: number;
    _BASE_BALANCE_: number;
    _QUOTE_BALANCE_: number;

    _BASE_CAPITAL_TOKEN_: string;
    _QUOTE_CAPITAL_TOKEN_: string;

    // ============ Variables for Final Settlement ============

    _BASE_CAPITAL_RECEIVE_QUOTE_: number;
    _QUOTE_CAPITAL_RECEIVE_BASE_: number;
constructor()
{
 this._INITIALIZED_=false;
   this._CLOSED_=false;
   this._DEPOSIT_QUOTE_ALLOWED_=false;
   this._DEPOSIT_BASE_ALLOWED_=false;
   this._TRADE_ALLOWED_=false;
   this._GAS_PRICE_LIMIT_=0;

    // ============ Advanced Controls ============
   this._BUYING_ALLOWED_=false;
   this._SELLING_ALLOWED_=false;
   this._BASE_BALANCE_LIMIT_=18446744073709551615;
   this._QUOTE_BALANCE_LIMIT_=18446744073709551615;

    // ============ Core Address ============

   this._SUPERVISOR_=""; // could freeze system in emergency
   this._MAINTAINER_=""; // collect maintainer fee to buy food for DODO

   this._BASE_TOKEN_="";
   this._QUOTE_TOKEN_="";
   this._ORACLE_="";

    // ============ Variables for PMM Algorithm ============

   this._LP_FEE_RATE_=0;
   this._MT_FEE_RATE_=0;
    this._K_=0.0001;

//   "_TARGET_BASE_TOKEN_AMOUNT_": 100002,
//   "_TARGET_QUOTE_TOKEN_AMOUNT_": 10000000,
//   "_BASE_BALANCE_": 89999,
//   "_QUOTE_BALANCE_": 11000300,
   this._R_STATUS_ = Types_RStatus.ONE;
   this._TARGET_BASE_TOKEN_AMOUNT_=100002;
   this._TARGET_QUOTE_TOKEN_AMOUNT_=10000000;
   this._BASE_BALANCE_=89999;
   this._QUOTE_BALANCE_=11000300;

   this._BASE_CAPITAL_TOKEN_="";
   this._QUOTE_CAPITAL_TOKEN_="";

    // ============ Variables for Final Settlement ============

   this._BASE_CAPITAL_RECEIVE_QUOTE_=0;
   this._QUOTE_CAPITAL_RECEIVE_BASE_=0;
}

    getOraclePrice():number {
        return 10000;
    }

    getBaseCapitalBalanceOf(lp: string) {
        // return IDODOLpToken(_BASE_CAPITAL_TOKEN_).balanceOf(lp);
    }

    getTotalBaseCapital() {
        // return IDODOLpToken(_BASE_CAPITAL_TOKEN_).totalSupply();
    }

    getQuoteCapitalBalanceOf(lp: string) {
        // return IDODOLpToken(_QUOTE_CAPITAL_TOKEN_).balanceOf(lp);
    }

    getTotalQuoteCapital() {
        // return IDODOLpToken(_QUOTE_CAPITAL_TOKEN_).totalSupply();
    }

}


// {
//   "dodo_name": "ethbasemkr11",
//   "initownable": {
//     "_OWNER_": "eosdoseosdos",
//     "_NEW_OWNER_": ""
//   },
//   "guard": {
//     "_ENTERED_": 0
//   },
//   "_INITIALIZED_": 1,
//   "_CLOSED_": 0,
//   "_DEPOSIT_QUOTE_ALLOWED_": 1,
//   "_DEPOSIT_BASE_ALLOWED_": 1,
//   "_TRADE_ALLOWED_": 1,
//   "_GAS_PRICE_LIMIT_": 0,
//   "_BUYING_ALLOWED_": 1,
//   "_SELLING_ALLOWED_": 1,
//   "_BASE_BALANCE_LIMIT_": "18446744073709551615",
//   "_QUOTE_BALANCE_LIMIT_": "18446744073709551615",
//   "_SUPERVISOR_": "eosdoseosdos",
//   "_MAINTAINER_": "dodoowner111",
//   "_BASE_TOKEN_": {
//     "sym": "4,WETH",
//     "contract": "eosdosxtoken"
//   },
//   "_QUOTE_TOKEN_": {
//     "sym": "4,MKR",
//     "contract": "eosdosxtoken"
//   },
//   "_ORACLE_": {
//     "sym": "4,WETH",
//     "contract": "eosdosxtoken"
//   },
//   "_LP_FEE_RATE_": 2,
//   "_MT_FEE_RATE_": 1,
//   "_K_": 1,
//   "_R_STATUS_": 1,
//   "_TARGET_BASE_TOKEN_AMOUNT_": 100002,
//   "_TARGET_QUOTE_TOKEN_AMOUNT_": 10000000,
//   "_BASE_BALANCE_": 89999,
//   "_QUOTE_BALANCE_": 11000300,
//   "_BASE_CAPITAL_TOKEN_": {
//     "sym": "4,WETH",
//     "contract": "ethbasemkr11"
//   },
//   "_QUOTE_CAPITAL_TOKEN_": {
//     "sym": "4,MKR",
//     "contract": "ethbasemkr11"
//   },
//   "_BASE_CAPITAL_RECEIVE_QUOTE_": 0,
//   "_QUOTE_CAPITAL_RECEIVE_BASE_": 0,
//   "_CLAIMED_": []
// }