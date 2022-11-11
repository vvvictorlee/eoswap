import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";
export default function Main(props) {
const {txOnClickHandler}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    name: "",
    symbol: "",
    platformFee: 10,
  });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { name, symbol, platformFee} = formState;


  return (
    <Grid.Column>
      <h1>CreateNftContract</h1>
      <Form>
        <Form.Field>
          <Input
            fluid
            label="name"
            type="text"
            placeholder="name"
            value={name}
            state="name"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="symbol"
            type="text"
            placeholder="symbol"
            value={symbol}
            state="symbol"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="platformFee"
            type="text"
            placeholder="platformFee"
            value={platformFee}
            state="platformFee"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field >
          <TxButton
            label="createNftContract"
            type="SIGNED-TXC"
            setStatus={setStatus}
            txOnClickHandler={(i) => txOnClickHandler("artFactoryPrivate","artTradablePrivate",i)}
            attrs={{
              palletRpc: "artFactoryPrivate",
              callable: "createNftContract",
              transferredValue: platformFee,
              inputParams: [name, symbol],
              paramFields: [true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
