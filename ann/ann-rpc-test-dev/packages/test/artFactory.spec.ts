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
const artFactory = require("../abi/sub_auction_v0.1.json");

const charliePair = keyring.getPair(CHARLIE);
const davePair = keyring.getPair(DAVE);
const evePair = keyring.getPair(EVE);
const ferdiePair = keyring.getPair(FERDIE);

const address = "5HUtUgvsRGHoevr3AzXCXUPQKwZAoFE9i1WmuCwAdYnWhcYW";
const zeroAddress = "5C4hrfjw9DjXZTzV3MwzrrAr9P1MJhSrvWGWqi1eSuyUpnhM";
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
const artTradableAddress = getTokenAddress("artTradable", 0);
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
    // let abi = new Abi(artFactory, api.registry.getChainProperties());
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
describe("artFactory", () => {
    beforeAll((): void => {
    });
    test("createNftContract", async () => {
        const artFactoryAddress = getContractAddress("artFactory");
        await signedTxC("artFactory", "createNftContract", ["test", "TEST"], null, null, null)
        await queryC("artFactory", "tokens", [], null)
        // expect(global.console.log).toHaveBeenCalledWith('log');

    });

    test("disableTokenContract", async () => {
        setCaller(alicePair)
        await signedTxC("artFactory", "disableTokenContract", [artTradableAddress], null, null, null)
        const a = await queryC("artFactory", "exists", [artTradableAddress], null)
        console.warn("========artFactory list item=======", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("registerTokenContract", async () => {
        setCaller(alicePair)
        const artionAddress = getContractAddress("artion");
        await signedTxC("artFactory", "registerTokenContract", [artTradableAddress], null, null, null)
        const a = await queryC("artFactory", "exists", [artTradableAddress], null)
        console.warn("========artFactory list item=======", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });


    test("updateBundleMarketplace", async () => {
        setCaller(alicePair)
        const bundleMarketplaceAddress = getContractAddress("bundleMarketplace");

        await signedTxC("artFactory", "updateBundleMarketplace", [bundleMarketplaceAddress], null, null, null)
        const a = await queryC("artFactory", "bundleMarketplace", [], null)
        console.warn("========artFactory list item=======", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("updateMarketplace", async () => {
        setCaller(alicePair)
        const marketplaceAddress = getContractAddress("marketplace");

        await signedTxC("artFactory", "updateMarketplace", [marketplaceAddress], null, null, null)
        const a = await queryC("artFactory", "marketplace", [], null)
        console.warn("========artFactory list item=======", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("updateEndowmentAmount", async () => {
        setCaller(alicePair)

        await signedTxC("artFactory", "updateEndowmentAmount", [10], null, null, null)
        const a = await queryC("artFactory", "endowmentAmount", [], null)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });
    test("updateMintFee", async () => {
        setCaller(alicePair)

        await signedTxC("artFactory", "updateMintFee", [10], null, null, null)
        const a = await queryC("artFactory", "platformFee", [], null)
        console.warn("========artFactory list item=======", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("updatePlatformFee", async () => {
        setCaller(alicePair)
        const priceSeedAddress = getContractAddress("priceSeed");

        await signedTxC("artFactory", "updatePlatformFee", [10], null, null, null)
        const a = await queryC("artFactory", "platformFee", [], null)
        console.warn("========artFactory list item=======", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

    test("updatePlatformFeeRecipient", async () => {
        setCaller(alicePair)
        await signedTxC("artFactory", "updatePlatformFeeRecipient", [BOB], null, null, null)
        const a = await queryC("artFactory", "feeRecipient", [], null)
        console.warn("========artFactory list item=======", a)
        expect(global.console.error).toHaveBeenCalledWith('log');

    });

});


