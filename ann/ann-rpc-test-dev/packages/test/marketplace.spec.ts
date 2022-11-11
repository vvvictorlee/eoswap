// Copyright 2019 Parity Technologies (UK) Ltd.
// This file is part of Substrate.

// Substrate is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// Substrate is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with Substrate. If not, see <http://www.gnu.org/licenses/>.

import { ApiPromise, WsProvider } from "@polkadot/api";
import { createTestKeyring } from "@polkadot/keyring/testing";
import { KeyringPair } from "@polkadot/keyring/types";
import { ContractPromise } from '@polkadot/api-contract';
import BN from "bn.js";
import { Abi } from '@polkadot/api-contract';

import { ALICE, BOB, CHARLIE, DAVE, EVE, FERDIE, WSURL } from "../utils/consts";
import { getAddress } from '../utils/configHelper'
import { Data } from "@polkadot/types";
import { setConsole, initialize, signedTxC, queryC, getContractAddress, getTokenAddress, setCaller } from '../index'

// This is a test account that is going to be created and funded each test.
const keyring = createTestKeyring({ type: "sr25519" });
const alicePair = keyring.getPair(ALICE);
const bobPair = keyring.getPair(BOB);
const salary = 100_000_000_000_000;

const setStatus = async (s: any) => { };
const txResHandler = async ({ status }: { status: any }) =>
    status.isFinalized
        ? setStatus(`😉Transaction Block hash: ${status.asFinalized.toString()}`)
        : setStatus(`Transaction status: ${status.type}`);

const txErrHandler = async (err: any) =>
    setStatus(`😞 Transaction Failed: ${err.toString()}`);
const auction = require("../abi/sub_auction_v0.1.json");

const charliePair = keyring.getPair(CHARLIE);
const davePair = keyring.getPair(DAVE);
const evePair = keyring.getPair(EVE);
const ferdiePair = keyring.getPair(FERDIE);

const address = "5HUtUgvsRGHoevr3AzXCXUPQKwZAoFE9i1WmuCwAdYnWhcYW";
// import {
//     sleepMs
// } from "./utils";
// // const abi = erc20abi;
const value = 0; // only useful on isPayable messagess
let contracts: any = {};
// NOTE the apps UI specified these in mega units
let gasLimit: any = new BN(300000) * new BN(1000000);
let testAccount: KeyringPair;
let api: ApiPromise;
let contract: ContractPromise;
let addresses;
jest.useRealTimers();
let originalLog: any;
let originalWarn: any;
let originalError: any;
const artAddress = getTokenAddress("artTradable", 0);
const nftAddress = getTokenAddress("nftTradable", 0);
const payTokenAddress = getContractAddress("erc20");
const tokenId = 1;

beforeAll(async () => {
    originalLog = global.console.log;
    originalWarn = global.console.warn;
    originalError = global.console.error;

    global.console.log = jest.fn();
    global.console.warn = jest.fn();
    global.console.error = jest.fn();
    // Create a spy on console (console.log in this case) and provide some mocked implementation
    // In mocking global objects it's usually better than simple `jest.fn()`
    // because you can `unmock` it in clean way doing `mockRestore` 
    jest.spyOn(console, 'log').mockImplementation(() => { });

    // api = new ApiPromise({ provider: new WsProvider(WSURL) });
    // await api.isReady;
    // let abi = new Abi(auction, api.registry.getChainProperties());
    // contract = new ContractPromise(api, abi, address);
    setConsole(global.console.log, global.console.warn, global.console.error)

    await initialize()
});


// beforeEach(
//     async () => {


//     }
// );

