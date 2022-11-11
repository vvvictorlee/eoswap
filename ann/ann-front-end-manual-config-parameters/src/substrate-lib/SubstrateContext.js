import addressRegistry from "../abi/sub_address_registry_v0.1.json";
import artion from "../abi/sub_artion_v0.1.json";
import auction from "../abi/sub_auction_v0.1.json";
import bundleMarketplace from "../abi/sub_bundle_marketplace_v0.1.json";
import marketplace from "../abi/sub_marketplace_v0.1.json";
import artFactory from "../abi/sub_art_factory_v0.1.json";
import artFactoryPrivate from "../abi/sub_art_factory_private_v0.1.json";
import nftFactory from "../abi/sub_nft_factory_v0.1.json";
import nftFactoryPrivate from "../abi/sub_nft_factory_private_v0.1.json";
import priceSeed from "../abi/sub_price_seed_v0.1.json";
import tokenRegistry from "../abi/sub_token_registry_v0.1.json";
import erc20 from "../abi/erc20_v0.1.json";
import oracle from "../abi/sub_oracle_v0.1.json";
import mgmt from "../abi/sub_contract_management_v0.1.json";
import React, { useReducer, useContext, useEffect } from "react";
import PropTypes from "prop-types";
import jsonrpc from "@polkadot/types/interfaces/jsonrpc";
import { ApiPromise, WsProvider } from "@polkadot/api";
import { web3Accounts, web3Enable } from "@polkadot/extension-dapp";
import { keyring as Keyring } from "@polkadot/ui-keyring";
import { isTestChain } from "@polkadot/util";
import { TypeRegistry } from "@polkadot/types/create";
import { cryptoWaitReady } from "@polkadot/util-crypto";

import config from "../config";
const parsedQuery = new URLSearchParams(window.location.search);
const connectedSocket = parsedQuery.get("rpc") || config.PROVIDER_SOCKET;

const { ContractPromise } = require("@polkadot/api-contract");
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

///
// Initial state for `useReducer`
const initialState = {
  // These are the states
  socket: connectedSocket,
  jsonrpc: { ...jsonrpc, ...config.CUSTOM_RPC_METHODS },
  keyring: null,
  keyringState: null,
  api: null,
  contract: null,
  contractState: null,
  tokens: null,
  tokenContracts: null,
  apiError: null,
  apiState: null,
  currentAccount: null,
};

const registry = new TypeRegistry();

///
// Reducer function for `useReducer`

const reducer = (state, action) => {
  switch (action.type) {
    case "CONNECT_INIT":
      return { ...state, apiState: "CONNECT_INIT" };
    case "CONNECT":
      return { ...state, api: action.payload, apiState: "CONNECTING" };
    case "CONNECT_SUCCESS":
      return { ...state, apiState: "READY" };
    case "CONNECT_ERROR":
      return { ...state, apiState: "ERROR", apiError: action.payload };
    case "LOAD_KEYRING":
      return { ...state, keyringState: "LOADING" };
    case "SET_KEYRING":
      return { ...state, keyring: action.payload, keyringState: "READY" };
    case "KEYRING_ERROR":
      return { ...state, keyring: null, keyringState: "ERROR" };
    case "SET_CONTRACT":
      return { ...state, contract: action.payload, contractState: "READY" };
    case "SET_TOKENS":
      return { ...state, tokens: action.payload };
    case "SET_TOKEN_CONTRACTS":
      return { ...state, tokenContracts: action.payload };
    case "CONTRACT_ERROR":
      return { ...state, contract: null, contractState: "ERROR" };
    case "SET_CURRENT_ACCOUNT":
      return { ...state, currentAccount: action.payload };
    default:
      throw new Error(`Unknown type: ${action.type}`);
  }
};

///
// Connecting to the Substrate node

const connect = (state, dispatch) => {
  const { apiState, socket, jsonrpc } = state;
  // We only want this function to be performed once
  if (apiState) return;

  dispatch({ type: "CONNECT_INIT" });
  const asyncConnectContracts = async () => {
    await cryptoWaitReady();
  };
  asyncConnectContracts();
  console.log(`Connected socket: ${socket}`);
  const provider = new WsProvider(socket);
  const _api = new ApiPromise({ provider, rpc: jsonrpc });

  // Set listeners for disconnection and reconnection event.
  _api.on("connected", () => {
    dispatch({ type: "CONNECT", payload: _api });
    // `ready` event is not emitted upon reconnection and is checked explicitly here.
    _api.isReady.then((_api) => dispatch({ type: "CONNECT_SUCCESS" }));
  });
  _api.on("ready", () => dispatch({ type: "CONNECT_SUCCESS" }));
  _api.on("error", (err) => dispatch({ type: "CONNECT_ERROR", payload: err }));
};

const retrieveChainInfo = async (api) => {
  const [systemChain, systemChainType] = await Promise.all([
    api.rpc.system.chain(),
    api.rpc.system.chainType
      ? api.rpc.system.chainType()
      : Promise.resolve(registry.createType("ChainType", "Live")),
  ]);

  return {
    systemChain: (systemChain || "<unknown>").toString(),
    systemChainType,
  };
};

