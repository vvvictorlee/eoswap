import React, { useState } from "react";
import { Form, Input, Grid ,Dropdown} from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {addresses,}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    nftAddress: "",
    tokenId: "",
    creator: "",
  });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { nftAddress, tokenId, creator } = formState;


  return (
    <Grid.Column>
      <h1>AcceptOffer</h1>
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
            label="nftAddress"
            type="text"
            placeholder="nftAddress"
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
              palletRpc: "marketplace",
              callable: "acceptOffer",
              inputParams: [nftAddress, tokenId, creator],
              paramFields: [true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
