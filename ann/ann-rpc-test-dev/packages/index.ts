const addressRegistry = require("./abi/sub_address_registry_v0.1.json");
const artion = require("./abi/sub_artion_v0.1.json");
const auction = require("./abi/sub_auction_v0.1.json");
const bundleMarketplace = require("./abi/sub_bundle_marketplace_v0.1.json");
const marketplace = require("./abi/sub_marketplace_v0.1.json");
const artFactory = require("./abi/sub_art_factory_v0.1.json");
const artFactoryPrivate = require("./abi/sub_art_factory_private_v0.1.json");
const nftFactory = require("./abi/sub_nft_factory_v0.1.json");
const nftFactoryPrivate = require("./abi/sub_nft_factory_private_v0.1.json");
const priceSeed = require("./abi/sub_price_seed_v0.1.json");
const tokenRegistry = require("./abi/sub_token_registry_v0.1.json");
const erc20 = require("./abi/erc20_v0.1.json");
const mgmt = require("./abi/sub_contract_management_v0.1.json");
const artTradable = require("./abi/sub_art_tradable_v0.1.json");
const artTradablePrivate = require("./abi/sub_art_tradable_private_v0.1.json");
const nftTradable = require("./abi/sub_nft_tradable_v0.1.json");
const nftTradablePrivate = require("./abi/sub_nft_tradable_private_v0.1.json");
// const erc20 = require( "./abi/erc20_v0.1.json");
const oracle = require("./abi/sub_oracle_v0.1.json");
const json = require("./abi/art.json");


import { ApiPromise, WsProvider } from "@polkadot/api";
import { createTestKeyring } from "@polkadot/keyring/testing";
import { KeyringPair } from "@polkadot/keyring/types";
import { ContractPromise } from '@polkadot/api-contract';
import { cryptoWaitReady } from "@polkadot/util-crypto";

import BN from "bn.js";
import { Abi } from '@polkadot/api-contract';

import { ALICE, BOB, CHARLIE, DAVE, EVE, FERDIE, WSURL } from "./utils/consts";
import { saveValue, getAddresses, getNames } from './utils/configHelper'
import { AnyNumber } from "@polkadot/types/types";

// This is a test account that is going to be created and funded each test.
const keyring = createTestKeyring({ type: "sr25519" });
const alicePair = keyring.getPair(ALICE);
const bobPair = keyring.getPair(BOB);
const charliePair = keyring.getPair(CHARLIE);
const davePair = keyring.getPair(DAVE);
const evePair = keyring.getPair(EVE);
const ferdiePair = keyring.getPair(FERDIE);
const salary = 100_000_000_000_000;

const abis = {
    addressRegistry,
    artion,
    auction,
    bundleMarketplace,
    marketplace,
    artFactory,
    artFactoryPrivate,
    nftFactory,
    nftFactoryPrivate,
    priceSeed,
    tokenRegistry,
    erc20,
    oracle,
};
const abisOfToken = {
    artTradable,
    artTradablePrivate,
    nftTradable,
    nftTradablePrivate,
    erc20,
    oracle,
};
const names = getNames();
const contractAddresses = getAddresses();
const asyncConnectContract = async (api: any, Abi: any, address: any) => {
    let _contract = new ContractPromise(api, Abi, address);
    return _contract;
};

export const asyncConnectContracts = async (name: any, address: any) => {
    try {
        let _contract = await asyncConnectContract(api, abis[name as keyof typeof abis], address);
        let contracts = contract;
        contractInstances[name] = _contract;
    } catch (e) {
        consoleerror(e);
    }
};
export function setCaller(user: any) {
    fromAcct = user;
}
export async function updateContracts() {
    Object.keys(abis).forEach(async (name: any) => {
        consolelog("==updateContracts=", name, contractAddresses[name]);
        if (contractAddresses[name] != undefined && contractAddresses[name] != null) {
            await asyncConnectContracts(name, contractAddresses[name]);
        }

    })
}
export async function updateTokenContracts() {
    tokenContracts =
        tokenContracts == null || tokenContracts === undefined ? {} : tokenContracts;
    Object.keys(abisOfToken).forEach(async (name: any) => {
        consolelog("==updateTokenContracts=", name, contractAddresses[name]);
        if (contractAddresses[name] != undefined && contractAddresses[name] != null) {
            let addresses = contractAddresses[name].trim();
            if (addresses[0] == "[") {
                addresses = addresses.slice(1, addresses.length - 1);
            }
            consolelog("====addresses.split(", ")=====", addresses.split(","))
            addresses.split(",").forEach(async (address: any) => {
                if (address != undefined && address != null && address.length > 0) {
                    address = address.trim()
                    let instance = await asyncConnectContract(api, abisOfToken[name as keyof typeof abisOfToken], address);
                    tokenContracts[address] = instance;
                }
            })
        }

    })

}
export function getTokenAddress(name: any, index: any) {
    if (contractAddresses[name] != undefined && contractAddresses[name] != null) {
        let addresses = contractAddresses[name].trim();
        if (addresses[0] == "[") {
            addresses = addresses.slice(1, addresses.length - 1);
        }
        consolelog("====addresses.split(", ")=====", addresses.split(","))
        let arr = addresses.split(",");
        return arr[index]
    }
    return ""
}
export function getContractAddress(name: any) {
    if (contractAddresses[name] != undefined && contractAddresses[name] != null) {
        return contractAddresses[name].trim()
    }
    return ""
}

