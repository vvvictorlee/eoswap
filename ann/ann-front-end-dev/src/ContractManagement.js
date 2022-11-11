import React, { useState } from "react";
import { Grid, Form, Table, Input, Button } from "semantic-ui-react";
import { useSubstrate, useSubstrateState } from "./substrate-lib";
import { TxButton } from "./substrate-lib/components";

const json = require("./abi/art.json");

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const { contract } = useSubstrateState();
  const { updateContracts } = useSubstrate();
  const [formState, setFormState] = useState({
    addressRegistry: "",
    auctionAddress: "",
    marketplaceAddress: "",
    bundleMarketplaceAddress: "",
    artTradableHash: JSON.stringify(json.hash["sub_art_tradable"]).replaceAll(
      '"',
      ""
    ),
    artTradablePrivateHash: JSON.stringify(
      json.hash["sub_art_tradable_private"]
    ).replaceAll('"', ""),
    nftTradableHash: JSON.stringify(json.hash["sub_nft_tradable"]).replaceAll(
      '"',
      ""
    ),
    nftTradablePrivateHash: JSON.stringify(
      json.hash["sub_nft_tradable_private"]
    ).replaceAll('"', ""),
    initialSupply: 1000000000000,
    wrappedToken: "",
    mintFee: 10,
    platformFee: 10,
    feeRecipient: "",
    endowmentAmount: 10000,
    version: 1,
    token: "",
    decimals: 12,
    owner: "",
  });

  const {
    addressRegistry,
    auctionAddress,
    marketplaceAddress,
    bundleMarketplaceAddress,
    artTradableHash,
    artTradablePrivateHash,
    nftTradableHash,
    nftTradablePrivateHash,
    initialSupply,
    wrappedToken,
    mintFee,
    platformFee,
    feeRecipient,
    endowmentAmount,
    version,
    token,
    decimals,
  } = formState;

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

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
  //   const message_indices = Array(hashes.length)
  //     .fill()
  //     .map((_, i) => i);
  //   console.log(message_indices)
  //   console.log(hashes)
  //   console.log(Object.keys(json.hash))
  //   console.log(contract["mgmt"].abi.messages)
  return (
    <Grid.Column>
      <Form>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="mint fee"
              fluid
              type="text"
              label="mint fee"
              state="mintFee"
              value={mintFee}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="Update mint fee"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateMintFee",
                inputParams: [mintFee],
                paramFields: [true],
              }}
            />
          </Form.Field>

          <Form.Field>
            <Input
              placeholder="platform fee"
              fluid
              type="text"
              label="platform fee"
              state="platformFee"
              value={platformFee}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="Update platform fee"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updatePlatformFee",
                inputParams: [platformFee],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="feeRecipient"
              fluid
              type="text"
              label="feeRecipient"
              state="feeRecipient"
              value={feeRecipient}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="Update fee recipient"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updatePlatformFeeRecipient",
                inputParams: [feeRecipient],
                paramFields: [true],
              }}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="transferOwnershipOfAddressRegistry"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "transferOwnershipOfAddressRegistry",
                inputParams: [],
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
            <Input
              placeholder="initialSupply"
              fluid
              type="text"
              label="initialSupply"
              state="initialSupply"
              value={initialSupply}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="Update initialSupply"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateInitialSupply",
                inputParams: [initialSupply],
                paramFields: [true],
              }}
            />
          </Form.Field>

          <Form.Field>
            <Input
              placeholder="wrappedToken"
              fluid
              type="text"
              label="wrappedToken"
              state="wrappedToken"
              value={wrappedToken}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="Update wrapped Token"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateWrappedToken",
                inputParams: [wrappedToken],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="endowmentAmount"
              fluid
              type="text"
              label="endowmentAmount"
              state="endowmentAmount"
              value={endowmentAmount}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="Update endowment Amount"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateEndowmentAmount",
                inputParams: [endowmentAmount],
                paramFields: [true],
              }}
            />
          </Form.Field>
          <Form.Field>
            <Input
              placeholder="version"
              fluid
              type="text"
              label="version"
              state="version"
              value={version}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="Update version"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateVersion",
                inputParams: [version],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="token"
              fluid
              type="text"
              label="token"
              state="token"
              value={token}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="Update token"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateToken",
                inputParams: [token],
                paramFields: [true],
              }}
            />
          </Form.Field>
          <Form.Field>
            <Input
              placeholder="decimals"
              fluid
              type="text"
              label="decimals"
              state="decimals"
              value={decimals}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="Update decimals"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateDecimals",
                inputParams: [decimals],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="artTradableHash"
              fluid
              type="text"
              label="artTradableHash"
              state="artTradableHash"
              value={artTradableHash}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="UpdateartTradableHash"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateArtTradableHash",
                inputParams: [artTradableHash],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="artTradablePrivateHash"
              fluid
              type="text"
              label="artTradablePrivateHash"
              state="artTradablePrivateHash"
              value={artTradablePrivateHash}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="updateArtTradablePrivateHash"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateArtTradablePrivateHash",
                inputParams: [artTradablePrivateHash],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="nftTradableHash"
              fluid
              type="text"
              label="nftTradableHash"
              state="nftTradableHash"
              value={nftTradableHash}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="UpdatenftTradableHash"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateNftTradableHash",
                inputParams: [nftTradableHash],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="nftTradablePrivateHash"
              fluid
              type="text"
              label="nftTradablePrivateHash"
              state="nftTradablePrivateHash"
              value={nftTradablePrivateHash}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="updateNftTradablePrivateHash"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateNftTradablePrivateHash",
                inputParams: [nftTradablePrivateHash],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="addressRegistry"
              fluid
              type="text"
              label="addressRegistry"
              state="addressRegistry"
              value={addressRegistry}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="updateAddressRegistry"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateAddressRegistry",
                inputParams: [addressRegistry],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="auctionAddress"
              fluid
              type="text"
              label="auctionAddress"
              state="auctionAddress"
              value={auctionAddress}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="updateAuctionAddress"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateAuctionAddress",
                inputParams: [auctionAddress],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="marketplaceAddress"
              fluid
              type="text"
              label="marketplaceAddress"
              state="marketplaceAddress"
              value={marketplaceAddress}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="updateMarketplaceAddress"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateMarketplaceAddress",
                inputParams: [marketplaceAddress],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="bundleMarketplaceAddress"
              fluid
              type="text"
              label="bundleMarketplaceAddress"
              state="bundleMarketplaceAddress"
              value={bundleMarketplaceAddress}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field>
            <TxButton
              label="updateBundleMarketplaceAddress"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "mgmt",
                callable: "updateBundleMarketplaceAddress",
                inputParams: [bundleMarketplaceAddress],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
      </Form>
      <Table celled striped size="small">
        <Table.Body>
          {contract == null || contract["mgmt"] === null ? (
            <Table.Row>
              <Table.Cell>Nocontracttobeshown</Table.Cell>
            </Table.Row>
          ) : (
            hashes.map((hash, index) => (
              <Table.Row key={hash}>
                <Table.Cell width={10} textAlign="right">
                  <Input
                    placeholder={contract["mgmt"].abi.messages[index].method}
                    fluid
                    type="text"
                    label="Hash"
                    value={hash}
                  />
                </Table.Cell>
                <Table.Cell width={5} textAlign="right">
                  <TxButton
                    label={contract["mgmt"].abi.messages[index].method}
                    type="SIGNED-TXC"
                    setStatus={setStatus}
                    txOnClickHandler={(i) => txOnClickHandler(index, i)}
                    attrs={{
                      palletRpc: "mgmt",
                      callable: contract["mgmt"].abi.messages[index].method,
                      inputParams: [hash],
                      paramFields: [true],
                    }}
                  />
                </Table.Cell>
              </Table.Row>
            ))
          )}
        </Table.Body>
      </Table>

      <div style={{ hidden: "hidden", overflowWrap: "break-word" }}>
        {status}
      </div>
    </Grid.Column>
  );
}
