import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    nftAddress: "",
    tokenId: "",
    bidAmount: 10,
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { nftAddress, tokenId, bidAmount } = formState;

    return (
    <Grid.Column>
      <h1>Place Bid Native</h1>
      <Form>
        <Form.Field>
          <Dropdown
            placeholder="Select NFT address"
            fluid
            selection
            search
            options={addresses}
            state="nftAddress"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="NFT Address"
            type="text"
            placeholder="NFT Address"
            value={nftAddress}
            state="nftAddress"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="tokenId"
            type="text"
            placeholder="tokenId"
            value={tokenId}
            state="tokenId"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="bidAmount"
            type="text"
            placeholder="bidAmount"
            value={bidAmount}
            state="bidAmount"
            onChange={onChange}
          />
        </Form.Field>

        <Form.Field >
          <TxButton
            label="placeBidNative"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "auction",
              callable: "placeBidNative",
              inputParams: [nftAddress, tokenId, bidAmount],
              paramFields: [true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
