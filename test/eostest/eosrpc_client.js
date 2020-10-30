const Eos = require('eosjs');
const dotenv = require('dotenv');
//const axios = require('axios');
const request = require('request');
let sleep = require('sleep');
// var request = require('request'); // https://www.npmjs.com/package/request
let async = require('async'); // https://www.npmjs.com/package/async
// const { logTime } = require("./log_aop");
require("./log_aop");
const jq = require('node-jq');

const prettyJson = async (log) => {
    let jsonstr = await jq.run('.', JSON.stringify(log), { input: 'string', output: 'pretty' });
    console.log(jsonstr);
};

dotenv.load();

const ecc = require('eosjs-ecc')
const EOS_RPC = require('./eos_rpc')
const eosrpc = EOS_RPC();

const interval = process.env.FREQ;
const owner = process.env.ADMIN;
const dosContract = process.env.DOS_CONTRACT;

const nonadmin = "alice1111111";
const user1 = "bob111111111";
const admin = "eosdoseosdos";
const tokenowner = "eosdosxtoken";
const dodo = "dodo";
const dodo1 = "dodo3";
const admin_pub = "EOS69tWc1VS6aP2P1D8ryzTiakPAYbV3whbHeWUzfD8QWYuHKqQxk";
const tokenowner_pub = "EOS69tWc1VS6aP2P1D8ryzTiakPAYbV3whbHeWUzfD8QWYuHKqQxk";
const pub = "EOS69X3383RzBZj41k73CSjUNXM5MYGpnDxyPnWUKPEtYQmTBWz4D";
const user1_pub = "EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S";

const eos = Eos({
    httpEndpoint: process.env.EOS_PROTOCOL + "://" + process.env.EOS_HOST + ":" + process.env.EOS_PORT,
    keyProvider: [process.env.EOS_KEY, '5JtUScZK2XEp3g9gh7F8bwtPTRAkASmNrrftmx4AxDKD5K4zDnr', '5JUNYmkJ5wVmtVY8x9A1KKzYe9UWLZ4Fq1hzGZxfwfzJB8jkw6u', '5KZFvhuNuU3es7hEoAorppkhfCuAfqBGGtzqvesArmzwVwJf64B', '5JtUScZK2XEp3g9gh7F8bwtPTRAkASmNrrftmx4AxDKD5K4zDnr', '5JCtWxuqPzcPUfFukj58q8TqyRJ7asGnhSYvvxi16yq3c5p6JRG', '5K79wAY8rgPwWQSRmyQa2BR8vPicieJdLCXL3cM5Db77QnsJess', "5K2L2my3qUKqj67KU61cSACoxgREkqGFi5nKaLGjbAbbRBYRq1m", "5JN8chYis1d8EYsCdDEKXyjLT3QmpW7HYoVB13dFKenK2uwyR65", "5Kju7hDTh3uCZqpzb5VWAdCp7cA1fAiEd94zdNhU59WNaQMQQmE", "5K6ZCUpk2jn1munFdiADgKgfAqcpGMHKCoJUue65p99xKX9WWCW"],
    chainId: process.env.EOS_CHAIN,
    verbose: false,
    logger: {
        log: null,
        error: null
    }
});


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

function to_sym(sym) {
    return { sym: "4," + sym, contract: 'eosdosxtoken' };
}

function to_asset(value, sym) {
    return { quantity: value + ".0000 " + sym, contract: "eosdosxtoken" };
}

function to_wei_asset(value, sym) {
    return to_asset(value + "00", sym);
}

class EosClient {
    constructor(dodo_name) {
        this.dodoName = dodo_name;
    }
    async allowSwapContract(user, pubk) {
        await eosrpc.transaction(allowContract(user, pubk, dosContract));
    }

    async allowSwapContracts() {
        this.allowSwapContract(admin, admin_pub);
        this.allowSwapContract(tokenowner, tokenowner_pub);
        this.allowSwapContract(nonadmin, pub);
        this.allowSwapContract(user1, user1_pub);
    }

