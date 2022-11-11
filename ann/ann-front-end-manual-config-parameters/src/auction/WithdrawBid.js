import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({ nftAddress: "", tokenId: "" });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { nftAddress, tokenId } = formState;



  return (
    <Grid.Column>
      <h1>Withdraw Bid</h1>
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
        <Form.Field >
          <TxButton
            label="placeBid"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "auction",
              callable: "withdrawBid",
              inputParams: [nftAddress, tokenId],
              paramFields: [true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
