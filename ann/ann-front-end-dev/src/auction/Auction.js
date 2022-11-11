import React from "react";
import { Grid } from "semantic-ui-react";

import CancelAuction from "./CancelAuction";
import CreateAuction from "./CreateAuction";
import PlaceBid from "./PlaceBid";
import PlaceBidNative from "./PlaceBidNative";
import ResultAuction from "./ResultAuction";
import WithdrawBid from "./WithdrawBid";
import UpdateParameters from "./UpdateParameters";
export default function Main(props) {
  return (
    <Grid>
    <Grid.Row columns="equal">
      <h1> Auction</h1>
      <CreateAuction {...props}/>
      <CancelAuction {...props}/>
    </Grid.Row>
    <Grid.Row columns="equal">
      <h1> Auction</h1>
      <PlaceBid {...props}/>
      <PlaceBidNative {...props}/>
    </Grid.Row>
    <Grid.Row columns="equal">
      <h1> Auction</h1>
      <ResultAuction {...props}/>
      <WithdrawBid {...props}/>
    </Grid.Row>
    <Grid.Row columns="equal">
      <h1> Auction</h1>
      <UpdateParameters {...props}/>
    </Grid.Row>
    </Grid>
  );
}
