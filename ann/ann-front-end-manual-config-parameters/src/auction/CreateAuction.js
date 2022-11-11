import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";
import { BN } from "@polkadot/util";

export default function Main(props) {
const {addresses,}=props;
  const [status, setStatus] = useState(null);
  const [form, setFormState] = useState({
    nftAddress: "",
    tokenId: 0,
    payToken: "",
    reservePrice: 10,
    startTime: Math.round(new Date() / 1000),
    minBidReserve: false,
    endTime: Math.round(new Date() / 1000)+300,
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const {
    nftAddress,
    tokenId,
    payToken,
    reservePrice,
    startTime,
    minBidReserve,
    endTime,
  } = form;

  
  return (
    <Grid.Column>
      <h1>create auction</h1>
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
            label="reservePrice"
            type="text"
            placeholder="reservePrice"
            value={reservePrice}
            state="reservePrice"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="startTime"
            type="text"
            placeholder="startTime"
            value={startTime}
            state="startTime"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="minBidReserve"
            type="text"
            placeholder="minBidReserve"
            value={minBidReserve}
            state="minBidReserve"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="endTime"
            type="text"
            placeholder="endTime"
            value={endTime}
            state="endTime"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <TxButton
            label="createAuction"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "auction",
              callable: "createAuction",
              inputParams: [
                nftAddress,
                new BN(tokenId),
                payToken,
                new BN(reservePrice),
                new BN(startTime),
                minBidReserve==="true",
                new BN(endTime),
              ],
              paramFields: [true, true, true, true, true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