let contractInstances: any = {};

let tokenContracts: any = {};
export const getContractInstance = async (contractAddress: any, palletRpc: any) => {
    let instance;
    if (
        contractAddress == null ||
        contractAddress === undefined ||
        contractAddress === ""
    ) {
        consolelog(
            "======contractAddress=====",
            contractAddress
        );
        instance = contractInstances[palletRpc];
    } else {
        if (
            tokenContracts !== null &&
            tokenContracts !== undefined &&
            tokenContracts[contractAddress] !== undefined
        ) {
            consolelog("===sss===contractAddress=====", contractAddress);

            return tokenContracts[contractAddress];
        }
        consolelog("===not null===contractAddress=====", contractAddress);

        instance = await asyncConnectContract(api,
            abisOfToken[palletRpc as keyof typeof abisOfToken],
            contractAddress
        );
        tokenContracts[contractAddress] = instance
    }
    return instance;
};

let fromAcct = alicePair;
export const signedTxC = async (palletRpc: any, callable: any, inputParams: any, contractAddress: any, nextNonce: any, transferredValue: any,) => {
    await cryptoWaitReady();
    const transformed = inputParams; //transformParams(paramFields, inputParams)
    // transformed can be empty parameters
    // setStatus(
    //     `Current contract transaction status: ${JSON.stringify(fromAcct)}`
    // );
    let value = transferredValue == null ? 0 : new BN(transferredValue);
    let nonce = nextNonce == null ? {} : { nextNonce }
    let address = fromAcct.address;
    let contractInstance = await getContractInstance(contractAddress, palletRpc);
    consoleerror(address, "=============", contractAddress, "=signedTxC=callable==", callable);
    const { gasRequired, result } = await contractInstance.query[callable](
        address,
        { gasLimit: -1, value },
        ...transformed
    );
    let gas = gasRequired.addn(1);
    if (!result.isOk) {
        consoleerror("==err==", result.toHuman());
        gas = -1;
    }
    setStatus(`Current gas status: ${gas}`);

    const unsub = await contractInstance.tx[callable](
        { value, gasLimit: gas },
        ...transformed
    )
        .signAndSend(fromAcct, nonce, txResHandler)
        .catch(txErrHandler);

    // setUnsub(() => unsub);
};

export const queryC = async (palletRpc: any, callable: any, inputParams: any, contractAddress: any,) => {
    const transformed = inputParams; //transformParams(paramFields, inputParams)

    let address = fromAcct.address;
    let contractInstance = await getContractInstance(contractAddress, palletRpc);
    consolelog(contractAddress, "=queryC=callable==", callable);
    // contractInstance.abi.messages.forEach((message:any, index: any) => {
    //     // consolelog(index,contract.abi.messages[index].method)
    //     consolelog(palletRpc,index, message.method)
    //     // let abi = new Abi(abis[key.toString()], api.registry.getChainProperties());
    //     // contracts[key.toString()] = new ContractPromise(api, abi, addresses[key]);
    // })
    const { unsub, result, output } = await contractInstance.query[callable](
        address,
        { gasLimit: -1, value: 0 },
        ...transformed
    );
    if (!result.isOk) {
        consoleerror(result.err);
    }
    setStatus(output.toString());
    return output.toString()
};

const setStatus = (s: any) => consolelog(s);
const txResHandler = ({ status }: { status: any }) =>
    status.isFinalized
        ? setStatus(`😉Transaction Block hash: ${status.asFinalized.toString()}`)
        : setStatus(`Transaction status: ${status.type}`);

