import React, { useState } from "react";
import { Grid, Form, Button } from "semantic-ui-react";
import { useSubstrate, useSubstrateState } from "./substrate-lib";
import { TxButton } from "./substrate-lib/components";

const json = require("./abi/art.json");

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const { contract } = useSubstrateState();
  const { updateContracts } = useSubstrate();
//   const [formState] = useState({
//     addressRegistry: "",
//     auctionAddress: "",
//     marketplaceAddress: "",
//     bundleMarketplaceAddress: "",
//     artTradableHash: JSON.stringify(json.hash["sub_art_tradable"]).replaceAll(
//       '"',
//       ""
//     ),
//     artTradablePrivateHash: JSON.stringify(
//       json.hash["sub_art_tradable_private"]
//     ).replaceAll('"', ""),
//     nftTradableHash: JSON.stringify(json.hash["sub_nft_tradable"]).replaceAll(
//       '"',
//       ""
//     ),
//     nftTradablePrivateHash: JSON.stringify(
//       json.hash["sub_nft_tradable_private"]
//     ).replaceAll('"', ""),
//     initialSupply: 1000000000000,
//     wrappedToken: "",
//     mintFee: 10,
//     platformFee: 10,
//     feeRecipient: "",
//     endowmentAmount: 10000,
//     version: 1,
//     token: "",
//     decimals: 12,
//     owner: "",
//   });

//   const { artTradableHash } = formState;

  //   const onChange = (_, data) =>
  //     setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const txOnClickHandler = async (i) => {
    const { output, result } = await contract["mgmt"].query[
      "addressByCodeHash"
    ](contract["mgmt"].address, { gasLimit: -1, value: 0 }, hashes[i]);
    if (result.isOk && output !== undefined) {
      updateContracts(names[i], output.toString());
    }

    console.log(i, names[i], JSON.stringify(Object.keys(contract)), result);
  };
  const fetchContractAddressesFromChain = async () => {
    for (let i = 0; i < hashes.length; i++) {
      await txOnClickHandler(i);
    }
  };

  const names = [
    "addressRegistry",
    "artFactory",
    "artFactoryPrivate",
    "artion",
    "auction",
    "bundleMarketplace",
    "marketplace",
    "nftFactory",
    "nftFactoryPrivate",
    "priceSeed",
    "tokenRegistry",
    "erc20",
    "oracle",
  ];
  const hashes = Object.keys(json.hash)
    .filter((x) => -1 === json.exclude.indexOf(x))
    .map((x) => JSON.stringify(json.hash[x]).replaceAll('"', ""));
  const tradable_hashes = json.exclude
    .slice(1)
    .map((x) => JSON.stringify(json.hash[x]).replaceAll('"', ""));

  return (
    <Grid.Column>
      <Form>
        <Form.Group widths="equal">
          <Form.Field>
            <TxButton
              label="updateParameters"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateParameters",
                inputParams: tradable_hashes,
                paramFields: [],
              }}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="instantiateContracts"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "instantiateContracts",
                inputParams: hashes,
                paramFields: [],
              }}
            />
          </Form.Field>
          <Form.Field>
            <Button
              basic
              size="medium"
              onClick={(_) => fetchContractAddressesFromChain()}
            >
              fetchContractAddressesFromChain
            </Button>
          </Form.Field>
        </Form.Group>

        <Form.Group widths="equal">
          <Form.Field>
            <TxButton
              label="addressesByCodeHashes"
              type="QUERYC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "addressByCodeHash",
                inputParams: hashes,
                paramFields: [],
              }}
            />
          </Form.Field>
        </Form.Group>
      </Form>
      <div style={{ hidden: "hidden", overflowWrap: "break-word" }}>
        {status}
      </div>
    </Grid.Column>
  );
}
