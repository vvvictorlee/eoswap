import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,}=props;

  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    nftAddress: "",
    tokenId: "",
    quantity: 10,
    payToken: "",
    pricePerItem: 10,
    deadline: Math.round(new Date() / 1000)+1000,
  });
 
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { nftAddress, tokenId, quantity, payToken, pricePerItem, deadline } =
    formState;


  return (
    <Grid.Column>
      <h1>CreateOffer</h1>
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
            label="pricePerItem"
            type="text"
            placeholder="pricePerItem"
            value={pricePerItem}
            state="pricePerItem"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="deadline"
            type="text"
            placeholder="deadline"
            value={deadline}
            state="deadline"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field >
          <TxButton
            label="createOffer"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "marketplace",
              callable: "createOffer",
              inputParams: [
                nftAddress,
                tokenId,
                payToken,
                pricePerItem,
                deadline,
              ],
              paramFields: [true, true, true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
