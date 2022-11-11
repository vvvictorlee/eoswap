import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,availableAccounts}=props;
  const [status, setStatus] = useState(null);
  const [form, setFormState] = useState({
    bundleId: "",
    nftAddresses: [""],
    tokenIds: [""],
    quantities: [0],
    payToken: "",
    price: 10,
    startTime: Math.round(new Date() / 1000),
  });
  
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const {
    bundleId,
    nftAddresses,
    tokenIds,
    quantities,
    payToken,
    price,
    startTime,
  } = form;

 

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
            state="nftAddresses"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="NFT Address"
            type="text"
            placeholder="NFT Address"
            value={nftAddresses}
            state="nftAddresses"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="tokenIds"
            type="text"
            placeholder="tokenIds"
            value={tokenIds}
            state="tokenIds"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="quantities"
            type="text"
            placeholder="quantities"
            value={quantities}
            state="quantities"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="bundleId"
            type="text"
            placeholder="bundleId"
            value={bundleId}
            state="bundleId"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Dropdown
            placeholder="Select pay token"
            fluid
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
              palletRpc: "bundleMarketplace",
              callable: "listItem",
              inputParams: [
                bundleId,
                nftAddresses,
                tokenIds,
                quantities,
                payToken,
                price,
                startTime,
              ],
              paramFields: [true, true, true, true, true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
