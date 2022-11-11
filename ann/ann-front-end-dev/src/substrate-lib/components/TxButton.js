import artTradable from "../../abi/sub_art_tradable_v0.1.json";
import artTradablePrivate from "../../abi/sub_art_tradable_private_v0.1.json";
import nftTradable from "../../abi/sub_nft_tradable_v0.1.json";
import nftTradablePrivate from "../../abi/sub_nft_tradable_private_v0.1.json";
import erc20 from "../../abi/erc20_v0.1.json";
import oracle from "../../abi/sub_oracle_v0.1.json";

import React, { useState, useEffect } from "react";
import PropTypes from "prop-types";
import { Button } from "semantic-ui-react";
import { web3FromSource } from "@polkadot/extension-dapp";
import { BN } from "@polkadot/util";
import { useSubstrateState, useSubstrate } from "../";
import utils from "../utils";
import { cryptoWaitReady } from "@polkadot/util-crypto";
import { saveValue, setIsInitilized, getNames } from "../configHelper";

const json = require("../../abi/art.json");
const names = getNames();
const { ContractPromise } = require("@polkadot/api-contract");

const abisOfToken = {
  artTradable,
  artTradablePrivate,
  nftTradable,
  nftTradablePrivate,
  erc20,
  oracle,
};
function TxButton({
  attrs = null,
  color = "blue",
  disabled = false,
  label,
  setStatus,
  style = null,
  type = "QUERY",
  txOnClickHandler = null,
}) {
  // Hooks
  const { api, currentAccount, tokenContracts, contract } = useSubstrateState();
  const { updateTokenContracts } = useSubstrate();
  const [unsub, setUnsub] = useState(null);
  const [sudoKey, setSudoKey] = useState(null);

  const {
    palletRpc,
    callable,
    inputParams,
    paramFields,
    contractAddress,
    transferredValue,
  } = attrs;

  const isQuery = () => type === "QUERY";
  const isQueryC = () => type === "QUERYC";
  const isSudo = () => type === "SUDO-TX";
  const isUncheckedSudo = () => type === "UNCHECKED-SUDO-TX";
  const isUnsigned = () => type === "UNSIGNED-TX";
  const isSigned = () => type === "SIGNED-TX";
  const isSignedC = () => type === "SIGNED-TXC";
  const isRpc = () => type === "RPC";
  const isConstant = () => type === "CONSTANT";

  const loadSudoKey = () => {
    (async function () {
      if (!api || !api.query.sudo) {
        return;
      }
      const sudoKey = await api.query.sudo.key();
      sudoKey.isEmpty ? setSudoKey(null) : setSudoKey(sudoKey.toString());
    })();
  };

  useEffect(loadSudoKey, [api]);

  const getFromAcct = async () => {
    const {
      address,
      meta: { source, isTesting },
    } = currentAccount;

    if (isTesting) {
      return [currentAccount];
    }

    // currentAccount is injected from polkadot-JS extension, need to return the addr and signer object.
    // ref: https://polkadot.js.org/docs/extension/cookbook#sign-and-send-a-transaction
    const injector = await web3FromSource(source);
    return [address, { signer: injector.signer }];
  };

  const txResHandler = ({ status }) =>
    status.isFinalized
      ? setStatus(`😉Transaction Block hash: ${status.asFinalized.toString()}`)
      : setStatus(`Transaction status: ${status.type}`);

  const txErrHandler = (err) =>
    setStatus(`😞 Transaction Failed: ${err.toString()}`);

  const sudoTx = async () => {
    const fromAcct = await getFromAcct();
    const transformed = transformParams(paramFields, inputParams);
    // transformed can be empty parameters
    const txExecute = transformed
      ? api.tx.sudo.sudo(api.tx[palletRpc][callable](...transformed))
      : api.tx.sudo.sudo(api.tx[palletRpc][callable]());

    const unsub = txExecute
      .signAndSend(...fromAcct, txResHandler)
      .catch(txErrHandler);

    setUnsub(() => unsub);
  };

  const uncheckedSudoTx = async () => {
    const fromAcct = await getFromAcct();
    const txExecute = api.tx.sudo.sudoUncheckedWeight(
      api.tx[palletRpc][callable](...inputParams),
      0
    );

    const unsub = txExecute
      .signAndSend(...fromAcct, txResHandler)
      .catch(txErrHandler);

    setUnsub(() => unsub);
  };

  const signedTx = async () => {
    const fromAcct = await getFromAcct();
    const transformed = transformParams(paramFields, inputParams);
    // transformed can be empty parameters

    const txExecute = transformed
      ? api.tx[palletRpc][callable](...transformed)
      : api.tx[palletRpc][callable]();

    const unsub = await txExecute
      .signAndSend(...fromAcct, txResHandler)
      .catch(txErrHandler);

    setUnsub(() => unsub);
  };

  const signedTxC = async () => {
    await cryptoWaitReady();
    const fromAcct = await getFromAcct();
    const transformed = inputParams; //transformParams(paramFields, inputParams)
    // transformed can be empty parameters
    setStatus(
      `Current contract transaction status: ${JSON.stringify(fromAcct)}`
    );
    console.log(
      Object.keys(contract),
      palletRpc,
      callable,
      paramFields,
      inputParams,
      fromAcct[0]
    );
    let value = transferredValue == null ? 0 : new BN(transferredValue);

    let address =
      fromAcct[0].address === undefined ? fromAcct[0] : fromAcct[0].address;
    let contractInstance = await getContractInstance();
    const { gasRequired, result } = await contractInstance.query[callable](
      address,
      { gasLimit: -1, value },
      ...transformed
    );
    let gas = gasRequired.addn(1);
    if (!result.isOk) {
      console.error("==err==", result.toHuman());
      gas = -1;
    }
    setStatus(`Current gas status: ${gas}`);
    const fromAccts = await getFromAcct();

    const unsub = await contractInstance.tx[callable](
      { value, gasLimit: gas },
      ...transformed
    )
      .signAndSend(...fromAccts, txResHandler)
      .catch(txErrHandler);
    console.log(
      value.toString(),
      new BN(transferredValue),
      "unsub==",
      contractInstance.address,
      JSON.stringify(unsub)
    );
    setUnsub(() => unsub);
  };
  const queryCC = async (palletRpc, callable, inputParams, contractAddress) => {
    const transformed = inputParams; //transformParams(paramFields, inputParams)
    const fromAcct = await getFromAcct();
    let address =
      fromAcct[0].address === undefined ? fromAcct[0] : fromAcct[0].address;

    let contractInstance = await getContractInstance(
      contractAddress,
      palletRpc
    );
    const { result, output } = await contractInstance.query[callable](
      address,
      { gasLimit: -1, value: 0 },
      ...transformed
    );
    if (!result.isOk) {
      console.log(result.err);
    }
    setStatus(output.toString());
    return output.toString();
  };
  const syncContractAddresses = async () => {
    Object.keys(json.hash).forEach(async (x) => {
      const callable = "addressByCodeHash";
      const address = await queryCC(
        "mgmt",
        callable,
        [JSON.stringify(json.hash[x]).replaceAll('"', "")],
        null
      );
      if (
        address !== undefined &&
        address != null &&
        address.length > 0 &&
        address !== "5C4hrfjw9DjXZTzV3MwzrrAr9P1MJhSrvWGWqi1eSuyUpnhM"
      ) {
        saveValue(names[x], address);
      }
      setIsInitilized();
    });
  };

  const syncTokens = async () => {
    const callable = "tokens";
    const tradables = [
      "artTradable",
      "artTradablePrivate",
      "nftTradable",
      "nftTradablePrivate",
    ];
    const factories = [
      "artFactory",
      "artFactoryPrivate",
      "nftFactory",
      "nftFactoryPrivate",
    ];
    for (let i = 0; i < factories.length; i++) {
      const artaddress = await queryC(factories[i], callable, [], null);
      saveValue(tradables[i], artaddress);
    }
  };

  const queryC = async () => {
    if (callable === "addressesByCodeHashes") {
      await syncContractAddresses();
      return;
    }
    if (callable === "syncTradableTokens") {
      await syncTokens();
      return;
    }
    await cryptoWaitReady();
    const fromAcct = await getFromAcct();
    const transformed = inputParams; //transformParams(paramFields, inputParams)
    setStatus(
      `Current contract transaction status: ${JSON.stringify(fromAcct)}`
    );
    console.log(paramFields, inputParams, fromAcct[0]);

    let address =
      fromAcct[0].address === undefined ? fromAcct[0] : fromAcct[0].address;
    let contractInstance = await getContractInstance();
    const { unsub, result, output } = await contractInstance.query[callable](
      address,
      { gasLimit: -1, value: 0 },
      ...transformed
    );
    if (!result.isOk) {
      console.error(result.err);
    }
    setStatus(output.toString());

    setUnsub(() => unsub);
  };
  const getContractInstance = async () => {
    let instance;
    if (
      contractAddress == null ||
      contractAddress === undefined ||
      contractAddress === ""
    ) {
      console.log(
        contract[palletRpc],
        "======contractAddress=====",
        contractAddress
      );
      instance = contract[palletRpc];
    } else {
      if (
        tokenContracts !== null &&
        tokenContracts !== undefined &&
        tokenContracts[contractAddress] !== undefined
      ) {
        console.log("===sss===contractAddress=====", contractAddress);

        return tokenContracts[contractAddress];
      }
      console.log("===not null===contractAddress=====", contractAddress);

      const asyncConnectContracts = async (Abi, address) => {
        let _contract = new ContractPromise(api, Abi, address);
        return _contract;
      };

      instance = await asyncConnectContracts(
        abisOfToken[palletRpc],
        contractAddress
      );
      await updateTokenContracts(instance, contractAddress);
    }
    return instance;
  };
  const unsignedTx = async () => {
    const transformed = transformParams(paramFields, inputParams);
    // transformed can be empty parameters
    const txExecute = transformed
      ? api.tx[palletRpc][callable](...transformed)
      : api.tx[palletRpc][callable]();

    const unsub = await txExecute.send(txResHandler).catch(txErrHandler);
    setUnsub(() => unsub);
  };

  const queryResHandler = (result) =>
    result.isNone ? setStatus("None") : setStatus(result.toString());

  const query = async () => {
    const transformed = transformParams(paramFields, inputParams);
    const unsub = await api.query[palletRpc][callable](
      ...transformed,
      queryResHandler
    );

    setUnsub(() => unsub);
  };

  const rpc = async () => {
    const transformed = transformParams(paramFields, inputParams, {
      emptyAsNull: false,
    });
    const unsub = await api.rpc[palletRpc][callable](
      ...transformed,
      queryResHandler
    );
    setUnsub(() => unsub);
  };

  const constant = () => {
    const result = api.consts[palletRpc][callable];
    result.isNone ? setStatus("None") : setStatus(result.toString());
  };

  const transaction = async () => {
    if (typeof unsub === "function") {
      unsub();
      setUnsub(null);
    }

    setStatus("Sending...");

    const asyncFunc =
      (isSudo() && sudoTx) ||
      (isUncheckedSudo() && uncheckedSudoTx) ||
      (isSigned() && signedTx) ||
      (isSignedC() && signedTxC) ||
      (isUnsigned() && unsignedTx) ||
      (isQuery() && query) ||
      (isQueryC() && queryC) ||
      (isRpc() && rpc) ||
      (isConstant() && constant);

    await asyncFunc();

    return txOnClickHandler && typeof txOnClickHandler === "function"
      ? txOnClickHandler(unsub)
      : null;
  };

  const transformParams = (
    paramFields,
    inputParams,
    opts = { emptyAsNull: true }
  ) => {
    // if `opts.emptyAsNull` is true, empty param value will be added to res as `null`.
    //   Otherwise, it will not be added
    const paramVal = inputParams.map((inputParam) => {
      // To cater the js quirk that `null` is a type of `object`.
      if (
        typeof inputParam === "object" &&
        inputParam !== null &&
        typeof inputParam.value === "string"
      ) {
        return inputParam.value.trim();
      } else if (typeof inputParam === "string") {
        return inputParam.trim();
      }
      return inputParam;
    });
    const params = paramFields.map((field, ind) => ({
      ...field,
      value: paramVal[ind] || null,
    }));

    return params.reduce((memo, { type = "string", value }) => {
      if (value == null || value === "")
        return opts.emptyAsNull ? [...memo, null] : memo;

      let converted = value;

      // Deal with a vector
      if (type.indexOf("Vec<") >= 0) {
        converted = converted.split(",").map((e) => e.trim());
        converted = converted.map((single) =>
          isNumType(type)
            ? single.indexOf(".") >= 0
              ? Number.parseFloat(single)
              : Number.parseInt(single)
            : single
        );
        return [...memo, converted];
      }

      // Deal with a single value
      if (isNumType(type)) {
        converted =
          converted.indexOf(".") >= 0
            ? Number.parseFloat(converted)
            : Number.parseInt(converted);
      }
      return [...memo, converted];
    }, []);
  };

  const isNumType = (type) =>
    utils.paramConversion.num.some((el) => type.indexOf(el) >= 0);

  const allParamsFilled = () => {
    if (paramFields.length === 0) {
      return true;
    }

    return paramFields.every((paramField, ind) => {
      const param = inputParams[ind];
      if (paramField.optional) {
        return true;
      }
      if (param == null) {
        return false;
      }

      const value = typeof param === "object" ? param.value : param;
      return value !== null && value !== "";
    });
  };

  const isSudoer = (acctPair) => {
    if (!sudoKey || !acctPair) {
      return false;
    }
    return acctPair.address === sudoKey;
  };

  return (
    <Button
      basic
      color={color}
      style={style}
      type="submit"
      onClick={transaction}
      disabled={
        disabled ||
        !palletRpc ||
        !callable ||
        !allParamsFilled() ||
        // These txs required currentAccount to be set
        ((isSudo() || isUncheckedSudo() || isSigned() || isSignedC()) &&
          !currentAccount) ||
        ((isSudo() || isUncheckedSudo()) && !isSudoer(currentAccount))
      }
    >
      {label}
    </Button>
  );
}

// prop type checking
TxButton.propTypes = {
  setStatus: PropTypes.func.isRequired,
  type: PropTypes.oneOf([
    "QUERY",
    "QUERYC",
    "RPC",
    "SIGNED-TX",
    "SIGNED-TXC",
    "UNSIGNED-TX",
    "SUDO-TX",
    "UNCHECKED-SUDO-TX",
    "CONSTANT",
  ]).isRequired,
  attrs: PropTypes.shape({
    palletRpc: PropTypes.string,
    callable: PropTypes.string,
    inputParams: PropTypes.array,
    paramFields: PropTypes.array,
  }).isRequired,
};

function TxGroupButton(props) {
  return (
    <Button.Group>
      <TxButton label="Unsigned" type="UNSIGNED-TX" color="grey" {...props} />
      <Button.Or />
      <TxButton label="Signed" type="SIGNED-TX" color="blue" {...props} />
      <Button.Or />
      <TxButton label="SUDO" type="SUDO-TX" color="red" {...props} />
    </Button.Group>
  );
}

export { TxButton, TxGroupButton };
