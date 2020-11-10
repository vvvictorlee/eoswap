const Eos = require('eosjs');
const dotenv = require('dotenv');
const dotenvParseVariables = require('dotenv-parse-variables');


let env = dotenv.config({})
if (env.error) throw env.error;
env = dotenvParseVariables(env.parsed);

//const axios = require('axios');
const request = require('request');
let sleep = require('sleep');
// var request = require('request'); // https://www.npmjs.com/package/request
let async = require('async'); // https://www.npmjs.com/package/async
// const { logTime } = require("./log_aop");
require("../lib/log_aop");
const jq = require('node-jq');
const EOS_RPC = require('../lib/eos_rpc')
const eosrpc = EOS_RPC();

const prettyJson = async (log) => {
    let jsonstr = await jq.run('.', JSON.stringify(log), { input: 'string', output: 'pretty' });
    console.log(jsonstr);
};

dotenv.load();
// # http://10.100.1.10:8889/v1/wallet/list_wallets



const interval = process.env.FREQ;
const owner = process.env.ADMIN;
const dosContract = process.env.DOS_CONTRACT;


// "5JZDFmwRwxJU2j1fugGtLLaNp2bcAP2PKy5zsqNkhn47v3S3e5w",
// "5JxT1aA8MiZZe7XjN3SYaQ65NSbZXrBcjePaSwRifK7jJLdjSf3",
// "5JHFTcGiKFDXFR64voMJXnxWZUqBgaEAnqMiyjJzBLQn9tHhWA8",
// "5HwYSQMW2Xy37Q9nhdKz7T32eLxwbDq29rMzGXrRQJwveh9B7sG",
// "5J6BA1U4QdQPwkFWsphU96oBusvsA8V2UJDtMtKgNneakBK9YrN",


// # ROXE6m2TpGWE59yDPWuBaB3xSJSgYWkggzSTuDv5vLfS3hYzB6UTU2=KEY:5JZDFmwRwxJU2j1fugGtLLaNp2bcAP2PKy5zsqNkhn47v3S3e5w


// # Private key: 5JxT1aA8MiZZe7XjN3SYaQ65NSbZXrBcjePaSwRifK7jJLdjSf3
// # Public key: ROXE5rM2nqtmCqyeRMpmQQMVTMYYZ9VYq9JDgve4t3Gzy6gVU1wB1z


// # Private key: 5JHFTcGiKFDXFR64voMJXnxWZUqBgaEAnqMiyjJzBLQn9tHhWA8
// # Public key: ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH

// # Private key: 5HwYSQMW2Xy37Q9nhdKz7T32eLxwbDq29rMzGXrRQJwveh9B7sG
// # Public key: ROXE8Av6ToXNYrGNdiQtpdUAG8LBDoMM3RZnin5NYpHk4WdKwiYk2W

// # Private key: 5J6BA1U4QdQPwkFWsphU96oBusvsA8V2UJDtMtKgNneakBK9YrN
// # Public key: ROXE6bYcFRBBLugKtxfkNxnyyrxUFV2LMGT3h9GcDisd6QYUyt2xfX

//         admin              = N(eosdoseosdos);
//        doowner            = N(dodoowner111);
//        tokenissuer        = N(tokenissuer1);
//        maintainer         = N(maintainer11);
//        oracleadmin        = N(eosdosoracle);
//        trader                 = N(alice);
//        trader             = N(bob);
//        dodo_ethbase_name  = N(ethbasemkr11);
//        dodo_ethquote_name = N(ethquotemkr1);

