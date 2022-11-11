import React, { createRef } from "react";
import {
  Container,
  Dimmer,
  Loader,
  Grid,
  Sticky,
  Message,
  Button,
} from "semantic-ui-react";
import "semantic-ui-css/semantic.min.css";

import {
  SubstrateContextProvider,
  useSubstrateState,
  useSubstrate,
} from "./substrate-lib";
import { DeveloperConsole } from "./substrate-lib/components";
import AccountSelector from "./AccountSelector";
import BlockNumber from "./BlockNumber";
import Metadata from "./Metadata";
import NodeInfo from "./NodeInfo";
import Events from "./Events";
import ContractManagement from "./ContractManagement";
import ContractMgmt from "./ContractMgmt";
import Auction from "./auction/Auction";
import BundleMarketplace from "./bundlemarketplace/BundleMarketplace";
import Marketplace from "./marketplace/Marketplace";
import ArtFactory from "./artfactory/ArtFactory";
import ArtFactoryPrivate from "./artfactoryprivate/ArtFactoryPrivate";
import NftFactory from "./nftfactory/NftFactory";
import NftFactoryPrivate from "./nftfactoryprivate/NftFactoryPrivate";
import AddressRegistry from "./AddressRegistry";
import PriceSeed from "./PriceSeed";
import TokenRegistry from "./TokenRegistry";

import ArtTradable from "./ArtTradable";
import ArtTradablePrivate from "./ArtTradablePrivate";
import NftTradable from "./NftTradable";
import NftTradablePrivate from "./NftTradablePrivate";
import Erc20 from "./Erc20";

