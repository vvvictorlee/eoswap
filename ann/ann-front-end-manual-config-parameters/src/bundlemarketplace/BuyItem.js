import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({ bundleId: "", payToken: "" });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { bundleId, payToken } = formState;


  return (
    <Grid.Column>
      <h1>Buy Item</h1>
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

        <Form.Field >
          <TxButton
            label="buyItem"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "bundleMarketplace",
              callable: "buyItem",
              inputParams: [bundleId, payToken],
              paramFields: [true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
