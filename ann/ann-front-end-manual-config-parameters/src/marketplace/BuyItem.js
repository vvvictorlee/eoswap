import React, { useState } from "react";
import { Form, Input, Grid,Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    nftAddress: "",
    tokenId: "",
    payToken: "",
    owner: "",
  });
  
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { nftAddress, tokenId, payToken, owner } = formState;

 
  return (
    <Grid.Column>
      <h1>Buy Item</h1>
      <Form>
        <Form.Field>
          <Dropdown
            placeholder="Select NFT address"
            fluid
            selection
            search
            options={addresses}
            state="nftAddress"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="nftAddress"
            type="text"
            placeholder="nftAddress"
            value={nftAddress}
            state="nftAddress"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="tokenId"
            type="text"
            placeholder="tokenId"
            value={tokenId}
            state="tokenId"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Dropdown
            placeholder="Select payToken"
            fluid
            selection
            search
            options={addresses}
            state="payToken"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="payToken"
            type="text"
            placeholder="payToken"
            value={payToken}
            state="payToken"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="owner"
            type="text"
            placeholder="owner"
            value={owner}
            state="owner"
            onChange={onChange}
          />
        </Form.Field>

        <Form.Field >
          <TxButton
            label="buyItem"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "marketplace",
              callable: "buyItem",
              inputParams: [nftAddress, tokenId, payToken, owner],
              paramFields: [true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
