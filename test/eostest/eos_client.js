const Eos = require('eosjs');
const dotenv = require('dotenv');
//const axios = require('axios');
const request = require('request');
let sleep = require('sleep');
// var request = require('request'); // https://www.npmjs.com/package/request
let async = require('async'); // https://www.npmjs.com/package/async
// const { logTime } = require("./log_aop");
const { logTime, observe } = require("./log_aop");

dotenv.load();

const ecc = require('eosjs-ecc')



const interval = process.env.FREQ;
const owner = process.env.ADMIN;
const swapContract = process.env.CONTRACT;

const nonadmin = "alice1111111";
const user1 = "bob111111111";
const admin = "eoswapeoswap";
const pool = "pool";
const pool1 = "pool3";
const admin_pub = "EOS69tWc1VS6aP2P1D8ryzTiakPAYbV3whbHeWUzfD8QWYuHKqQxk";
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
        ]
    };

    return tx_data;
};

// const pub = "EOS89PeKPVQG3f48KCX2NEg6HDW7YcoSracQMRpy46da74yi3fTLP";
// eos.transaction(allowContract(nonadmin, pub, nonadmin));
//   await oraclizeContract.setup(oraclizeAccount, oracle, masterAccount, {
// 	authorization: [oraclizeAccount]
//   });



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

function to_sym(sym) {
    return "4," + sym + "@eoswap.token";
}

function to_asset(value, sym) {
    return value + ".0000 " + sym + "@eoswap.token";
}

function to_wei_asset(value, sym) {
    return to_asset(value + "00", sym);
}


class EosClient {
    constructor(pool_name) {
        this.poolName = pool_name;
    }
    allowSwapContract(user, pubk) {
        eos.transaction(allowContract(user, pubk, swapContract));
    }

    allowSwapContracts() {
        this.allowSwapContract(admin, admin_pub);
        this.allowSwapContract(nonadmin, pub);
        this.allowSwapContract(user1, user1_pub);
    }