const txErrHandler = (err: any) =>
    setStatus(`😞 Transaction Failed: ${err.toString()}`);
// const auction = require("./abi/sub_auction_v0.1.json");

let consolelog: any = console.log
let consoleerror: any = console.error
let consolewarn: any = console.warn
export const setConsole = (log: any, warn: any, error: any) => { consolelog = log; consoleerror = error; consolewarn = warn }
const mgmtAddress = contractAddresses["sub_contract_management"];
import {
    sleepMs, putCode
} from "./utils/utils";
// // const abi = erc20abi;
const value = 0; // only useful on isPayable messagess
// let abis: { [key: string]: any } = { "auth": authenticated_proxyabi, "delegate": ownable_delegate_proxyabi, "atom": wyvern_atomicizerabi, "registry": wyvern_proxy_registryabi, "token": wyvern_token_transfer_proxyabi, "erc20": erc20abi, "erc721": erc721abi };
let contracts: any = {};
// NOTE the apps UI specified these in mega units
let gasLimit: any = 1;//new BN(300000) * new BN(1000000);
let testAccount: KeyringPair;
let api: ApiPromise;
let contract: ContractPromise;
let addresses;

const hashes = Object.keys(json.hash)
    .filter((x) => -1 === json.exclude.indexOf(x))
    .map((x) => JSON.stringify(json.hash[x]).replaceAll('"', ""));

// let tradable_hashes = json.exclude.slice(1).map((x: any) => [JSON.stringify(json.hash[x]).replaceAll('"', "")])
let tradable_hashes = json.exclude.slice(1).map((x: any) => JSON.stringify(json.hash[x]).replaceAll('"', ""))

async function init() {
    api = new ApiPromise({ provider: new WsProvider(WSURL) });
    // Set listeners for disconnection and reconnection event.
    api.on("connected", () => {
        consolelog({ type: "CONNECT", payload: api });
        // `ready` event is not emitted upon reconnection and is checked explicitly here.
        api.isReady.then((api) => consolelog({ type: "CONNECT_SUCCESS" }));
    });
    api.on("ready", () => consolelog({ type: "CONNECT_SUCCESS" }));
    api.on("error", (err) => consolelog({ type: "CONNECT_ERROR", payload: err }));
    await api.isReady;
    let abi = new Abi(mgmt, api.registry.getChainProperties());
    contract = new ContractPromise(api, abi, mgmtAddress);
    contractInstances["mgmt"] = contract;
    contract.abi.messages.forEach((message, index: any) => {
        // consolelog(index,contract.abi.messages[index].method)
        consolelog(index, message.method)
        // let abi = new Abi(abis[key.toString()], api.registry.getChainProperties());
        // contracts[key.toString()] = new ContractPromise(api, abi, addresses[key]);
    })
}


export async function updateParas() {
    const transferredValues: any = new Array(10).fill(0);

    const paras: any = [...tradable_hashes, [10], [10], [EVE], [1000_000], [1], [12]];

    let address = ALICE;
    let value = 0;
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    for (let index = 0; index < 10; index++) {
        nonce = (Number(nonce.toString()) + Number(1)).toString();
        const callable = contract.abi.messages[index + 21].method;
        const transferredValue = transferredValues[index];
        const transformed = paras[index];
        await signedTxC("mgmt", callable, transformed, null, nonce, null);
    }
}


export async function updateParameters() {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    nonce = (Number(nonce.toString()) + Number(1)).toString();
    const callable = "updateParameters"
    console.log("========tradable_hashes============", tradable_hashes, ...tradable_hashes)
    await signedTxC("mgmt", callable, [...tradable_hashes], null, nonce, null);
}

export async function instantiateMarket() {
    let address = ALICE;
    let value = 0;
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    for (let index of [0, 4, 5, 6]) {
        let hash = hashes[index]
        const callable = contract.abi.messages[index].method;
        await signedTxC("mgmt", callable, [hash], null, nonce, null);
        nonce = (Number(nonce.toString()) + Number(1)).toString();
    }
}

export async function instantiateFactoryToken() {

    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    for (let index of [1, 7, 10, 11]) {
        let hash = hashes[index]
        const callable = contract.abi.messages[index].method;
        await signedTxC("mgmt", callable, [hash], null, nonce, null);
        nonce = (Number(nonce.toString()) + Number(1)).toString();
    }
}


