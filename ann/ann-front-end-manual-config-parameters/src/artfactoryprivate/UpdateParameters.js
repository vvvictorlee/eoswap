import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    marketplaceAddress: "",
    bundleMarketplaceAddress: "",
    mintFee: 10,
    platformFee: 10,
    feeRecipient: "",
    endowmentAmount: 10000,
  });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const {
    marketplaceAddress,
    bundleMarketplaceAddress,
    mintFee,
    platformFee,
    feeRecipient,
    endowmentAmount,
  } = formState;

 

  return (
    <Grid.Column>
      <h1>Update Parameters</h1>

      <Form>
        <Form.Group widths="equal">
          <Form.Field >
            <Input
              placeholder="marketplaceAddress"
              fluid
              type="text"
              label="marketplaceAddress"
              state="marketplaceAddress"
              value={marketplaceAddress}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field >
            <TxButton
              label="updateMarketplaceAddress"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "artFactoryPrivate",
                callable: "updateMarketplaceAddress",
                inputParams: [marketplaceAddress],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field >
            <Input
              placeholder="bundleMarketplaceAddress"
              fluid
              type="text"
              label="bundleMarketplaceAddress"
              state="bundleMarketplaceAddress"
              value={bundleMarketplaceAddress}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field >
            <TxButton
              label="updateBundleMarketplaceAddress"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "artFactoryPrivate",
                callable: "updateBundleMarketplaceAddress",
                inputParams: [bundleMarketplaceAddress],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
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
                palletRpc: "artFactoryPrivate",
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
                palletRpc: "artFactoryPrivate",
                callable: "updatePlatformFee",
                inputParams: [platformFee],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field >
            <Input
              placeholder="mint fee"
              fluid
              type="text"
              label="mint fee"
              state="mintFee"
              value={mintFee}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field >
            <TxButton
              label="Update mint fee"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "artFactoryPrivate",
                callable: "updateMintFee",
                inputParams: [mintFee],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field >
            <Input
              placeholder="endowmentAmount"
              fluid
              type="text"
              label="endowmentAmount"
              state="endowmentAmount"
              value={endowmentAmount}
              onChange={onChange}
            />
          </Form.Field>

          <Form.Field >
            <TxButton
              label="Update endowment Amount"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "artFactoryPrivate",
                callable: "updateEndowmentAmount",
                inputParams: [endowmentAmount],
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
