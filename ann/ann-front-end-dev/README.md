<!-- vscode-markdown-toc -->
* 1. [Deploy  & Config Guide](#DeployConfigGuide)
	* 1.1. [Key Steps](#KeySteps)
	* 1.2. [Get source code](#Getsourcecode)
* 2. [Setup Contracts](#SetupContracts)
	* 2.1. [Get contracts](#Getcontracts)
* 3. [Deploy contracts](#Deploycontracts)
		* 3.1. [set the node IP and port ( `ws://127.0.0.1:9944` default).](#setthenodeIPandportws:127.0.0.1:9944default.)
		* 3.2. [Upload & Deploy contracts](#UploadDeploycontracts)
* 4. [Initialization & Deploy HexSpace](#InitializationDeployHexSpace)
* 5. [Install `Polkadot JS Extension`](#InstallPolkadotJSExtension)
	* 5.1. [Config front-end](#Configfront-end)
		* 5.1.1. [1. Click Hex Space contract icon,copy the contract address.](#ClickHexSpacecontracticoncopythecontractaddress.)
		* 5.1.2. [2. Replace contract address of key 'REACT_APP_CONTRACT_ADDRESS' in .env](#ReplacecontractaddressofkeyREACT_APP_CONTRACT_ADDRESSin.env)
		* 5.1.3. [3. Replace connect path](#Replaceconnectpath)
	* 5.2. [Install dependencies](#Installdependencies)
	* 5.3. [Start front-end](#Startfront-end)
* 6. [Start front-end  docker](#Startfront-enddocker)
* 7. [Start front-end  using docker-compose](#Startfront-endusingdocker-compose)
	* 7.1. [front-end  Test Guide](#front-endTestGuide)
* 8. [Acknowledgements](#Acknowledgements)
* 9. [Upstream](#Upstream)

##  1. <a name='DeployConfigGuide'></a>Deploy  & Config Guide
###  1.1. <a name='KeySteps'></a>Key Steps
- Manual Deploy erc1155 contract 
- Copy  erc1155 contract Code Hash as init parameter 'code_hash' of deploying hex space contract .
- Manual config  REACT_APP_PROVIDER_SOCKET=ws://127.0.0.1:9944  -e REACT_APP_CONTRACT_ADDRESS=5H4rkHhc6w1A95GDMDuFoTQ6MZcjxY4N5aHiUrSncXDrSasR  in the  .env file 
  Or Set docker environment -e REACT_APP_PROVIDER_SOCKET=ws://127.0.0.1:9944  -e REACT_APP_CONTRACT_ADDRESS=5H4rkHhc6w1A95GDMDuFoTQ6MZcjxY4N5aHiUrSncXDrSasR  
  Or update docker-compose.yml  environment:
      - REACT_APP_PROVIDER_SOCKET=ws://127.0.0.1:9944
      - REACT_APP_CONTRACT_ADDRESS=5H4rkHhc6w1A95GDMDuFoTQ6MZcjxY4N5aHiUrSncXDrSasR

###  1.2. <a name='Getsourcecode'></a>Get source code

Please get the code from `https://github.com/rust-0x0/hex-space-protocol-front-end/tree/milestone-1`

```
git clone -b milestone-1 https://github.com/rust-0x0/hex-space-protocol-front-end.git
```

##  2. <a name='SetupContracts'></a>Setup Contracts

HexSpaceSocialGraph Protocol Contracts builded version are provided in `builded_contracts`. 

It's developed with ink!.

###  2.1. <a name='Getcontracts'></a>Get contracts

```
hex-space-protocol-front-end/builded_contracts/
```

##  3. <a name='Deploycontracts'></a>Deploy contracts

The HexSpaceSocialGraph Protocol creates the substrate chain to connect the POLKADOT Ecology, and all contracts are deployed on the HexSpaceSocialGraph dev node. This section explains how to make use of Polkadot JS App to deploy contracts.

Use `https://polkadot.js.org/apps/` upload .contract file to deploy contract.

####  3.1. <a name='setthenodeIPandportws:127.0.0.1:9944default.'></a>set the node IP and port ( `ws://127.0.0.1:9944` default).

![](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/images/deploy1.png)

####  3.2. <a name='UploadDeploycontracts'></a>Upload & Deploy contracts

Enter `Developer-> Contracts` and click Upload & deploy code.

![image](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/images/deploy2.png)

Select the ERC1155 contract files that required to deploy contract.

![](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/images/deploy3.png)

After you upload the contracts,  click 'copy' icon copy erc1155 hash value.
 you can instantiate the contract on the chain. In substrate, you need to perform the contract’s initialization function, usually new or the default function.
Select the initialization function call, fill in the initialization parameters, set the main contract administrator, and set the contract initial balance, click `Deploy`. Click `Deploy `, and `Submit and Sign`

##  4. <a name='InitializationDeployHexSpace'></a>Initialization & Deploy HexSpace

![](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/images/deploy4.png)

# Setup HexSpaceSocialGraph Protocol Front-end

##  5. <a name='InstallPolkadotJSExtension'></a>Install `Polkadot JS Extension`

Please install `Polkadot JS Extension` before you start. You can get it from here https://polkadot.js.org/extension/

###  5.1. <a name='Configfront-end'></a>Config front-end

Please  rename `example.env `  the file name to  '.env', and update the correct  contract address in   ```.env ```. 
####  5.1.1. <a name='ClickHexSpacecontracticoncopythecontractaddress.'></a>1. Click Hex Space contract icon,copy the contract address. 
![](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/images/deploy5.png)
####  5.1.2. <a name='ReplacecontractaddressofkeyREACT_APP_CONTRACT_ADDRESSin.env'></a>2. Replace contract address of key 'REACT_APP_CONTRACT_ADDRESS' in .env 

![](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/images/deploy6.png)

####  5.1.3. <a name='Replaceconnectpath'></a>3. Replace connect path

And replace `.env REACT_APP_PROVIDER_SOCKET` to your connect path.

it should be `ws://127.0.0.1:9944` by default.

###  5.2. <a name='Installdependencies'></a>Install dependencies

Run `yarn ` to install packages needed for this App.

###  5.3. <a name='Startfront-end'></a>Start front-end

`yarn start` runs the app in the development mode.
Open http://localhost:8100 to view it in the browser.


  
##### Get gas from extension account

In `https://polkadot.js.org/apps` Account page, use account  send gas to your extension account.

##  6. <a name='Startfront-enddocker'></a>Start front-end  docker

```bash
docker pull rust0x0/hex-space-protocol-front-end:0.1

# update the correct  contract address to env REACT_APP_CONTRACT_ADDRESS
docker run -e REACT_APP_PROVIDER_SOCKET=ws://127.0.0.1:9944  -e REACT_APP_CONTRACT_ADDRESS=5H4rkHhc6w1A95GDMDuFoTQ6MZcjxY4N5aHiUrSncXDrSasR -p 8100:8100  rust0x0/hex-space-protocol-front-end:0.1

```

##  7. <a name='Startfront-endusingdocker-compose'></a>Start front-end  using docker-compose
update env REACT_APP_CONTRACT_ADDRESS  to the correct  contract address  in docker-compose.yml file 
```bash
 environment:
      - REACT_APP_PROVIDER_SOCKET=ws://127.0.0.1:9944
      - REACT_APP_CONTRACT_ADDRESS=5H4rkHhc6w1A95GDMDuFoTQ6MZcjxY4N5aHiUrSncXDrSasR
```
```bash
 cd hex-space-protocol-front-end
docker-compose  up -d
## view logs
docker-compose logs -ft
```
###  7.1. <a name='front-endTestGuide'></a>front-end  Test Guide

- [How to follow my Web3.0 friends Tutorial](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/Follow.md)
- [How to edit the Profile on HexSpaceFrontEnd  Tutorial](https://github.com/rust-0x0/hex-space-protocol-docs/blob/milestone-1/Profile.md)


##  8. <a name='Acknowledgements'></a>Acknowledgements

It is inspired by existing projects & standards:

- [5degrees](https://github.com/5DegreesProtocol/5degrees-protocol.git)


NOTE: This pallet implements the aforementioned process in a simplified way, thus it is intended for demonstration purposes and is not audited or ready for production use.

##  9. <a name='Upstream'></a>Upstream

This project was forked from
- [the Substrate Contracts node](https://github.com/paritytech/substrate-contracts-node.git).
- [the Substrate DevHub Front-end Template](https://github.com/substrate-developer-hub/substrate-front-end-template)
