import React, { Component } from 'react';
import './App.css';
import ScatterJS from 'scatter-js/dist/scatter.esm';
import Eos from 'eosjs';
import Button from '@material-ui/core/Button'

class App extends Component {
    //   network =  {
    //     blockchain:'eos',
    //     protocol:'https',
    //     host:'mainnet.meet.one',
    //     port:443,
    //     chainId:'aca376f206b8fc25a6ed44dbdc66547c36c6c33e3a119ffbeaef943642f0e906'
    //   };
    network = {
        blockchain: 'eos',
        protocol: 'http',
        host: '10.11.5.37',
        port: 8000,
        chainId: '1c6ae7719a2a3b4ecb19584a30ff510ba1b6ded86e1fd8b8fc22f1179c622a32'
    };
    currentAccount = null;
    connected = false;
    swapContract = 'eoswapeoswap';
    poolName = "pool";
    poolName1 = "pool1";
    admin = 'eoswapeoswap';
    nonadmin = 'alic1111111';
    user1 = 'bob111111111';

    require_permissions = ({ account, key, actor, parent }) => {
        return {
            account: `${account}`,
            permission: "active",
            parent: `${parent}`,
            auth: {
                threshold: 1,
                keys: [
                    {
                        key: `${key}`,
                        weight: 1
                    }
                ],
                accounts: [
                    {
                        permission: {
                            actor: `${actor}`,
                            permission: "eosio.code"
                        },
                        weight: 1
                    }
                ],
                waits: []
            }
        };
    };

    allowContract = (auth, key, contract, parent) => {
        let [account, permission] = auth.split("@");
        permission = permission || "active";
        parent = parent || "owner";

        const tx_data = {
            actions: [
                {
                    account: "eosio",
                    name: "updateauth",
                    authorization: [
                        {
                            actor: account,
                            permission: permission
                        }
                    ],
                    data: this.require_permissions({
                        account: account,
                        key: key,
                        actor: contract,
                        parent: parent
                    })
                }
            ]
        };
        console.log(tx_data);
        return tx_data;
    };

    to_wei(value) {
        return value * Math.pow(10, 6);
    }

    to_max_supply(sym) {
        return "100000000000.0000 " + sym + "@eoswapxtoken";
    }

    to_asset(value, sym) {
        return value + ".0000 " + sym + "@eoswapxtoken";
    }

    to_wei_asset(value, sym) {
        return this.to_asset(value + "00", sym);
    }


    async connect() {
        //change name 'hello-scatter' to your application's name
        this.connected = await ScatterJS.scatter.connect('hello-scatter')
        console.log(this.connected);
    }

    // login with eos account via scatter
    async login() {
        if (!this.connected) {
            console.log('not connected');
            return;
        }
        try {
            let result = await ScatterJS.scatter.getIdentity({ accounts: [this.network] })
            this.currentAccount = result.accounts[0];
            console.log("login success,", this.currentAccount)
            alert("login success" + JSON.stringify(this.currentAccount))
        } catch (e) {
            alert("login fail")
            console.log("login fail,", e)
        }
    }

    async transfer() {
        if (this.currentAccount == null) {
            await this.handleLogin()
        }
        let eos = ScatterJS.scatter.eos(this.network, Eos);
        try {
            let result = await eos.transfer(this.currentAccount.name, 'bob111111111', '0.0001 SYS', 'hello-eos-scatter, dapp demo transfer');
            console.log(result)
        } catch (e) {
            console.log("error", e)
        }
    }

    async allowSwapContract() {
        if (this.currentAccount == null) {
            await this.handleLogin();
        }
        let eos = ScatterJS.scatter.eos(this.network, Eos);
        try {
            let tr = await eos.transaction(
                this.allowContract(this.currentAccount.name, this.currentAccount.publicKey, this.swapContract)
            );
            console.log(tr);
        } catch (e) {
            console.log("error", e)
        }
    }

    async handleActionSender(data) {
        if (data.hasOwnProperty("msg_sender")) {
            data.msg_sender = this.currentAccount.name;
        }
        else if (data.hasOwnProperty("from")) {
            data.from = this.currentAccount.name;
        }

        return data;
    }