///
// Loading accounts from dev and polkadot-js extension
const loadAccounts = (state, dispatch) => {
  const { api } = state;
  dispatch({ type: "LOAD_KEYRING" });

  const asyncLoadAccounts = async () => {
    try {
      await cryptoWaitReady();
      let allInjected = await web3Enable(config.APP_NAME);
      if (allInjected.length === 0) {
        console.error("!!!!! No wallet extention detected!!");
        return;
      }
      let allAccounts = await web3Accounts();

      allAccounts = allAccounts.map(({ address, meta }) => ({
        address,
        meta: { ...meta, name: `${meta.name} (${meta.source})` },
      }));
      // Logics to check if the connecting chain is a dev chain, coming from polkadot-js Apps
      // ref: https://github.com/polkadot-js/apps/blob/15b8004b2791eced0dde425d5dc7231a5f86c682/packages/react-api/src/Api.tsx?_pjax=div%5Bitemtype%3D%22http%3A%2F%2Fschema.org%2FSoftwareSourceCode%22%5D%20%3E%20main#L101-L110
      const { systemChain, systemChainType } = await retrieveChainInfo(api);
      const isDevelopment =
        systemChainType.isDevelopment ||
        systemChainType.isLocal ||
        isTestChain(systemChain);

      Keyring.loadAll({ isDevelopment }, allAccounts);

      dispatch({ type: "SET_KEYRING", payload: Keyring });
    } catch (e) {
      console.error(e);
      dispatch({ type: "KEYRING_ERROR" });
    }
  };
  asyncLoadAccounts();
};

const asyncConnectContracts = async (api, Abi, address) => {
  let _contract = new ContractPromise(api, Abi, address);
  return _contract;
};

const connContract = (state, dispatch) => {
  const { api } = state;

  const asyncConnectContract = async () => {
    try {
      let contracts = {};
      let _contract = await asyncConnectContracts(
        api,
        mgmt,
        config.CONTRACT_ADDRESS
      );
      contracts["mgmt"] = _contract;
      dispatch({ type: "SET_CONTRACT", payload: contracts });
    } catch (e) {
      console.error(e);
      dispatch({ type: "CONTRACT_ERROR" });
    }
  };
  asyncConnectContract();
};

const SubstrateContext = React.createContext();

let keyringLoadAll = false;
let contractInit = false;
const SubstrateContextProvider = (props) => {
  const neededPropNames = ["socket"];
  neededPropNames.forEach((key) => {
    initialState[key] =
      typeof props[key] === "undefined" ? initialState[key] : props[key];
  });

  const [state, dispatch] = useReducer(reducer, initialState);
  connect(state, dispatch);

  useEffect(() => {
    const { apiState, keyringState, contractState } = state;
    if (apiState === "READY" && !keyringState && !keyringLoadAll) {
      keyringLoadAll = true;
      loadAccounts(state, dispatch);
    }
    if (apiState === "READY" && !contractState && !contractInit) {
      contractInit = true;
      connContract(state, dispatch);
    }
  }, [state, dispatch]);

  function setCurrentAccount(acct) {
    dispatch({ type: "SET_CURRENT_ACCOUNT", payload: acct });
  }

  function updateTokens(obj) {
    try {
      if (
        obj == null ||
        obj === undefined ||
        obj === "" ||
        obj === {} ||
        Object.keys(obj).length === 0 ||
        obj[Object.keys(obj)[0]] === undefined ||
        obj[Object.keys(obj)[0]].length === 0
      ) {
        return;
      }
      const { tokens } = state;
      let _tokens = tokens == null ? {} : tokens;
      console.log("======updateTokens======", obj, "======updateTokens======");
      _tokens = Object.assign(_tokens, obj);
      dispatch({ type: "SET_TOKENS", payload: _tokens });
    } catch (e) {
      console.error(e);
    }
  }

  function updateContracts(name, address) {
    console.log("==updateContracts=", name, address);
    const asyncConnectContractss = async () => {
      try {
        const { api, contract } = state;
        let _contract = await asyncConnectContracts(api, abis[name], address);
        let contracts = contract;
        contracts[name] = _contract;
        dispatch({ type: "SET_CONTRACT", payload: contracts });
      } catch (e) {
        console.error(e);
      }
    };
    asyncConnectContractss();
  }
  function updateTokenContracts(instance, address) {
    const { tokenContract } = state;
    let contracts =
      tokenContract == null || tokenContract === undefined ? {} : tokenContract;
    contracts[address] = instance;
    dispatch({ type: "SET_TOKEN_CONTRACTS", payload: contracts });
  }
  return (
    <SubstrateContext.Provider
      value={{
        state,
        setCurrentAccount,
        updateTokens,
        updateContracts,
        updateTokenContracts,
      }}
    >
      {props.children}
    </SubstrateContext.Provider>
  );
};

// prop typechecking
SubstrateContextProvider.propTypes = {
  socket: PropTypes.string,
};

const useSubstrate = () => useContext(SubstrateContext);
const useSubstrateState = () => useContext(SubstrateContext).state;

export { SubstrateContextProvider, useSubstrate, useSubstrateState };
