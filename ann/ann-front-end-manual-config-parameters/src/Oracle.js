import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "./substrate-lib/components";

export default function Main(props) {
  const { oracleAddresses } = props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    oracleAddress: "",
    price: 1,
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { oracleAddress, price } = formState;

  return (
    <Grid.Column>
      <h1>Oracle</h1>
      <Form>
        <Form.Group widths="equal">
          <Form.Field>
            <Dropdown
              placeholder="Select Oracle address"
              fluid
              selection
              search
              options={oracleAddresses}
              state="oracleAddress"
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <Input
              fluid
              label="oracleAddress"
              type="text"
              placeholder="oracleAddress"
              value={oracleAddress}
              state="oracleAddress"
              onChange={onChange}
            />
          </Form.Field>
        </Form.Group>
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
        <Form.Group widths="equal">
          <Form.Field>
            <TxButton
              label="update"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                contractAddress: oracleAddress,
                palletRpc: "oracle",
                callable: "update",
                inputParams: [ price],
                paramFields: [ true],
              }}
            />
          </Form.Field>
        </Form.Group>

        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