//cosnt      doowner            = "dodoowner111";
const bp = "roxe1";
const lp = "alice1111111";
const trader = "bob111111111";
const hexuser = "carol1111111";
const admin = "eosdoseosdos";
const tokenowner = "eosdosxtoken";
const tokenissuer = "tokenissuer1";
const maintainer = "maintainer11";
const oracleadmin = "eosdosoracle";
const doowner = admin;
const dodo_ethbase_name = "ethbasemkr11";
const dodo_ethquote_name = "ethquotemkr1";
const dodo_stablecoin_name = "daimkrdaimkr";
const admin_pub = "ROXE6m2TpGWE59yDPWuBaB3xSJSgYWkggzSTuDv5vLfS3hYzB6UTU2";
const tokenowner_pub = "ROXE5rM2nqtmCqyeRMpmQQMVTMYYZ9VYq9JDgve4t3Gzy6gVU1wB1z";
const pub = "ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH";
const trader_pub = "ROXE6bYcFRBBLugKtxfkNxnyyrxUFV2LMGT3h9GcDisd6QYUyt2xfX";

acc2pub_keys = {
    "roxe1": "ROXE6m2TpGWE59yDPWuBaB3xSJSgYWkggzSTuDv5vLfS3hYzB6UTU2",
    "eosdoseosdos": "ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH",
    "eosdosxtoken": "ROXE5rM2nqtmCqyeRMpmQQMVTMYYZ9VYq9JDgve4t3Gzy6gVU1wB1z",
    "eosdosoracle": "ROXE5rM2nqtmCqyeRMpmQQMVTMYYZ9VYq9JDgve4t3Gzy6gVU1wB1z",
    "ethbasemkr11": "ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH",
    "ethquotemkr1": "ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH",
    "daimkrdaimkr": "ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH",
    "tokenissuer1": "ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH",
    "maintainer11": "ROXE6m2TpGWE59yDPWuBaB3xSJSgYWkggzSTuDv5vLfS3hYzB6UTU2",
    "alice1111111": "ROXE6ftHab5c81LAcL1izHNyFVawBaZTEpFDXN3BYybx1pcJHQsTmH",
    "bob111111111": "ROXE6bYcFRBBLugKtxfkNxnyyrxUFV2LMGT3h9GcDisd6QYUyt2xfX",
    "carol1111111": "ROXE6bYcFRBBLugKtxfkNxnyyrxUFV2LMGT3h9GcDisd6QYUyt2xfX"
};
const dodo = "dodo";
const dodo1 = "dodo3";
const admin_pub = "EOS69tWc1VS6aP2P1D8ryzTiakPAYbV3whbHeWUzfD8QWYuHKqQxk";
const tokenowner_pub = "EOS69tWc1VS6aP2P1D8ryzTiakPAYbV3whbHeWUzfD8QWYuHKqQxk";
const pub = "EOS69X3383RzBZj41k73CSjUNXM5MYGpnDxyPnWUKPEtYQmTBWz4D";
const user1_pub = "EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S";



const require_permissions = ({ account, key, actor, parent }) => {
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

const allowContract = (auth, key, contract, parent) => {
    let [account, permission] = auth.split("@");
    permission = permission || "active";
    parent = parent || "owner";
    let pub_keys = [key];
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
                data: require_permissions({
                    account: account,
                    key: key,
                    actor: contract,
                    parent: parent
                })
            }
        ], pub_keys
    };

    return tx_data;
};

// const pub = "EOS89PeKPVQG3f48KCX2NEg6HDW7YcoSracQMRpy46da74yi3fTLP";
// eos.transaction(allowContract(nonadmin, pub, nonadmin));
//   await oraclizeContract.setup(oraclizeAccount, oracle, masterAccount, {
// 	authorization: [oraclizeAccount]
//   });



const pushAction = (account, key, action, data) => {
    // let [account, permission] = auth.split("@");
    let permission = "active";
    pub_keys = [key];
    const tx_data = {
        actions: [
            {
                account: dosContract,
                name: action,
                authorization: [
                    {
                        actor: account,
                        permission: permission
                    }
                ],
                data: data
            }
        ],
        pub_keys: pub_keys
    };

    return tx_data;
};

const pushTransaction = async (account, action, data) => {
    const results = await eosrpc.transaction(pushAction(account, acc2pub_keys[account], action, data));
    return results;
}

