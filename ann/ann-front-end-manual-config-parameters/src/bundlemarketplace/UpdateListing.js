import React, { useState } from "react";
import { Form, Input, Grid,Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {availableAccounts}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    payToken: "",
    bundleId: "",
    newPrice: 10,
  });
 
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { payToken, bundleId, newPrice } = formState;



  return (
    <Grid.Column>
      <h1>UpdateListing</h1>
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
            label="newPrice"
            type="text"
            placeholder="newPrice"
            value={newPrice}
            state="newPrice"
            onChange={onChange}
          />
        </Form.Field>

        <Form.Field >
          <TxButton
            label="updateListing"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "bundleMarketplace",
              callable: "updateListing",
              inputParams: [bundleId, payToken, newPrice],
              paramFields: [true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
