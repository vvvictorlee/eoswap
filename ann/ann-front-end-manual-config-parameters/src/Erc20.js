import React, { useState } from "react";
import { Form, Input, Grid, Dropdown } from "semantic-ui-react";
import { TxButton } from "./substrate-lib/components";

export default function Main(props) {
  const { erc20Addresses, availableAccounts } = props;
  const [status, setStatus] = useState(null);
  const [formState, setFormState] = useState({
    payAddress: "",
    owner: "",
    spender: "",
    to: "",
    amount: 1,
  });

  const onChange = (_, data) =>
    setFormState((prev) => ({ ...prev, [data.state]: data.value }));

  const { payAddress, owner, spender, to, amount } = formState;

  return (
    <Grid.Column>
      <h1>Erc20</h1>
      <Form>
        <Form.Group widths="equal">
          <Form.Field>
            <Dropdown
              placeholder="Select ERC20 address"
              fluid
              selection
              search
              options={erc20Addresses}
              state="payAddress"
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <Input
              fluid
              label="payAddress"
              type="text"
              placeholder="payAddress"
              value={payAddress}
              state="payAddress"
              onChange={onChange}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
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
        </Form.Group>
        <Form.Group widths="equal">
          <Form.Field>
            <Dropdown
              placeholder="spender"
              fluid
              selection
              search
              options={availableAccounts}
              state="spender"
              onChange={onChange}
            />
          </Form.Field>
          <Form.Field>
            <Input
              fluid
              label="spender"
              type="text"
              placeholder="spender"
              value={spender}
              state="spender"
              onChange={onChange}
            />
          </Form.Field>
        </Form.Group>
        <Form.Group widths="equal">
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
        </Form.Group>
        <Form.Field>
          <Input
            fluid
            label="amount"
            type="text"
            placeholder="amount"
            value={amount}
            state="amount"
            onChange={onChange}
          />
        </Form.Field>
        <Form.Group widths="equal">
          <Form.Field>
            <TxButton
              label="approve"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                contractAddress: payAddress,
                palletRpc: "erc20",
                callable: "approve",
                inputParams: [spender, amount],
                paramFields: [true, true],
              }}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="transfer"
              type="SIGNED-TXC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "erc20",
                callable: "transfer",
                contractAddress: payAddress,
                inputParams: [to, amount],
                paramFields: [true, true],
              }}
            />
          </Form.Field>
          <Form.Field>
            <TxButton
              label="balanceOf"
              type="QUERYC"
              setStatus={setStatus}
              attrs={{
                palletRpc: "erc20",
                callable: "balanceOf",
                contractAddress: payAddress,
                inputParams: [owner],
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
