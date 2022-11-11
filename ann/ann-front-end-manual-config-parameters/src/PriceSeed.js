import React, { useState } from "react";
import { Grid, Input } from "semantic-ui-react";
import { TxButton } from "./substrate-lib/components";

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    token: "",
    oracle: "",
    addressRegistry: "",
  });
  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { token, oracle, addressRegistry } = formState;

  return (
    <Grid.Column>
      <h1>PriceSeed</h1>
      <Input
        placeholder="token"
        fluid
        type="text"
        label="token"
        state="token"
        value={token}
        onChange={onChange}
      />
      <Input
        placeholder="oracle"
        fluid
        type="text"
        label="oracle"
        state="oracle"
        value={oracle}
        onChange={onChange}
      />
      <TxButton
        label="registerOracle"
        type="SIGNED-TXC"
        setStatus={setStatus}
        attrs={{
          palletRpc: "priceSeed",
          callable: "registerOracle",
          inputParams: [token, oracle],
          paramFields: [true, true],
        }}
      />
      <TxButton
        label="updateOracle"
        type="SIGNED-TXC"
        setStatus={setStatus}
        attrs={{
          palletRpc: "priceSeed",
          callable: "updateOracle",
          inputParams: [token, oracle],
          paramFields: [true, true],
        }}
      />
      <Input
        fluid
        label="addressRegistry"
        type="text"
        placeholder="addressRegistry"
        value={addressRegistry}
        state="addressRegistry"
        onChange={onChange}
      />
      <TxButton
        label="updateAddressRegistry"
        type="SIGNED-TXC"
        setStatus={setStatus}
        attrs={{
          palletRpc: "marketplace",
          callable: "updateAddressRegistry",
          inputParams: [addressRegistry],
          paramFields: [true],
        }}
      />
      <div style={{ hidden: "hidden", overflowWrap: "break-word" }}>
        {status}
      </div>
    </Grid.Column>
  );
}