    async swapContractPushAction(actionName, para_data) {
        if (this.currentAccount == null) {
            await this.handleLogin()
        }
        let eos = ScatterJS.scatter.eos(this.network, Eos);
        try {
            let paras_data = [];
            if (Array.isArray(para_data)) {
                paras_data = para_data;
            }
            else {
                paras_data.push(para_data);
            }

            let actions_data = [];
            for (let data of paras_data) {
                data = await this.handleActionSender(data);
                actions_data.push({
                    account: this.swapContract,
                    name: actionName,
                    authorization: [{
                        actor: this.currentAccount.name,
                        permission: this.currentAccount.authority
                    }],
                    data,
                });
            }
            console.log(actions_data);
            let tr = await eos.transaction(
                {
                    actions: actions_data
                }
            )
            console.log(tr);
        } catch (e) {
            console.log("error", e)
        }
    }

    async handleMintByNonadmin() {
        if (this.currentAccount == null) {
            await this.handleLogin();
        }

        let data = [{
            msg_sender: this.currentAccount.name,
            amt: this.to_wei_asset(1, "WETH")
        }, {
            msg_sender: this.currentAccount.name,
            amt: this.to_wei_asset(200, "DAI")
        }];

        await this.swapContractPushAction("mint", data);

    }

    async handleMintByUser1() {
        if (this.currentAccount == null) {
            await this.handleLogin();
        }

        let data = [{
            msg_sender: this.currentAccount.name,
            amt: this.to_wei_asset(25, "WETH")
        }, {
            msg_sender: this.currentAccount.name,
            amt: this.to_wei_asset(4, "MKR")
        }, {
            msg_sender: this.currentAccount.name,
            amt: this.to_wei_asset(40000, "DAI")
        }, {
            msg_sender: this.currentAccount.name,
            amt: this.to_wei_asset(10, "XXX")
        }];

        await this.swapContractPushAction("mint", data);

    }

    actions_paras_data = {
        "newtoken": [{
            msg_sender: "",
            token: this.to_max_supply("WETH")
        }, {
            msg_sender: "",
            token: this.to_max_supply("DAI")
        }],
        "mint": [{
            msg_sender: "",
            amt: this.to_wei_asset(5, "WETH")
        }, {
            msg_sender: "",
            amt: this.to_wei_asset(200, "DAI")
        }],
        "newpool": {
            msg_sender: "",
            pool_name: this.poolName
        },
        "setswapfee": {
            msg_sender: "",
            pool_name: this.poolName,
            swapFee: 3000
        },
        "bind": [{
            msg_sender: "",
            pool_name: this.poolName,
            balance: this.to_wei_asset(5, "WETH"),
            denorm: this.to_wei(5)
        }, {
            msg_sender: "",
            pool_name: this.poolName,
            balance: this.to_wei_asset(200, "DAI"),
            denorm: this.to_wei(5)
        }],
        "finalize": {
            msg_sender: "",
            pool_name: this.poolName
        },
        "joinpool": {
            msg_sender: "",
            pool_name: this.poolName,
            poolAmountOut: this.to_wei(10),
            maxAmountsIn: [Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER]
        },
        "exitpool": {
            msg_sender: "",
            pool_name: this.poolName,
            poolAmountIn: this.to_wei(10),
            minAmountsOut: [0, 0]
        },
        "collect": {
            msg_sender: "",
            pool_name: this.poolName
        },
        "extransfer": {
            from: "",
            to: this.admin,
            quantity: "1.0000 SYS@eosio.token",
            memo: ""
        }

    };

