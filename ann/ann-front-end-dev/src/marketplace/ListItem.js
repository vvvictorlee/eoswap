import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,availableAccounts}=props;

  const [status, setStatus] = useState(null);
  const [form, setFormState] = useState({
    nftAddress: "",
    tokenId: "",
    quantity: 10,
    payToken: "",
    pricePerItem: 10,
    startTime: Math.round(new Date() / 1000),
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { nftAddress, tokenId, quantity, payToken, price, startTime } = form;

 

  return (
    <Grid.Column>
      <h1>ListItem</h1>
      <Form>
        <Form.Field>
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
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="NFT Address"
            type="text"
            placeholder="NFT Address"
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
          <Input
            fluid
            label="quantity"
            type="text"
            placeholder="quantity"
            value={quantity}
            state="quantity"
            onChange={onChange}
          />
        </Form.Field>
      <Form.Field>
          <Dropdown
            placeholder="Select pay Token"
            fluid
            multiple
            selection
            search
            options={availableAccounts}
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
            label="price"
            type="text"
            placeholder="price"
            value={price}
            state="price"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="startTime"
            type="text"
            placeholder="startTime"
            value={startTime}
            state="startTime"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field >
          <TxButton
            label="listItem"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "marketplace",
              callable: "listItem",
              inputParams: [
                nftAddress,
                tokenId,
                quantity,
                payToken,
                price,
                startTime,
              ],
              paramFields: [true, true, true, true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
