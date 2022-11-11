## Install Rust and the Rust toolchain

#####  1.Install `rustup` by running the following command: 

` curl https://sh.rustup.rs -sSf | sh `

##### 2.Configure your current shell to reload your PATH environment variable so that it includes the Cargo `bin` directory by running the following command: 

` source ~/.cargo/env `

##### 3.Configure the Rust toolchain to default to the latest `stable` version by running the following commands: 

`rustup default stable`

`rustup update`

##### 4. Add the `nightly` release and the `nightly` WebAssembly (`wasm`) targets by running the following commands: 

`rustup update nightly`

`rustup target add wasm32-unknown-unknown --toolchain nightly`

##### 5. Verify your installation by running the following commands: 

`rustc --version`
`rustup show`


### 2. ink! CLI

```
# For Ubuntu or Debian users
sudo apt install 
# For MacOS users
brew install binaryen
```

#### cargo-contract

`cargo install cargo-contract --vers 0.15.0 --force --locked`


### Get contracts
HexSpaceSocialGraph Protocol Contracts are provided in `https://github.com/rust-0x0/hex-space-protocol-substrate/tree/milestone-1`. 

It's developed with ink!.
```
git clone -b milestone-1 https://github.com/rust-0x0/hex-space-protocol-substrate.git
```

## Compile contracts from source code

The HexSpaceSocialGraph-Protocol provides script to simplify the contract compilation process while collecting the editing results into a unified directory to facilitate contract deployment and usage. Execute in the project root directory

```bash
cd hex-space-protocol-substrate
bash ./build.sh
```

All contract compilation results are saved in the release directory.

## Testing contracts
Run the tests with:
```bash
cd  hex-space-protocol-substrate/hex-space
cargo test
```


## Acknowledgements

It is inspired by existing projects & standards:

- [5degrees](https://github.com/5DegreesProtocol/5degrees-protocol.git)


NOTE: This pallet implements the aforementioned process in a simplified way, thus it is intended for demonstration purposes and is not audited or ready for production use.

## Upstream

This project was forked from
- [the Ink! Contract Example ERC115](https://github.com/paritytech/ink/tree/master/examples/erc1155)
- [the Substrate Contracts node](https://github.com/paritytech/substrate-contracts-node.git).
- [the Substrate DevHub Front-end Template](https://github.com/substrate-developer-hub/substrate-front-end-template)