    swapinout_actions_paras_data = {
        "newtoken": [{
            msg_sender: "",
            token: this.to_max_supply("MKR")
        }, {
            msg_sender: "",
            token: this.to_max_supply("XXX")
        }],
        "mint": [{
            msg_sender: "",
            amt: this.to_wei_asset(50, "WETH")
        }, {
            msg_sender: "",
            amt: this.to_wei_asset(200, "MKR")
        }, {
            msg_sender: "",
            amt: this.to_wei_asset(10000, "DAI")
        }, {
            msg_sender: "",
            amt: this.to_wei_asset(10, "XXX")
        }],
        "newpool": {
            msg_sender: "",
            pool_name: this.poolName1
        },
        "setswapfee": {
            msg_sender: "",
            pool_name: this.poolName1,
            swapFee: 3000
        },
        "bind": [{
            msg_sender: "",
            pool_name: this.poolName1,
            balance: this.to_wei_asset(50, "WETH"),
            denorm: this.to_wei(5)
        }, {
            msg_sender: "",
            pool_name: this.poolName1,
            balance: this.to_wei_asset(20, "MKR"),
            denorm: this.to_wei(5)
        }, {
            msg_sender: "",
            pool_name: this.poolName1,
            balance: this.to_wei_asset(10000, "DAI"),
            denorm: this.to_wei(5)
        }],
        "finalize": {
            msg_sender: "",
            pool_name: this.poolName1
        },
        "joinpool": {
            msg_sender: "",
            pool_name: this.poolName1,
            poolAmountOut: this.to_wei(10),
            maxAmountsIn: [Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER]
        },
        "swapamtin": {
            msg_sender: "",
            pool_name: this.poolName1,
            tokenAmountIn: this.to_asset(250, "WETH"),
            minAmountOut: this.to_wei_asset(475, "DAI"),
            maxPrice: this.to_wei(200)
        },
        "swapamtout": {
            msg_sender: "",
            pool_name: this.poolName1,
            maxAmountIn: this.to_wei_asset(3, "WETH"),
            tokenAmountOut: this.to_wei_asset(1, "MKR"),
            maxPrice: this.to_wei(500)
        }

    };


    async handleAction(actionName, currPoolName) {
        if (actionName == "mint" && this.currentAccount.name != this.admin) {
            if (currPoolName == this.poolName) {
                await this.handleMintByNonadmin();
            }
            else {
                await this.handleMintByUser1();
            }

            return;
        }

        if (this.currentAccount == null) {
            await this.handleLogin();
        }

        let data = null;
        if (currPoolName == this.poolName) {
            data = this.actions_paras_data[actionName];
        }
        else { data = this.swapinout_actions_paras_data[actionName]; }

        if (undefined != data && null != data) {
            await this.swapContractPushAction(actionName, data);
        }
        else {
            console.log("unknown action");
        }
    }


    actions = Object.keys(this.actions_paras_data);
    swap_actions = Object.keys(this.swapinout_actions_paras_data);


    async logout() {
        await ScatterJS.scatter.forgetIdentity();
        this.currentAccount = null;
        this.connected = false;
    }

    async handleLogin() {
        await this.connect()
        await this.login()
    }

    render() {
        document.title = "hello-eos-scatter";
        return (
            <div className="App">
                <div className="BtnDiv">
                    <div className="Btn">
                        <Button variant="contained" color="primary" onClick={this.handleLogin.bind(this)}>login</Button>
                    </div>
                    <div className="Btn">
                        <Button variant="contained" color="primary" onClick={this.logout.bind(this)}>logout</Button>
                    </div>
                    <div className="Btn">
                        <Button variant="contained" color="primary" onClick={this.transfer.bind(this)}>transfer</Button>
                    </div>
                    <div className="Btn">
                        <Button variant="contained" color="primary" onClick={this.allowSwapContract.bind(this)}>allowSwapContract</Button>
                    </div>
                    {this.actions.map(action => (
                        <div className="Btn">
                            <Button variant="contained" color="primary" key={this.poolName + action} onClick={this.handleAction.bind(this, action, this.poolName)}>{action}</Button>
                        </div>
                    ))}
                    {this.swap_actions.map(action => (
                        <div className="Btn">
                            <Button variant="contained" color="primary" key={this.poolName1 + action} onClick={this.handleAction.bind(this, action, this.poolName1)}>{action}</Button>
                        </div>
                    ))}
                </div>
            </div>
        );
    }
}

export default App;
