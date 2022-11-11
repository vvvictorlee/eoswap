import React, { useState } from "react";
import { Grid, Input, Dropdown } from "semantic-ui-react";
import { TxButton } from "../substrate-lib/components";

export default function Main(props) {
const {availableAccounts}=props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    nftAddress: "",
    tokenId: "",
    reservePrice: 10,
    minBidIncrement: 10,
    bidWithdrawalLockTime: 10,
    startTime: Math.round(new Date() / 1000),
    endTime: Math.round(new Date() / 1000)+300,
    platformFee: 10,
    feeRecipient: "",
    addressRegistry: "",
  });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const {
    nftAddress,
    tokenId,
    reservePrice,
    minBidIncrement,
    bidWithdrawalLockTime,
    startTime,
    endTime,
    platformFee,
    feeRecipient,
    addressRegistry,
  } = formState;



  return (
    <Grid.Column>
      <h1>Update Parameters</h1>
      <Grid>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Dropdown
              placeholder="Select NFT address"
              fluid
              selection
              search
              options={availableAccounts}
              state="nftAddress"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <Input
              fluid
              label="NFT Address"
              type="text"
              placeholder="NFT Address"
              value={nftAddress}
              state="nftAddress"
              onChange={onChange}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="tokenId"
              type="text"
              placeholder="tokenId"
              value={tokenId}
              state="tokenId"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <Input
              fluid
              label="reservePrice"
              type="text"
              placeholder="reservePrice"
              value={reservePrice}
              state="reservePrice"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updateAuctionreservePrice"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "updateAuctionreservePrice",
                inputParams: [nftAddress, tokenId, reservePrice],
                paramFields: [true, true, true],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="minBidIncrement"
              type="text"
              placeholder="minBidIncrement"
              value={minBidIncrement}
              state="minBidIncrement"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updateMinBidIncrement"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "updateMinBidIncrement",
                inputParams: [minBidIncrement],
                paramFields: [true],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="bidWithdrawalLockTime"
              type="text"
              placeholder="bidWithdrawalLockTime"
              value={bidWithdrawalLockTime}
              state="bidWithdrawalLockTime"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updateMinBidWithdrawalLockTime"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "updateMinBidWithdrawalLockTime",
                inputParams: [bidWithdrawalLockTime],
                paramFields: [true],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="startTime"
              type="text"
              placeholder="startTime"
              value={startTime}
              state="startTime"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updateAuctionStartTime"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "updateAuctionStartTime",
                inputParams: [nftAddress, tokenId, startTime],
                paramFields: [true, true, true],
              }}
            />
          </Grid.Column>
          <Grid.Column>
            <Input
              fluid
              label="endTime"
              type="text"
              placeholder="endTime"
              value={endTime}
              state="endTime"
              onChange={onChange}
            />
          </Grid.Column>

          <Grid.Column>
            <TxButton
              label="updateAuctionEndTime"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "updateAuctionEndTime",
                inputParams: [nftAddress, tokenId, endTime],
                paramFields: [true, true, true],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="feeRecipient"
              type="text"
              placeholder="feeRecipient"
              value={feeRecipient}
              state="feeRecipient"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updatePlatformFeeRecipient"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "updatePlatformFeeRecipient",
                inputParams: [feeRecipient],
                paramFields: [true],
              }}
            />
          </Grid.Column>
          <Grid.Column>
            <Input
              fluid
              label="addressRegistry"
              type="text"
              placeholder="addressRegistry"
              value={addressRegistry}
              state="addressRegistry"
              onChange={onChange}
            />
          </Grid.Column>
          <TxButton
            label="updateAddressRegistry"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "auction",
              callable: "updateAddressRegistry",
              inputParams: [addressRegistry],
              paramFields: [true],
            }}
          />
        </Grid.Row>
        <Grid.Row columns="equal">
          <Grid.Column>
            <Input
              fluid
              label="platformFee"
              type="text"
              placeholder="platformFee"
              value={platformFee}
              state="platformFee"
              onChange={onChange}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="updatePlatformFee"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "updatePlatformFee",
                inputParams: [platformFee],
                paramFields: [true],
              }}
            />
          </Grid.Column>
          <Grid.Column>
            <TxButton
              label="toggleIsPaused"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "auction",
                callable: "toggleIsPaused",
                inputParams: [],
                paramFields: [],
              }}
            />
          </Grid.Column>
        </Grid.Row>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Grid>
    </Grid.Column>
  );
}
