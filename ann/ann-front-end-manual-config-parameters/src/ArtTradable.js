import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "./substrate-lib/components";

export default function Main(props) {
  const { addresses, availableAccounts } = props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    nftAddress: "",
    owner: "",
    operator: "",
    to: "",
    supply: 1,
    uri: "",
    approved: "true",
    platformFee: 10,
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { nftAddress, owner, operator, to, supply, uri, platformFee,approved } =
    formState;

  return (
    <Grid.Column>
      <h1>artTradable</h1>
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
          <Dropdown
            placeholder="owner"
            fluid
            selection
            search
            options={availableAccounts}
            state="owner"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="owner"
            type="text"
            placeholder="owner"
            value={owner}
            state="owner"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Dropdown
            placeholder="operator"
            fluid
            selection
            search
            options={availableAccounts}
            state="operator"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="operator"
            type="text"
            placeholder="operator"
            value={operator}
            state="operator"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="approved"
            type="text"
            placeholder="approved"
            value={approved}
            state="approved"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Group widths="equal">
          <Form.Field>
            <TxButton
              label="setApprovalForAll"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                contractAddress: nftAddress,
                palletRpc: "artTradable",
                callable: "erc1155::setApprovalForAll",
                inputParams: [operator,approved==="true"],
                paramFields: [true, true],
              }}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="isApprovedForAll"
              type="QUERYC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "artTradable",
                callable: "erc1155::isApprovedForAll",
                contractAddress: nftAddress,
                inputParams: [owner,operator],
                paramFields: [true,true],
              }}
            />
          </Form.Field>
        </Form.Group>
        <Form.Field>
          <Dropdown
            placeholder="to"
            fluid
            selection
            search
            options={availableAccounts}
            state="to"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="to"
            type="text"
            placeholder="to"
            value={to}
            state="to"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="supply"
            type="text"
            placeholder="supply"
            value={supply}
            state="supply"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="uri"
            type="text"
            placeholder="uri"
            value={uri}
            state="uri"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <Input
            fluid
            label="platformFee"
            type="text"
            placeholder="platformFee"
            value={platformFee}
            state="platformFee"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Field>
          <TxButton
            label="mintArt"
            type="SIGNED-TXC"
            setStatus={setStatus}
            attrs={{
              palletRpc: "artTradable",
              callable: "mintArt",
              transferredValue: platformFee,
              contractAddress: nftAddress,
              inputParams: [to, supply, uri],
              paramFields: [true, true, true],
            }}
          />
        </Form.Field>
        <div style={{ overflowWrap: "break-word" }}>{status}</div>
      </Form>
    </Grid.Column>
  );
}