export async function updateTokens() {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();

    const callable = contract.abi.messages[44].method;
    const erc20Address = await queryC("mgmt", callable, [hashes[11]], null);

    await signedTxC("mgmt", "updateWrappedToken", [erc20Address], null, nonce, null);
    nonce = (Number(nonce.toString()) + Number(1)).toString();
    await signedTxC("mgmt", "updateToken", [erc20Address], null, nonce, null);
}

export async function instantiate() {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    for (let index of [9, 12]) {
        let hash = hashes[index]
        const callable = contract.abi.messages[index].method;
        await signedTxC("mgmt", callable, [hash], null, nonce, null);
        nonce = (Number(nonce.toString()) + Number(1)).toString();

    }
}

export async function instantiateContracts() {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    const callable = "instantiateContracts";//instantiateContracts
    console.log("=========hashes========", hashes)
    await signedTxC("mgmt", callable, hashes, null, nonce, null);
    nonce = (Number(nonce.toString()) + Number(1)).toString();

}

export async function syncContractAddresses() {
    Object.keys(json.hash).forEach(async (x: any, index: any) => {
        const callable = "addressByCodeHash";
        const address = await queryC("mgmt", callable, [JSON.stringify(json.hash[x]).replaceAll('"', "")], null);
        consolelog(x, "==address==", address, address != undefined, address != null, address.length > 0)
        if (address != undefined && address != null && address.length > 0 && address != "5C4hrfjw9DjXZTzV3MwzrrAr9P1MJhSrvWGWqi1eSuyUpnhM") {
            saveValue(names[x], address)
        }
    })

}
export async function syncTokens() {
    const callable = "tokens";
    const tradables = ["artTradable", "artTradablePrivate", "nftTradable", "nftTradablePrivate"]
    const factories = ["artFactory", "artFactoryPrivate", "nftFactory", "nftFactoryPrivate"]
    for (let i = 0; i < factories.length; i++) {
        const artaddress = await queryC(factories[i], callable, [], null);
        saveValue(tradables[i], artaddress)
    }

    const nftaddress = await queryC("nftFactory", callable, [], null);

    saveValue("nftTradable", nftaddress)
}
export async function addToken(address: any) {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    await signedTxC("tokenRegistry", "add", [address], null, nonce, null);
    const callable = "enabled";
    const enabled = await queryC("tokenRegistry", callable, [address], null);
    consolelog(enabled, "=enabled---------===", address)
}
export async function newToken(name: any, symbol: any) {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    await signedTxC("artFactory", "createNftContract", [name, symbol], null, nonce, 10);
    nonce = (Number(nonce.toString()) + Number(1)).toString();
    await signedTxC("nftFactory", "createNftContract", [name, symbol], null, nonce, 10);
    const callable = "tokens";
    const artaddress = await queryC("artFactory", callable, [], null);
    const nftaddress = await queryC("nftFactory", callable, [], null);
    saveValue("artTradable", artaddress)
    saveValue("nftTradable", nftaddress)
}

let market = ["auction", "bundleMarketplace", "marketplace"];
export async function approve() {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = 0;
    let index = 0;
    const callable = "approve";
    // ["auction", "bundleMarketplace", "marketplace"].forEach(async (name: any,index:any) => {
    for (let index = 0; index < market.length; index++) {
        let name = market[index]
        nonce = Number(nextNonce) + Number(index);
        consolelog(index, "=nonce---------===", nonce)

        await signedTxC("erc20", callable, [contractInstances[name].address, salary], null, Number(nextNonce) + Number(index), null);
        const amount = await queryC("erc20", "allowance", [fromAcct.address, contractInstances[name].address], null);
        consolelog(index, "====", nonce, "=====allowance====", amount, Number(nextNonce) + Number(index))
    }

    // })
}
export async function transfer(to: any, amount: any) {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    const callable = "transfer";
    await signedTxC("erc20", callable, [to, amount], null, nonce, null);
    nonce = (Number(nonce.toString()) + Number(1)).toString();
    const balance = await queryC("erc20", "balanceOf", [to], null);
    consolelog(to, "=====balanceOf====", balance)
}
export async function setApprovalForAllArt(contractAddress: any) {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    consolelog("====contractInstances====", Object.keys(contractInstances))
    for (let index = 0; index < market.length; index++) {
        let name = market[index]
        consolelog("=======name===========", name)
        await signedTxC("artTradable", "erc1155::setApprovalForAll", [contractInstances[name].address, true], contractAddress, nonce, null);
        nonce = (Number(nonce.toString()) + Number(1)).toString();
        const artflag = await queryC("artTradable", "erc1155::isApprovedForAll", [fromAcct.address, contractInstances[name].address], contractAddress);

        consolelog(artflag, "=====isApprovedForAll====")
    }

}