afterAll(() => {
    // Restore mock after all tests are done, so it won't affect other test suites
    console.log.mockRestore();
    global.console.log = originalLog;
    global.console.warn = originalWarn;
    global.console.error = originalError;
});
afterEach(() => {
    // Clear mock (all calls etc) after each test. 
    // It's needed when you're using console somewhere in the tests so you have clean mock each time
    console.log.mockClear();
});
describe("marketplace", () => {
    beforeAll((): void => {
    });
    test("updatePlatformFee", async () => {
        let fee = 2;
        console.log(parseInt(+new Date() / 1000));
        await signedTxC("marketplace", "updatePlatformFee", [11], null, null, null)
        await queryC("marketplace", "platformFee", [], null)
        // expect(global.console.log).toHaveBeenCalledWith('log');

    });

    test("listItem", async () => {

        let now = parseInt(+new Date() / 1000);
        setCaller(bobPair)
        let paras = { nftAddress, tokenId, quantity: 1, payTokenAddress, pricePerItem: 10, startTime: now + 100 }
        console.error(Object.values(paras), "========paras=======", paras)
        await signedTxC("marketplace", "listItem", Object.values(paras), null, null, null)
        const a = await queryC("marketplace", "listingOf", [nftAddress, tokenId, BOB], null)
        console.warn("==============", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });


    test("buyItem", async () => {
        setCaller(charliePair)
        let paras = { nftAddress, tokenId, payTokenAddress, owner: BOB }

        await signedTxC("marketplace", "buyItem", Object.values(paras), null, null, null)
        const a = await queryC("marketplace", "listingOf", [nftAddress, tokenId, BOB], null)
        console.warn("==============", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("createOffer", async () => {
        let now = parseInt(+new Date() / 1000);
        let paras = { nftAddress, tokenId, payTokenAddress, quantity: 1, pricePerItem: 10, deadline: now + 90000 }
        // console.error(Object.values(paras), "========paras=======", paras)
        setCaller(bobPair)

        await signedTxC("marketplace", "createOffer", Object.values(paras), null, null, null)
        const a = await queryC("marketplace", "offerOf", [nftAddress, tokenId, BOB], null)
        console.warn("====createOffer==========", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("acceptOffer", async () => {

        setCaller(charliePair)

        await signedTxC("marketplace", "acceptOffer", [nftAddress, tokenId, BOB], null, null, null)
        const a = await queryC("marketplace", "offerOf", [nftAddress, tokenId, BOB], null)
        console.error("====acceptOffer==========", a, nftAddress, tokenId)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });
    test("cancelListing", async () => {
        setCaller(bobPair)

        await signedTxC("marketplace", "cancelListing", [nftAddress, tokenId], null, null, null)
        const a = await queryC("marketplace", "listingOf", [nftAddress, tokenId, BOB], null)
        console.warn("==============", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });
    test("cancelOffer", async () => {
        setCaller(bobPair)

        await signedTxC("marketplace", "cancelOffer", [nftAddress, tokenId], null, null, null)
        const a = await queryC("marketplace", "offerOf", [nftAddress, tokenId, BOB], null)
        console.warn("==============", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });


    test("listItem  art", async () => {

        let now = parseInt(+new Date() / 1000);
        setCaller(bobPair)
        let paras = { artAddress, tokenId, quantity: 1, payTokenAddress, pricePerItem: 10, startTime: now + 100 }
        console.error(Object.values(paras), "========paras=======", paras)
        await signedTxC("marketplace", "listItem", Object.values(paras), null, null, null)
        const a = await queryC("marketplace", "listingOf", [artAddress, tokenId, BOB], null)
        console.warn("====art==========", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });


    test("buyItem  art", async () => {
        setCaller(charliePair)
        let paras = { artAddress, tokenId, payTokenAddress, owner: BOB }

        await signedTxC("marketplace", "buyItem", Object.values(paras), null, null, null)
        const a = await queryC("marketplace", "listingOf", [artAddress, tokenId, BOB], null)
        console.warn("==============", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("createOffer  art", async () => {
        let now = parseInt(+new Date() / 1000);
        let paras = { artAddress, tokenId, payTokenAddress, quantity: 1, pricePerItem: 10, deadline: now + 90000 }
        // console.error(Object.values(paras), "========paras=======", paras)
        setCaller(bobPair)

        await signedTxC("marketplace", "createOffer", Object.values(paras), null, null, null)
        const a = await queryC("marketplace", "offerOf", [artAddress, tokenId,BOB], null)
        console.warn("====createOffer==========", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test.only("acceptOffer  art", async () => {

        setCaller(charliePair)

        await signedTxC("marketplace", "acceptOffer", [artAddress, tokenId, BOB], null, null, null)
        const a = await queryC("marketplace", "offerOf", [artAddress, tokenId,BOB], null)
        console.error("====acceptOffer==========", a, artAddress, tokenId)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });
    test("cancelListing  art", async () => {
        setCaller(bobPair)

        await signedTxC("marketplace", "cancelListing", [artAddress, tokenId], null, null, null)
        const a = await queryC("marketplace", "listingOf", [artAddress, tokenId, BOB], null)
        console.warn("==============", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });
    test("cancelOffer  art", async () => {
        setCaller(bobPair)

        await signedTxC("marketplace", "cancelOffer", [artAddress, tokenId], null, null, null)
        const a = await queryC("marketplace", "offerOf", [artAddress, tokenId,BOB], null)
        console.warn("==============", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

});


