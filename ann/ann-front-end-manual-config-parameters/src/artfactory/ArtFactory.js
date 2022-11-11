import React from "react";
import { Grid } from "semantic-ui-react";

import CreateNftContract from "./CreateNftContract";
import RegisterTokenContract from "./RegisterTokenContract";
import DisableTokenContract from "./DisableTokenContract";
import UpdateParameters from "./UpdateParameters";
export default function Main(props) {
  return (
    <Grid>
      <Grid.Row columns="equal">
        <h1>ArtFactory</h1>
        <CreateNftContract {...props}/>
        <RegisterTokenContract {...props}/>
        <DisableTokenContract {...props}/>
      </Grid.Row>
      <Grid.Row columns="equal">
        <h1>ArtFactory</h1>
        <UpdateParameters {...props}/>
      </Grid.Row>
    </Grid>
  );
}
