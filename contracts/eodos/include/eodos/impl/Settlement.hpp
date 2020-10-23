/*

    Copyright 2020 DODO ZOO.
    SPDX-License-Identifier: Apache-2.0

*/

#include <common/defines.hpp>


#include <eodos/lib/SafeMath.hpp>
#include <eodos/lib/SafeERC20.hpp>
#include <eodos/lib/DecimalMath.hpp>
#include <eodos/lib/Types.hpp>
#include <eodos/intf/IDODOLpToken.hpp>
#include <eodos/intf/IERC20.hpp>
#include <eodos/Storage.hpp>


/**
 * @title Settlement
 * @author DODO Breeder
 *
 * @notice Functions for assets settlement
 */
class Settlement : public  Storage {
    
    

    // ============ Events ============

    

    // ============ Assets IN/OUT Functions ============

    void  _baseTokenTransferIn(address from, uint256 amount) {
        require(_BASE_BALANCE_.add(amount) <= _BASE_BALANCE_LIMIT_, "BASE_BALANCE_LIMIT_EXCEEDED");
        IERC20(_BASE_TOKEN_).safeTransferFrom(from, address(this), amount);
        _BASE_BALANCE_ = _BASE_BALANCE_.add(amount);
    }

    void  _quoteTokenTransferIn(address from, uint256 amount) {
        require(
            _QUOTE_BALANCE_.add(amount) <= _QUOTE_BALANCE_LIMIT_,
            "QUOTE_BALANCE_LIMIT_EXCEEDED"
        );
        IERC20(_QUOTE_TOKEN_).safeTransferFrom(from, address(this), amount);
        _QUOTE_BALANCE_ = _QUOTE_BALANCE_.add(amount);
    }

    void  _baseTokenTransferOut(address to, uint256 amount) {
        IERC20(_BASE_TOKEN_).safeTransfer(to, amount);
        _BASE_BALANCE_ = _BASE_BALANCE_.sub(amount);
    }

    void  _quoteTokenTransferOut(address to, uint256 amount) {
        IERC20(_QUOTE_TOKEN_).safeTransfer(to, amount);
        _QUOTE_BALANCE_ = _QUOTE_BALANCE_.sub(amount);
    }

    // ============ Donate to Liquidity Pool Functions ============

    void  _donateBaseToken(uint256 amount) {
        _TARGET_BASE_TOKEN_AMOUNT_ = _TARGET_BASE_TOKEN_AMOUNT_.add(amount);
        
    }

    void  _donateQuoteToken(uint256 amount) {
        _TARGET_QUOTE_TOKEN_AMOUNT_ = _TARGET_QUOTE_TOKEN_AMOUNT_.add(amount);
        
    }

    void  donateBaseToken(uint256 amount) {
        _baseTokenTransferIn(msg.sender, amount);
        _donateBaseToken(amount);
    }

    void  donateQuoteToken(uint256 amount) {
        _quoteTokenTransferIn(msg.sender, amount);
        _donateQuoteToken(amount);
    }

    // ============ Final Settlement Functions ============

    // last step to shut down dodo
    void  finalSettlement() {
        _CLOSED_ = true;
        _DEPOSIT_QUOTE_ALLOWED_ = false;
        _DEPOSIT_BASE_ALLOWED_ = false;
        _TRADE_ALLOWED_ = false;
        uint256 totalBaseCapital = getTotalBaseCapital();
        uint256 totalQuoteCapital = getTotalQuoteCapital();

        if (_QUOTE_BALANCE_ > _TARGET_QUOTE_TOKEN_AMOUNT_) {
            uint256 spareQuote = _QUOTE_BALANCE_.sub(_TARGET_QUOTE_TOKEN_AMOUNT_);
            _BASE_CAPITAL_RECEIVE_QUOTE_ = DecimalMath.divFloor(spareQuote, totalBaseCapital);
        } else {
            _TARGET_QUOTE_TOKEN_AMOUNT_ = _QUOTE_BALANCE_;
        }

        if (_BASE_BALANCE_ > _TARGET_BASE_TOKEN_AMOUNT_) {
            uint256 spareBase = _BASE_BALANCE_.sub(_TARGET_BASE_TOKEN_AMOUNT_);
            _QUOTE_CAPITAL_RECEIVE_BASE_ = DecimalMath.divFloor(spareBase, totalQuoteCapital);
        } else {
            _TARGET_BASE_TOKEN_AMOUNT_ = _BASE_BALANCE_;
        }

        _R_STATUS_ = Types.RStatus.ONE;
    }

    // claim remaining assets after final settlement
    void  claimAssets() {
        require(_CLOSED_, "DODO_NOT_CLOSED");
        require(!_CLAIMED_[msg.sender], "ALREADY_CLAIMED");
        _CLAIMED_[msg.sender] = true;

        uint256 quoteCapital = getQuoteCapitalBalanceOf(msg.sender);
        uint256 baseCapital = getBaseCapitalBalanceOf(msg.sender);

        uint256 quoteAmount = 0;
        if (quoteCapital > 0) {
            quoteAmount = _TARGET_QUOTE_TOKEN_AMOUNT_.mul(quoteCapital).div(getTotalQuoteCapital());
        }
        uint256 baseAmount = 0;
        if (baseCapital > 0) {
            baseAmount = _TARGET_BASE_TOKEN_AMOUNT_.mul(baseCapital).div(getTotalBaseCapital());
        }

        _TARGET_QUOTE_TOKEN_AMOUNT_ = _TARGET_QUOTE_TOKEN_AMOUNT_.sub(quoteAmount);
        _TARGET_BASE_TOKEN_AMOUNT_ = _TARGET_BASE_TOKEN_AMOUNT_.sub(baseAmount);

        quoteAmount = quoteAmount.add(DecimalMath.mul(baseCapital, _BASE_CAPITAL_RECEIVE_QUOTE_));
        baseAmount = baseAmount.add(DecimalMath.mul(quoteCapital, _QUOTE_CAPITAL_RECEIVE_BASE_));

        _baseTokenTransferOut(msg.sender, baseAmount);
        _quoteTokenTransferOut(msg.sender, quoteAmount);

        IDODOLpToken(_BASE_CAPITAL_TOKEN_).burn(msg.sender, baseCapital);
        IDODOLpToken(_QUOTE_CAPITAL_TOKEN_).burn(msg.sender, quoteCapital);

        
        return;
    }

    // in case someone transfer to contract directly
    void  retrieve(address token, uint256 amount) {
        if (token == _BASE_TOKEN_) {
            require(
                IERC20(_BASE_TOKEN_).balanceOf(address(this)) >= _BASE_BALANCE_.add(amount),
                "DODO_BASE_BALANCE_NOT_ENOUGH"
            );
        }
        if (token == _QUOTE_TOKEN_) {
            require(
                IERC20(_QUOTE_TOKEN_).balanceOf(address(this)) >= _QUOTE_BALANCE_.add(amount),
                "DODO_QUOTE_BALANCE_NOT_ENOUGH"
            );
        }
        IERC20(token).safeTransfer(msg.sender, amount);
    }
}
