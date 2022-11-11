import React, { useState } from "react";
import { Form, Input, Grid } from "semantic-ui-react";
import { TxButton } from "./substrate-lib/components";

export default function Main(props) {
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    artion: "",
    auction: "",
    marketplace: "",
    bundleMarketplace: "",
    artFactory: "",
    artFactoryPrivate: "",
    nftFactory: "",
    nftFactoryPrivate: "",
    priceSeed: "",
    tokenRegistry: "",
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const {
    artion,
    auction,
    marketplace,
    bundleMarketplace,
    artFactory,
    artFactoryPrivate,
    nftFactory,
    nftFactoryPrivate,
    priceSeed,
    tokenRegistry,
  } = formState;

  return (
    <Grid.Column>
      <h1>AddressRegistry</h1>
      <Form>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="artion"
              fluid
              type="text"
              label="artion"
              state="artion"
              value={artion}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateArtion"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateArtion",
                inputParams: [artion],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="auction"
              fluid
              type="text"
              label="auction"
              state="auction"
              value={auction}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateAuction"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateAuction",
                inputParams: [auction],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="marketplace"
              fluid
              type="text"
              label="marketplace"
              state="marketplace"
              value={marketplace}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateMarketplace"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateMarketplace",
                inputParams: [marketplace],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="bundleMarketplace"
              fluid
              type="text"
              label="bundleMarketplace"
              state="bundleMarketplace"
              value={bundleMarketplace}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateBundleMarketplace"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateBundleMarketplace",
                inputParams: [bundleMarketplace],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="artFactory"
              fluid
              type="text"
              label="artFactory"
              state="artFactory"
              value={artFactory}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateArtFactory"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateArtFactory",
                inputParams: [artFactory],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="artFactoryPrivate"
              fluid
              type="text"
              label="artFactoryPrivate"
              state="artFactoryPrivate"
              value={artFactoryPrivate}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateArtFactoryPrivate"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateArtFactoryPrivate",
                inputParams: [artFactoryPrivate],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="nftFactory"
              fluid
              type="text"
              label="nftFactory"
              state="nftFactory"
              value={nftFactory}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateNftFactory"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateNftFactory",
                inputParams: [nftFactory],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="nftFactoryPrivate"
              fluid
              type="text"
              label="nftFactoryPrivate"
              state="nftFactoryPrivate"
              value={nftFactoryPrivate}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateNftFactoryPrivate"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateNftFactoryPrivate",
                inputParams: [nftFactoryPrivate],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="priceSeed"
              fluid
              type="text"
              label="priceSeed"
              state="priceSeed"
              value={priceSeed}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updatePriceSeed"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updatePriceSeed",
                inputParams: [priceSeed],
                paramFields: [true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Input
              placeholder="tokenRegistry"
              fluid
              type="text"
              label="tokenRegistry"
              state="tokenRegistry"
              value={tokenRegistry}
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="updateTokenRegistry"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "addressRegistry",
                callable: "updateTokenRegistry",
                inputParams: [tokenRegistry],
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
