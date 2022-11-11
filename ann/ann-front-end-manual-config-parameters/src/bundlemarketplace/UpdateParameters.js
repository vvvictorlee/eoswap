import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";
import { useSubstrateState } from "../substrate-lib";

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    platformFee: 10,
    feeRecipient: "",
    addressRegistry: "",
  });
  const { keyring } = useSubstrateState();
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { platformFee, feeRecipient, addressRegistry } = formState;

  const accounts = keyring.getPairs();

  const availableAccounts = [];
  accounts.map((account) => {
    return availableAccounts.push({
      key: account.meta.name,
      text: account.meta.name,
      value: account.address,
    });
  });

  return (
    <Grid.Column>
      <h1>Update Parameters</h1>
      <Form>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              fluid
              label="feeRecipient"
              type="text"
              placeholder="feeRecipient"
              value={feeRecipient}
              state="feeRecipient"
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field >
            <TxButton
              label="updatePlatformFeeRecipient"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "bundleMarketplace",
                callable: "updatePlatformFeeRecipient",
                inputParams: [feeRecipient],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              fluid
              label="addressRegistry"
              type="text"
              placeholder="addressRegistry"
              value={addressRegistry}
              state="addressRegistry"
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field >
            <TxButton
              label="updateAddressRegistry"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "bundleMarketplace",
                callable: "updateAddressRegistry",
                inputParams: [addressRegistry],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
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
              label="updatePlatformFee"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "bundleMarketplace",
                callable: "updatePlatformFee",
                inputParams: [platformFee],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