function find_from_array(arr) {
    let newArr = arr.filter(function (p) {
        return p.name === "United States";
    });

    return newArr;
}

function repeat(str, n) {
    return new Array(n + 1).join(str);
}

function current_time() {
    return Date.parse(new Date()) / 1000;
}

function to_timestamp(time) {
    return Date.parse(new Date(time)) / 1000;
}

function to_wei(value) {
    return value * Math.pow(10, 6);
}

function to_max_supply(sym) {
    return { quantity: "100000000000.0000 " + sym, contract: "eosdosxtoken" };
}

function get_core_symbol() {
    return { symbol: "4,ROC", contract: 'roxe.token' };
}

function to_sym(sym) {
    return { symbol: "4," + sym, contract: 'eosdosxtoken' };
}
const decimals = Math.pow(10, 4);
function tounit(value) {
    return todecimal(scalar_decimals(value));
}

function todecimal(value) {
    return Number(Number(value) / Number(decimals)).toFixed(4) + " ";
}

function scalar_decimals(value) {
    return Number(value) * Number(decimals);
}

function to_core_asset(value, sym) {
    return { quantity: tounit(value) + sym, contract: "roxe.token" };
}

function to_asset(value, sym) {
    return { quantity: todecimal(value) + sym, contract: "eosdosxtoken" };
}

function to_wei_asset(value, sym) {
    return to_asset(scalar_decimals(value), sym);
}

class EosClient {
    constructor(dodo_name) {
        this.dodoName = dodo_name;
    }
    async allowDosContract(user, pubk) {
        await eosrpc.transaction(allowContract(user, pubk, dosContract));
    }

    async allowDosContracts() {
        const accounts = Object.keys(acc2pub_keys);
        for (let acc of accounts) {
            this.allowDosContract(acc, acc2pub_keys[acc]);
        }
    }

    async create_wallet() {
        const results = await eosrpc.create_default_wallet();

        console.log(__line); prettyJson(results);
    }

    async import_keys() {
        const keys = [process.env.EOS_KEY,
            "5JZDFmwRwxJU2j1fugGtLLaNp2bcAP2PKy5zsqNkhn47v3S3e5w",
            "5JxT1aA8MiZZe7XjN3SYaQ65NSbZXrBcjePaSwRifK7jJLdjSf3",
            "5JHFTcGiKFDXFR64voMJXnxWZUqBgaEAnqMiyjJzBLQn9tHhWA8",
            "5HwYSQMW2Xy37Q9nhdKz7T32eLxwbDq29rMzGXrRQJwveh9B7sG",
            "5J6BA1U4QdQPwkFWsphU96oBusvsA8V2UJDtMtKgNneakBK9YrN"];
        const results = await eosrpc.import_keys(keys);

        console.log(__line); prettyJson(results);
    }

    async init(msg_sender, dodoZoo, weth, core_symbol) {
        const results = await pushTransaction(msg_sender, "init", {
            msg_sender: msg_sender,
            dodoZoo: dodoZoo,
            weth: weth,
            core_symbol: core_symbol
        });
        console.log(__line); prettyJson(results);
    }

    async newtoken(msg_sender, token) {
        const results = await pushTransaction(msg_sender, "newtoken", {
            msg_sender: msg_sender,
            token: token
        });
        console.log(__line); prettyJson(results);
    }

    async mint(to, amount) {

        const results = await pushTransaction(to, "mint", {
            msg_sender: to,
            amt: amount
        });

        console.log(__line); prettyJson(results);
    }

    async neworacle(msg_sender, token) {

        const results = await pushTransaction(msg_sender, "neworacle", {
            msg_sender: msg_sender,
            token: token
        });

        console.log(__line); prettyJson(results);
    }

    async setprice(msg_sender, amt) {
        const results = await pushTransaction(msg_sender, "setprice", {
            msg_sender: msg_sender,
            amt: amt
        });
        console.log(__line); prettyJson(results);
    }