    async newtoken(token) {
        eos.contract(swapContract)
            .then((contract) => {
                contract.newtoken({
                    msg_sender: admin,
                    token: token
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }

    async mint(user, amount) {
        eos.contract(swapContract)
            .then((contract) => {
                contract.mint({
                    msg_sender: user,
                    amt: amount
                },
                    {
                        scope: swapContract,
                        authorization: [`${user}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }


    extransfer() {

        eos.contract(swapContract)
            .then((contract) => {
                contract.extransfer({
                    from: nonadmin,
                    to: admin,
                    quantity: "1.0000 SYS@eosio.token",
                    memo: ""
                },
                    {
                        scope: swapContract,
                        authorization: [`${nonadmin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }


    newpool() {
        eos.contract(swapContract)
            .then((contract) => {
                contract.newpool({
                    msg_sender: admin,
                    pool_name: this.poolName
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }

    setswapfee() {

        eos.contract(swapContract)
            .then((contract) => {
                contract.setswapfee({
                    msg_sender: admin,
                    pool_name: this.poolName,
                    swapFee: 3000
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }


    bind(balance) {

        eos.contract(swapContract)
            .then((contract) => {
                contract.bind({
                    msg_sender: admin,
                    pool_name: this.poolName,
                    balance: balance,
                    denorm: to_wei(5)
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }

    finalize() {

        eos.contract(swapContract)
            .then((contract) => {
                contract.finalize({
                    msg_sender: admin,
                    pool_name: this.poolName
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }


    joinpool(user) {
        eos.contract(swapContract)
            .then((contract) => {
                contract.joinpool({
                    msg_sender: user,
                    pool_name: this.poolName,
                    poolAmountOut: to_wei(10),
                    maxAmountsIn: [Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER]
                },
                    {
                        scope: swapContract,
                        authorization: [`${user}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }

    exitpool() {

        eos.contract(swapContract)
            .then((contract) => {
                contract.exitpool({
                    msg_sender: nonadmin,
                    pool_name: this.poolName,
                    poolAmountIn: to_wei(10),
                    minAmountsOut: [0, 0]
                },
                    {
                        scope: swapContract,
                        authorization: [`${nonadmin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }



    collect() {

        eos.contract(swapContract)
            .then((contract) => {
                contract.collect({
                    msg_sender: admin,
                    pool_name: this.poolName
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }

    swapamtin(user) {

        eos.contract(swapContract)
            .then((contract) => {
                contract.swapamtin({
                    msg_sender: user,
                    pool_name: this.poolName,
                    tokenAmountIn: to_asset(250, "WETH"),
                    minAmountOut: to_wei_asset(475, "DAI"),
                    maxPrice: to_wei(200)
                },
                    {
                        scope: swapContract,
                        authorization: [`${user}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }


    swapamtout(user) {

        eos.contract(swapContract)
            .then((contract) => {
                contract.swapamtout({
                    msg_sender: user,
                    pool_name: this.poolName,
                    maxAmountIn: to_wei_asset(3, "WETH"),
                    tokenAmountOut: to_wei_asset(1, "MKR"),
                    maxPrice: to_wei(500)
                },
                    {
                        scope: swapContract,
                        authorization: [`${user}@${process.env.SWAP_PERMISSION || 'active'}`]
                    })
                    .then(results => {
                        console.log("results:", results);
                    })
                    .catch(error => {
                        console.log("error:", error);
                    });
            })
            .catch(error => {
                console.log("error:", error);
            });
    }

}

var arguments = process.argv.splice(2);
console.log('所传递的参数是：', arguments);

//////////////////////////
// print process.argv
process.argv.forEach(function (val, index, array) {
    console.log(index + ': ' + val);
});

const client = new EosClient(pool);
const client1 = new EosClient(pool1);

let handlers = {
    "a": (async function () {
        client.allowSwapContracts();
    }),
    "n": (async function () {
        await client.newtoken(to_sym("WETH"));
        await client.newtoken(to_sym("DAI"));
    }),
    "m": (async function () {
        await client.mint(admin, to_wei_asset(5, "WETH"));
        await client.mint(admin, to_wei_asset(200, "DAI"));
        await client.mint(nonadmin, to_wei_asset(1, "WETH"));
        await client.mint(nonadmin, to_wei_asset(200, "DAI"));
    }),
    "p": (async function () {
        client.newpool();
    }),
    "s": (async function () {
        client.setswapfee();
    }),
    "b": (async function () {
        client.bind(to_wei_asset(5, "WETH"));
        client.bind(to_wei_asset(200, "DAI"));
    }),
    "f": (async function () {
        client.finalize();
    }),
    "j": (async function () {
        client.joinpool(nonadmin);
    }),
    "x": (async function () {
        client.exitpool();
    }),
    "c": (async function () {
        client.collect();
    }),
    "i": (async function () {
        client1.swapamtin(user1);
    }),
    "o": (async function () {
        client1.swapamtout(user1);
    }),
    "e": (async function () {
        logTime(client.extransfer)();
    }),
    "B": (async function () {
        await client1.newtoken(to_sym("MKR"));
        await client1.newtoken(to_sym("XXX"));
        await client1.mint(admin, to_wei_asset(50, "WETH"));
        await client1.mint(admin, to_wei_asset(200, "MKR"));
        await client1.mint(admin, to_wei_asset(10000, "DAI"));
        await client1.mint(admin, to_wei_asset(10, "XXX"));
        await client1.mint(user1, to_wei_asset(25, "WETH"));
        await client1.mint(user1, to_wei_asset(4, "MKR"));
        await client1.mint(user1, to_wei_asset(40000, "DAI"));
        await client1.mint(user1, to_wei_asset(10, "XXX"));
        client1.newpool();
        client1.setswapfee();
        client1.bind(to_wei_asset(50, "WETH"));
        client1.bind(to_wei_asset(20, "MKR"));
        client1.bind(to_wei_asset(10000, "DAI"));
        client1.finalize();
        client1.joinpool(user1);
    }),
    default: (async function () {
        console.log("test option");
    })

};

// console.log(process.argv);
const f = handlers[arguments[0]] || handlers["default"];
f();
