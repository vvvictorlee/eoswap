import React from "react";
import { Grid } from "semantic-ui-react";

import ListItem from "./ListItem";
import UpdateListing from "./UpdateListing";
import CancelListing from "./CancelListing";
import BuyItem from "./BuyItem";
import CreateOffer from "./CreateOffer";
import AcceptOffer from "./AcceptOffer";
import CancelOffer from "./CancelOffer";
import UpdateParameters from "./UpdateParameters";
export default function Main(props) {
  return (
    <Grid>
      <Grid.Row columns="equal">
        <h1>BundleMarketplace</h1>
        <ListItem {...props}/>
        <UpdateListing {...props}/>
      </Grid.Row>
      <Grid.Row columns="equal">
        <h1>BundleMarketplace</h1>
        <CancelListing {...props}/>
        <BuyItem {...props}/>
      </Grid.Row>
      <Grid.Row columns="equal">
        <h1>BundleMarketplace</h1>
        <CreateOffer {...props}/>
        <CancelOffer {...props}/>
        <AcceptOffer {...props}/>
      </Grid.Row>
      <Grid.Row columns="equal">
        <h1>BundleMarketplace</h1>
        <UpdateParameters {...props}/>
      </Grid.Row>
    </Grid>
  );
}