    async breeddodo(msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit) {
        const results = await pushTransaction(msg_sender, "breeddodo", {
            msg_sender: msg_sender, dodo_name: dodo_name, maintainer: maintainer, baseToken: baseToken, quoteToken: quoteToken, oracle: oracle, lpFeeRate: lpFeeRate, mtFeeRate: mtFeeRate, k: k, gasPriceLimit, gasPriceLimit
        });
        console.log(__line); prettyJson(results);
    }

    async enablex(msg_sender, dodo_name, action_name) {
        const results = await pushTransaction(msg_sender, action_name, {
            msg_sender: msg_sender,
            dodo_name: dodo_name
        });
        console.log(__line); prettyJson(results);
    }

    async depositquote(msg_sender, dodo_name, amt) {
        const results = await pushTransaction(msg_sender, "depositquote", {
            msg_sender: msg_sender,
            dodo_name: dodo_name,
            amt: amt
        });
        console.log(__line); prettyJson(results);
    }

    async depositbase(msg_sender, dodo_name, amt) {
        const results = await pushTransaction(msg_sender, "depositbase", {
            msg_sender: msg_sender,
            dodo_name: dodo_name,
            amt: amt
        });
        console.log(__line); prettyJson(results);
    }

    async withdrawquote(msg_sender, dodo_name, amt) {
        const results = await pushTransaction(msg_sender, "withdrawquote", {
            msg_sender: msg_sender,
            dodo_name: dodo_name,
            amt: amt
        });
        console.log(__line); prettyJson(results);
    }

    async withdrawbase(msg_sender, dodo_name, amt) {
        const results = await pushTransaction(msg_sender, "withdrawbase", {
            msg_sender: msg_sender,
            dodo_name: dodo_name,
            amt: amt
        });
        console.log(__line); prettyJson(results);
    }

    async withdrawallq(msg_sender, dodo_name) {
        const results = await pushTransaction(msg_sender, "withdrawallq", {
            msg_sender: msg_sender,
            dodo_name: dodo_name
        });
        console.log(__line); prettyJson(results);
    }

    async withdrawallb(msg_sender, dodo_name, amt) {
        const results = await pushTransaction(msg_sender, "withdrawallb", {
            msg_sender: msg_sender,
            dodo_name: dodo_name
        });
        console.log(__line); prettyJson(results);
    }

    async sellethtoken(ethToken, minReceiveToken) {
        const results = await pushTransaction(msg_sender, "sellethtoken", {
            msg_sender: msg_sender,
            ethToken: ethToken,
            minReceiveToken: minReceiveToken
        });
        console.log(__line); prettyJson(results);
    }

    async buyethtoken(ethToken, maxPayTokenAmount) {
        const results = await pushTransaction(msg_sender, "buyethtoken", {
            msg_sender: msg_sender,
            ethToken: ethToken,
            maxPayTokenAmount: maxPayTokenAmount
        });
        console.log(__line); prettyJson(results);
    }

    async selltokeneth(baseToken, minReceiveEth) {
        const results = await pushTransaction(msg_sender, "selltokeneth", {
            msg_sender: msg_sender,
            baseToken: baseToken,
            minReceiveEth: minReceiveEth
        });
        console.log(__line); prettyJson(results);
    }

    async buytokeneth(msg_sender, baseToken, maxPayEthAmount) {
        const results = await pushTransaction(msg_sender, "buytokeneth", {
            msg_sender: msg_sender,
            baseToken: baseToken,
            maxPayEthAmount: maxPayEthAmount
        });
        console.log(__line); prettyJson(results);
    }

    async sellbastoken(msg_sender, dodo_name, amount, minReceiveQuote) {
        const results = await pushTransaction(msg_sender, "sellbastoken", {
            msg_sender: msg_sender,
            dodo_name: dodo_name,
            amount: amount,
            minReceiveQuote: minReceiveQuote
        });
        console.log(__line); prettyJson(results);
    }