function Main() {
  const { apiState, apiError, keyringState, keyring, tokens } =
    useSubstrateState();
  const { contract } = useSubstrateState();

  const { updateTokens } = useSubstrate();
  const txOnClickHandler = (factory, tradable, i) => {
    const txOnClickHandleri = async (factory, tradable, i) => {
      const { output, result } = await contract[factory].query["tokens"](
        contract[factory].address,
        { gasLimit: -1, value: 0 }
      );
      if (result.isOk && output !== undefined) {
        console.log("==txOnClickHandleri=tokens====", output.toString());
        updateTokens({
          tradable: output
            .toString()
            .trim()
            .substring(1, output.toString().trim().length - 1)
            .split(",")
            .filter((x) => x.length > 0),
        });
      }

      console.log(i, result.toHuman(), output);
    };
    txOnClickHandleri(factory, tradable, i);
  };
  const syncAllTokens = async () => {
    const syncTokens = async (factory ) => {
      if (contract[factory] === undefined) {
        console.warn(factory, "==factory=is null====", factory);
        return [];
      }
      const { output, result } = await contract[factory].query["tokens"](
        contract[factory].address,
        { gasLimit: -1, value: 0 }
      );
      if (result.isOk && output !== undefined) {
        console.log("==syncAllTokens=tokens====", output.toString());
        return output
          .toString()
          .trim()
          .substring(1, output.toString().trim().length - 1)
          .split(",")
          .filter((x) => x.length > 0);
      }

      console.log( result.toHuman(), output);
      return [];
    };
    let f = [
      "artFactory",
      "artFactoryPrivate",
      "nftFactory",
      "nftFactoryPrivate",
    ];
    let t = [
      "artTradable",
      "artTradablePrivate",
      "nftTradable",
      "nftTradablePrivate",
    ];
    let tt = {};
    for (let i = 0; i < f.length; i++) {
      let a = await syncTokens(f[i]);
      if (a.length > 0) {
        tt[t[i]] = a;
      }
    }
    updateTokens(tt);
  };
  let addresses = [];
  console.log(tokens, "====tokens=====");
  if (tokens != null) {
    addresses = Object.keys(tokens).reduce((a, t) => {
      return a.concat(
        tokens[t].map((b) => {
          return {
            key: b,
            text: t + (t.indexOf("nft") !== -1 ? "(ERC721)" : "(ERC1155)") + b,
            value: b,
          };
        })
      );
    }, []);
    console.log(addresses, "====addresses=====");
  }
 let erc20Addresses = [];
 let erc20tokens = [];
  console.log(erc20tokens, "====erc20tokenstokens=====");
  if (erc20tokens != null) {
    erc20Addresses = erc20tokens.map((b) => {
          return { key: b,
            text: b,
            value: b}
        });
    console.log(erc20Addresses, "====erc20Addresses=====");
  }
  const accounts = keyring == null ? [] : keyring.getPairs();

  const availableAccounts = [];
  accounts.map((account) => {
    return availableAccounts.push({
      key: account.meta.name,
      text: account.meta.name,
      value: account.address,
    });
  });
  const contextRef = createRef();
  //   const loaderRef = createRef()
  // const loaderRef = React.useRef(null);
  const CustomLoader = React.forwardRef((props, ref) => {
    return (
      <Loader {...props} ref={ref}>
        {props.children}
      </Loader>
    );
  });
  const CustomDimmer = React.forwardRef((props, ref) => {
    return (
      <Dimmer {...props} ref={ref}>
        {props.children}
      </Dimmer>
    );
  });

  const loader = (text) => (
    <CustomDimmer active>
      <CustomLoader size="small">{text}</CustomLoader>
    </CustomDimmer>
  );

  const message = (errObj) => (
    <Grid centered columns={2} padded>
      <Grid.Column>
        <Message
          negative
          compact
          floating
          header="Error Connecting to Substrate"
          content={`Connection to websocket '${errObj.target.url}' failed.`}
        />
      </Grid.Column>
    </Grid>
  );

  if (apiState === "ERROR") return message(apiError);
  else if (apiState !== "READY") return loader("Connecting to Substrate");

  if (keyringState !== "READY") {
    return loader(
      "Loading accounts (please review any extension's authorization)"
    );
  }

  return (
    <div ref={contextRef}>
      <Sticky context={contextRef}>
        <AccountSelector />
      </Sticky>
      <Container>
        <Grid stackable columns="equal">
          <Grid.Row stretched>
            <NodeInfo />
            <Metadata />
            <BlockNumber />
            <BlockNumber finalized />
          </Grid.Row>
          <Grid.Row>
            <ContractMgmt />
          </Grid.Row>
          <Grid.Row>
            <ContractManagement />
          </Grid.Row>
        </Grid>
        <Auction {...{ addresses, availableAccounts }} />
        <Marketplace {...{ addresses, availableAccounts }} />
        <BundleMarketplace {...{ addresses, availableAccounts }} />
        <ArtFactory {...{ addresses, availableAccounts, txOnClickHandler }} />
        <ArtFactoryPrivate
          {...{ addresses, availableAccounts, txOnClickHandler }}
        />
        <NftFactory {...{ addresses, availableAccounts, txOnClickHandler }} />
        <NftFactoryPrivate
          {...{ addresses, availableAccounts, txOnClickHandler }}
        />

        <Grid>
          <Grid.Row>
            <Grid.Column>
              <Button basic size="massive" onClick={(_) => syncAllTokens()}>
                syncAllTokens
              </Button>
            </Grid.Column>
          </Grid.Row>
          <Grid.Row columns="equal">
            <PriceSeed />
            <TokenRegistry />
          </Grid.Row>
          <Grid.Row columns="equal">
            <AddressRegistry />
          </Grid.Row>
          <Grid.Row columns="equal">
            <ArtTradable {...{ addresses, availableAccounts }} />
            <ArtTradablePrivate {...{ addresses, availableAccounts }} />
          </Grid.Row>
          <Grid.Row columns="equal">
            <NftTradable {...{ addresses, availableAccounts }} />
            <NftTradablePrivate {...{ addresses, availableAccounts }} />
          </Grid.Row>
        <Grid.Row columns="equal">
            <Erc20 {...{ erc20Addresses, availableAccounts }} />
          </Grid.Row>
          <Grid.Row>
            <Events />
          </Grid.Row>
        </Grid>
      </Container>
      <DeveloperConsole />
    </div>
  );
}

export default function App() {
  return (
    <SubstrateContextProvider>
      <Main />
    </SubstrateContextProvider>
  );
}