    async import_keys() {
        const keys = [process.env.EOS_KEY, '5JtUScZK2XEp3g9gh7F8bwtPTRAkASmNrrftmx4AxDKD5K4zDnr', '5JUNYmkJ5wVmtVY8x9A1KKzYe9UWLZ4Fq1hzGZxfwfzJB8jkw6u', '5KZFvhuNuU3es7hEoAorppkhfCuAfqBGGtzqvesArmzwVwJf64B', '5JtUScZK2XEp3g9gh7F8bwtPTRAkASmNrrftmx4AxDKD5K4zDnr', '5JCtWxuqPzcPUfFukj58q8TqyRJ7asGnhSYvvxi16yq3c5p6JRG', '5K79wAY8rgPwWQSRmyQa2BR8vPicieJdLCXL3cM5Db77QnsJess', "5K2L2my3qUKqj67KU61cSACoxgREkqGFi5nKaLGjbAbbRBYRq1m", "5JN8chYis1d8EYsCdDEKXyjLT3QmpW7HYoVB13dFKenK2uwyR65", "5Kju7hDTh3uCZqpzb5VWAdCp7cA1fAiEd94zdNhU59WNaQMQQmE", "5K6ZCUpk2jn1munFdiADgKgfAqcpGMHKCoJUue65p99xKX9WWCW"];
        const results = await eosrpc.import_keys(keys);

        console.log(__line); prettyJson(results);
    }
    async newtoken(token) {
        const results = await eosrpc.transaction(pushAction(admin, admin_pub, "mint", {
            msg_sender: admin,
            token: token
        }));

        console.log(__line); prettyJson(results);

    }

    async mint(user, amount) {
        const results = await eosrpc.transaction(pushAction(admin, admin_pub, "mint", {
            msg_sender: user,
            amt: amount
        }));

        console.log(__line); prettyJson(results);
    }

    async extransfer() {
        const results = await eosrpc.transaction(pushAction(admin, admin_pub, "extransfer", {
            from: nonadmin,
            to: admin,
            quantity: "1.0000 SYS@eosio.token",
            memo: ""
        }));

        console.log(__line); prettyJson(results);
    }

    async neworacle() {
        const results = await eosrpc.transaction(pushAction(admin, admin_pub, "neworacle", {
            msg_sender: admin,
            token: to_sym("WETH")
        }));

        console.log(__line); console.log("results:", JSON.stringify(results));
    }

    async setprice() {
        const results = await eosrpc.transaction(pushAction(admin, admin_pub, "setprice", {
            msg_sender: admin,
            amt: to_wei_asset(100, "WETH")
        }));

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

const client = new EosClient(dodo);
const client1 = new EosClient(dodo1);

let handlers = {
    "i": (async function () {
        await client.import_keys();
    }),
    "a": (async function () {
        await client.allowSwapContracts();
    }),
    "n": (async function () {
        await client.newtoken(to_max_supply("WETH"));
        await client.newtoken(to_max_supply("MKR"));
    }),
    "m": (async function () {
        await client.mint(admin, to_wei_asset(1000, "WETH"));
        await client.mint(admin, to_wei_asset(2000, "MKR"));
        await client.mint(lp, to_wei_asset(1000, "WETH"));
        await client.mint(lp, to_wei_asset(2000, "MKR"));
        await client.mint(trader, to_wei_asset(1000, "WETH"));
        await client.mint(trader, to_wei_asset(2000, "MKR"));
    }),
    "o": (async function () {
        await client.neworacle(admin, "WETH");
    }),
    "s": (async function () {
        await client.setprice(admin, to_wei_asset(1000, "WETH"));
    }),
    "default": (async function () {
        console.log(__line); console.log("test option");
    })

};

// console.log(__line);console.log(process.argv);
const f = handlers[arguments[0]] || handlers["default"];
f();