    async buybasetoken(msg_sender, dodo_name, amount, maxPayQuote) {
        const results = await pushTransaction(msg_sender, "buybasetoken", {
            msg_sender: msg_sender,
            dodo_name: dodo_name,
            amount: amount,
            maxPayQuote: maxPayQuote
        });
        console.log(__line); prettyJson(results);
    }

    async depositethab(msg_sender, ethtokenamount, quoteToken) {
        //   depositethab(lp, to_wei_asset(10,"WETH"), to_sym("MKR"));
        const results = await pushTransaction(msg_sender, "depositethab", {
            msg_sender: msg_sender,
            ethtokenamount: ethtokenamount,
            quoteToken: quoteToken
        });
        console.log(__line); prettyJson(results);
    }

    async withdraweab(msg_sender, ethtokenamount, quoteToken) {
        const results = await pushTransaction(msg_sender, "withdraweab", {
            msg_sender: msg_sender,
            ethtokenamount: ethtokenamount,
            quoteToken: quoteToken
        });
        console.log(__line); prettyJson(results);
    }

    async withdrawaeab(msg_sender, quoteToken) {
        const results = await pushTransaction(msg_sender, "withdrawaeab", {
            msg_sender: msg_sender,
            quoteToken: quoteToken
        });
        console.log(__line); prettyJson(results);
    }

    async depositethaq(msg_sender, ethtokenamount, baseToken) {
        const results = await pushTransaction(msg_sender, "depositethaq", {
            msg_sender: msg_sender,
            ethtokenamount: ethtokenamount,
            baseToken: baseToken
        });
        console.log(__line); prettyJson(results);
    }

    async withdraweaq(msg_sender, ethtokenamount, baseToken) {
        const results = await pushTransaction(msg_sender, "withdraweaq", {
            msg_sender: msg_sender,
            ethtokenamount: ethtokenamount,
            baseToken: baseToken
        });
        console.log(__line); prettyJson(results);
    }

    async withdrawaeaq(msg_sender, baseToken) {
        const results = await pushTransaction(msg_sender, "withdrawaeaq", {
            msg_sender: msg_sender,
            ethtokenamount: ethtokenamount,
            baseToken: baseToken
        });
        console.log(__line); prettyJson(results);

    }

    async extransfer(acc) {
        const results = await pushTransaction(bp, "extransfer", {
            from: bp,
            to: acc,
            quantity: to_core_asset(10000, "ROC"),
            memo: ""
        });

        console.log(__line); prettyJson(results);
    }

    async extransfers() {
        const accounts = Object.keys(acc2pub_keys);
        for (let acc of accounts) {
            this.extransfer(acc);
        }
        console.log(__line); prettyJson(results);
    }

}

var arguments = process.argv.splice(2);
console.log(__line); console.log('所传递的参数是：', arguments);

//////////////////////////
// print process.argv
process.argv.forEach(function (val, index, array) {
    console.log(__line); console.log(index + ': ' + val);
});

const client = new EosClient(dodo_ethbase_name);

