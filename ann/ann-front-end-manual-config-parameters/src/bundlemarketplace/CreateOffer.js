import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {availableAccounts}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    bundleId: "",
    payToken: "",
    price: 10,
    deadline: Math.round(new Date() / 1000)+1000,
  });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { bundleId, payToken, price, deadline } = formState;

  return (
    <Grid.Column>
      <h1>CreateOffer</h1>
      <Form>
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
              palletRpc: "bundleMarketplace",
              callable: "createOffer",
              inputParams: [bundleId, payToken, price, deadline],
              paramFields: [true, true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