export async function setApprovalForAllNFT(contractAddress: any) {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    for (let index = 0; index < market.length; index++) {
        let name = market[index]
        await signedTxC("nftTradable", "erc721::setApprovalForAll", [contractInstances[name].address, true], contractAddress, nonce, null);
        nonce = (Number(nonce.toString()) + Number(1)).toString();
        const nftflag = await queryC("nftTradable", "erc721::isApprovedForAll", [fromAcct.address, contractInstances[name].address], contractAddress);
        consolelog("=====isApprovedForAll====", nftflag)
    }

}

export async function mintArt(to: any, contractAddress: any) {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    await signedTxC("artTradable", "mintArt", [to, 1, "uri"], contractAddress, nonce, 10);
    nonce = (Number(nonce.toString()) + Number(1)).toString();

    const artflag = await queryC("artTradable", "erc1155::balanceOf", [to, 1], contractAddress);

    consolelog(artflag, "=====mintArt====",)
}

export async function mintNFT(to: any, contractAddress: any) {
    let nextNonce = await api.rpc.system.accountNextIndex(fromAcct.address)
    let nonce = nextNonce.toString();
    await signedTxC("nftTradable", "mintNft", [to, "uri"], contractAddress, nonce, 10);
    nonce = (Number(nonce.toString()) + Number(1)).toString();

    const nftflag = await queryC("nftTradable", "erc721::balanceOf", [to], contractAddress);
    consolelog("=====mintNFT====", nftflag)
}
export async function initialize() {
    await init();
    await updateContracts();
    await updateTokenContracts()
}
export async function uploadCode() {
   let codeHash= await putCode(api,fromAcct,"../abi/sub_contract_management_v0.1.wasm");
    console.log("codeHash",codeHash);
}
(async function () {
    switch (process.argv[2]) {
        case "uploadCode": {
            await init();
            await uploadCode();
            break;
        }
        case "init": {
            await init();
            break;
        }
        case "instantiate": {
            await init();
            await updateParas();
            sleepMs(1000);
            await instantiateMarket();
            sleepMs(1000);
            await instantiateFactoryToken();
            sleepMs(1000);
            await updateTokens();
            sleepMs(1000);
            await instantiate();
            sleepMs(1000);
            break;
        }
        case "updateTokenContracts": {
            await init();
            await updateTokenContracts()
            break;
        }
        case "updateContracts": {
            await init();
            await updateContracts();
            break;
        }
        case "addToken": {
            await initialize();
            let ca = getContractAddress("erc20");
            await addToken(ca);
            break;
        }
        case "instantiateContracts": {
            await init();
            await updateParameters();
            sleepMs(7000);
            await instantiateContracts();
            break;
        }
        case "syncContractAddresses": {
            await initialize();
            await syncContractAddresses();
            break;
        }
        case "newToken": {
            await initialize();
            await newToken("b", "b");
            break;
        }
        case "syncTokens": {
            await initialize();
            await syncTokens();
            break;
        }
        case "setApprovalForAllArt": {
            await initialize();
            let ca = getTokenAddress("artTradable", 0);
            fromAcct = bobPair
            await setApprovalForAllArt(ca);
            fromAcct = charliePair
            await setApprovalForAllArt(ca);
            break;
        }
        case "setApprovalForAllNFT": {
            await initialize();
            let ca = getTokenAddress("nftTradable", 0);

            fromAcct = bobPair
            await setApprovalForAllNFT(ca);
            fromAcct = charliePair
            await setApprovalForAllNFT(ca);
            break;
        }
        case "approve": {
            await initialize();
            fromAcct = bobPair
            await approve();
            fromAcct = charliePair
            await approve();
            break;
        }
        case "transfer": {
            await initialize();
            await transfer(BOB, 120);
            await transfer(CHARLIE, 120);
            await transfer(DAVE, 120);
            break;
        }
        case "mintArt": {
            await initialize();
            let ca = getTokenAddress("artTradable", 0);
            await mintArt(BOB, ca);
            break;
        }
        case "mintNFT": {
            await initialize();
            let ca = getTokenAddress("nftTradable", 0);
            await mintNFT(BOB, ca);
            break;
        }
        default: {
            consolelog(process.argv)
        }
    }

})();