let handlers = {
    "c": (async function () {
        await client.create_wallet();
    }),
    "i": (async function () {
        await client.import_keys();
    }),
    "a": (async function () {
        await client.allowDosContracts();
    }),
    "t": (async function () {
        await client.extransfers();
    }),
    "tt": (async function () {
        await client.extransfer(lp);
    }),
    "ip": (async function () {
        await client.init(admin, maintainer, to_sym("WETH"), get_core_symbol());
    }),
    "n": (async function () {
        await client.newtoken(tokenissuer, to_max_supply("WETH"));
        await client.newtoken(tokenissuer, to_max_supply("DAI"));
        await client.newtoken(tokenissuer, to_max_supply("MKR"));
    }),
    "m": (async function () {
        await client.mint(lp, to_wei_asset(1000, "MKR"));
        await client.mint(trader, to_wei_asset(1000, "MKR"));
    }),
    "ms": (async function () {
        await client.mint(lp, to_wei_asset(10000, "DAI"));
        await client.mint(trader, to_wei_asset(10000, "DAI"));
        await client.mint(lp, to_wei_asset(10000, "MKR"));
        await client.mint(trader, to_wei_asset(10000, "MKR"));
    }),
    "o": (async function () {
        await client.neworacle(oracleadmin, to_sym("WETH"));
        await client.neworacle(oracleadmin, to_sym("DAI"));
        await client.neworacle(oracleadmin, to_sym("MKR"));
    }),
    "sp": (async function () {
        await client.setprice(oracleadmin, to_wei_asset(1, "DAI"));
    }),
    "b": (async function () {
        //  init(admin, maintainer, to_sym("WETH"), get_core_symbol());
        // // dodoZoo, weth, core_symbol
        // // newethtoken(tokenissuer, to_maximum_supply("WETH"));
        // newtoken(tokenissuer, to_maximum_supply("MKR"));
        // mint(lp, to_wei_asset(1000,"MKR"));
        // mint(trader, to_wei_asset(1000,"MKR"));
        // await client.neworacle(oracleadmin, to_sym("WETH"));
        // await client. neworacle(oracleadmin, to_sym("MKR"));
        //   setprice(oracleadmin, to_asset(1000000,"WETH"));
        await client.setprice(oracleadmin, to_wei_asset(100, "WETH"));
        const msg_sender = admin;
        const dodo_name = dodo_ethbase_name;
        const maintainer = doowner;
        const baseToken = to_sym("WETH");
        const quoteToken = to_sym("MKR");
        const oracle = to_sym("WETH");
        const lpFeeRate = 2;
        const mtFeeRate = 1;
        const k = 1;
        const gasPriceLimit = 0; // gweiStr("100")
        await client.breeddodo(
            msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
        await client.enablex(admin, dodo_name, "enabletradin");
        await client.enablex(admin, dodo_name, "enablequodep");
        await client.enablex(admin, dodo_name, "enablebasdep");
        await client.depositquote(lp, dodo_ethbase_name, to_wei_asset(1000, "MKR"));
        await client.depositethab(lp, to_wei_asset(10, "WETH"), to_sym("MKR"));

    }),
    "q": (async function () {
        //  init(admin, maintainer, to_sym("WETH"), get_core_symbol());
        // // dodoZoo, weth, core_symbol
        // // newethtoken(tokenissuer, to_maximum_supply("WETH"));
        // newtoken(tokenissuer, to_maximum_supply("MKR"));
        // mint(lp, to_wei_asset(1000,"MKR"));
        // mint(trader, to_wei_asset(1000,"MKR"));
        // await client.neworacle(oracleadmin, to_sym("WETH"));
        // await client. neworacle(oracleadmin, to_sym("MKR"));
        //   setprice(oracleadmin, to_asset(1000000,"WETH"));
        await client.setprice(oracleadmin, to_asset(100, "MKR"));

        const msg_sender = admin;
        const dodo_name = dodo_ethquote_name;
        const maintainer = doowner;
        const baseToken = to_sym("WETH");
        const quoteToken = to_sym("MKR");
        const oracle = to_sym("MKR");
        const lpFeeRate = 2;
        const mtFeeRate = 1;
        const k = 1;
        const gasPriceLimit = 0; // gweiStr("100")
        await client.breeddodo(
            msg_sender, dodo_name, maintainer, quoteToken, baseToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
        await client.enablex(admin, dodo_name, "enabletradin");
        await client.enablex(admin, dodo_name, "enablequodep");
        await client.enablex(admin, dodo_name, "enablebasdep");
        await client.depositbase(lp, dodo_ethquote_name, to_wei_asset(1000, "MKR"));
        await client.depositethaq(lp, to_wei_asset(10, "WETH"), to_sym("MKR"));
    }),
    "s": (async function () {
        //  init(admin, maintainer, to_sym("WETH"), get_core_symbol());
        // // dodoZoo, weth, core_symbol
        // // newethtoken(tokenissuer, to_maximum_supply("WETH"));
        // newtoken(tokenissuer, to_maximum_supply("MKR"));
        // mint(lp, to_wei_asset(1000,"MKR"));
        // mint(trader, to_wei_asset(1000,"MKR"));
        // await client.neworacle(oracleadmin, to_sym("WETH"));
        // await client. neworacle(oracleadmin, to_sym("MKR"));
        //   setprice(oracleadmin, to_asset(1000000,"WETH"));
        await client.setprice(oracleadmin, to_wei_asset(1, "DAI"));
        const msg_sender = admin;
        const dodo_name = dodo_stablecoin_name;
        const maintainer = doowner;
        const baseToken = to_sym("DAI");
        const quoteToken = to_sym("MKR");
        const oracle = to_sym("DAI");
        const lpFeeRate = 1;
        const mtFeeRate = 0;
        const k = 1;
        const gasPriceLimit = 0; // gweiStr("100")
        await client.breeddodo(
            msg_sender, dodo_name, maintainer, baseToken, quoteToken, oracle, lpFeeRate, mtFeeRate, k, gasPriceLimit);
        await client.enablex(admin, dodo_name, "enabletradin");
        await client.enablex(admin, dodo_name, "enablequodep");
        await client.enablex(admin, dodo_name, "enablebasdep");
        await client.depositbase(lp, dodo_name, to_wei_asset(10000, "DAI"));
        await client.depositquote(lp, dodo_name, to_wei_asset(10000, "MKR"));
    }),
    "scd": (async function () {
        const dodo_name = dodo_stablecoin_name;
        await client.depositbase(lp, dodo_name, to_wei_asset(10000, "DAI"));
        await client.depositquote(lp, dodo_name, to_wei_asset(10000, "MKR"));
    }),
    "scw": (async function () {
        const dodo_name = dodo_stablecoin_name;
        await client.withdrawbase(lp, dodo_name, to_wei_asset(10000, "DAI"));
        await client.withdrawquote(lp, dodo_name, to_wei_asset(10000, "MKR"));
    }),
    "scwa": (async function () {
        const dodo_name = dodo_stablecoin_name;
        await client.withdrawallb(lp, dodo_name);
        await client.withdrawallq(lp, dodo_name);
    }),
    "buy": (async function () {
        await client.buyethtoken(trader, to_wei_asset(1, "WETH"), to_wei_asset(200, "MKR"));
        await client.sellethtoken(trader, to_wei_asset(1, "WETH"), to_wei_asset(50, "MKR"));
        await client.withdraweab(lp, to_wei_asset(5, "WETH"), to_sym("MKR"));
        await client.withdrawaeab(lp, to_sym("MKR"));
        await client.buytokeneth(trader, to_wei_asset(200, "MKR"), to_asset(21000, "WETH"));
        await client.selltokeneth(trader, to_wei_asset(50, "MKR"), to_asset(4500, "WETH"));
        await client.withdraweaq(lp, to_wei_asset(5, "WETH"), to_sym("MKR"));
        await client.withdrawaeaq(lp, to_sym("MKR"));
        await client.buybasetoken(trader, to_wei_asset(1000, "DAI"), to_wei_asset(1001, "MKR"));
        await client.sellbastoken(trader, to_wei_asset(8990, "DAI"), to_wei_asset(10000, "MKR"));
    }),
    "bbt": (async function () {
        await client.buybasetoken(trader, dodo_stablecoin_name, to_wei_asset(1000, "DAI"), to_wei_asset(1001, "MKR"));
    }),
    "sbt": (async function () {
        await client.sellbastoken(trader, dodo_stablecoin_name, to_wei_asset(8990, "DAI"), to_wei_asset(9000, "MKR"));
    }),
    "default": (async function () {

        console.log(__line); console.log(todecimal((1000)), "test option", todecimal(1000),);
    })

};

// console.log(__line);console.log(process.argv);
const f = handlers[arguments[0]] || handlers["default"];
f();
