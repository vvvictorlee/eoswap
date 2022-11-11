import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";
export default function Main(props) {
const {txOnClickHandler}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({ tokenContract: "" });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { tokenContract } = formState;


  return (
    <Grid.Column>
      <h1>DisableTokenContract</h1>
      <Form>
        <Form.Field>
          <Input
            fluid
            label="tokenContract"
            type="text"
            placeholder="tokenContract"
            value={tokenContract}
            state="tokenContract"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field >
          <TxButton
            label="disableTokenContract"
            type="SIGNED-TXC"
            setStatus={setStatus}
            txOnClickHandler={(i) => txOnClickHandler("artFactoryPrivate","artTradablePrivate",i)}
            attrs={{
              palletRpc: "artFactoryPrivate",
              callable: "disableTokenContract",
              inputParams: [tokenContract],
              paramFields: [true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
