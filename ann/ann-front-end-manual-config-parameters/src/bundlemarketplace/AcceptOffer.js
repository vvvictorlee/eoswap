import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({ bundleId: "", creator: "" });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { bundleId, creator } = formState;

  
  return (
    <Grid.Column>
      <h1>AcceptOffer</h1>
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
            label="creator"
            type="text"
            placeholder="creator"
            value={creator}
            state="creator"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field >
          <TxButton
            label="acceptOffer"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "bundleMarketplace",
              callable: "acceptOffer",
              inputParams: [bundleId, creator],
              paramFields: [true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
