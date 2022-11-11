import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "./substrate-lib/components";

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    token: "",
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { token } = formState;

  return (
    <Grid.Column>
      <h1>TokenRegistry</h1>
      <Form>
        <Form.Field>
          <Input
            fluid
            label="token"
            type="text"
            placeholder="token"
            value={token}
            state="token"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field >
          <TxButton
            label="add"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "tokenRegistry",
              callable: "add",
              inputParams: [token],
              paramFields: [true],
            }}
          />
          <TxButton
            label="remove"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "tokenRegistry",
              callable: "remove",
              inputParams: [token],
              paramFields: [true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
