import React, { useState } from "react";
import { Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,}=props;

  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    nftAddress: "",
    tokenId: "",
    royalty: 10,
    creator: "",
    platformFee: 10,
    feeRecipient: "",
    addressRegistry: "",
    seller: "",
    buyer: "",
  });
  
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const {
    nftAddress,
    tokenId,
    royalty,
    creator,
    platformFee,
    feeRecipient,
    addressRegistry,
    seller,
    buyer,
  } = formState;


  return (
    <Grid.Column>
      <h1>Update Parameters</h1>
      <Grid>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Dropdown
              placeholder="Select NFT address"
              fluid
              multiple
              selection
              search
              options={addresses}
              state="nftAddress"
              onChange={onChange}
            />
            <Input
              fluid
              label="nftAddress"
              type="text"
              placeholder="nftAddress"
              value={nftAddress}
              state="nftAddress"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <Input
              fluid
              label="tokenId"
              type="text"
              placeholder="tokenId"
              value={tokenId}
              state="tokenId"
              onChange={onChange}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="seller"
              type="text"
              placeholder="seller"
              value={seller}
              state="seller"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <Input
              fluid
              label="buyer"
              type="text"
              placeholder="buyer"
              value={buyer}
              state="buyer"
              onChange={onChange}
            />
          </Grid.Column>
          <TxButton
            label="validateItemSold"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "marketplace",
              callable: "validateItemSold",
              inputParams: [addressRegistry],
              paramFields: [true],
            }}
          />
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="royalty"
              type="text"
              placeholder="royalty"
              value={royalty}
              state="royalty"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="registerRoyalty"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "marketplace",
                callable: "registerRoyalty",
                inputParams: [nftAddress, tokenId, royalty],
                paramFields: [true, true, true],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="feeRecipient"
              type="text"
              placeholder="feeRecipient"
              value={feeRecipient}
              state="feeRecipient"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updatePlatformFeeRecipient"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "marketplace",
                callable: "updatePlatformFeeRecipient",
                inputParams: [feeRecipient],
                paramFields: [true],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="creator"
              type="text"
              placeholder="creator"
              value={creator}
              state="creator"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="registerCollectionRoyalty"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "marketplace",
                callable: "registerCollectionRoyalty",
                inputParams: [nftAddress, creator, royalty, feeRecipient],
                paramFields: [true, true, true, true],
              }}
            />
          </Grid.Column>
        </Grid.Row>

        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="addressRegistry"
              type="text"
              placeholder="addressRegistry"
              value={addressRegistry}
              state="addressRegistry"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updateAddressRegistry"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "marketplace",
                callable: "updateAddressRegistry",
                inputParams: [addressRegistry],
                paramFields: [true],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="platformFee"
              type="text"
              placeholder="platformFee"
              value={platformFee}
              state="platformFee"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updatePlatformFee"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "marketplace",
                callable: "updatePlatformFee",
                inputParams: [platformFee],
                paramFields: [true],
              }}
            />
          </Grid.Column>
        </Grid.Row>

        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Grid>
    </Grid.Column>
  );
}
