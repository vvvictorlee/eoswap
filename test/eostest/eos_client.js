const Eos = require('eosjs');
const dotenv = require('dotenv');
//const axios = require('axios');
const request = require('request');
let sleep = require('sleep');
// var request = require('request'); // https://www.npmjs.com/package/request
let async = require('async'); // https://www.npmjs.com/package/async

dotenv.load();

const ecc = require('eosjs-ecc')



const interval = process.env.FREQ;
const owner = process.env.ORACLE;
const swapContract = process.env.CONTRACT;

const oraclize = "oraclebosbos";
const nonadmin = "useraaaaaaab";
const admin = "eoswapeoswap";
const pool = "pool";
const admin_pub ="EOS8Znrtgwt8TfpmbVpTKvA2oB8Nqey625CLN8bCN3TEbgx86Dsvr";
const pub = "EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S";

const eos = Eos({
    httpEndpoint: process.env.EOS_PROTOCOL + "://" + process.env.EOS_HOST + ":" + process.env.EOS_PORT,
    keyProvider: [process.env.EOS_KEY, '5JtUScZK2XEp3g9gh7F8bwtPTRAkASmNrrftmx4AxDKD5K4zDnr', '5JUNYmkJ5wVmtVY8x9A1KKzYe9UWLZ4Fq1hzGZxfwfzJB8jkw6u', '5K463ynhZoCDDa4RDcr63cUwWLTnKqmdcoTKTHBjqoKfv4u5V7p', '5JBqSZmzhvf3wopwyAwXH5g2DuNw9xdwgnTtuWLpkWP7YLtDdhp', '5JCtWxuqPzcPUfFukj58q8TqyRJ7asGnhSYvvxi16yq3c5p6JRG', '5K79wAY8rgPwWQSRmyQa2BR8vPicieJdLCXL3cM5Db77QnsJess', "5K2L2my3qUKqj67KU61cSACoxgREkqGFi5nKaLGjbAbbRBYRq1m", "5JN8chYis1d8EYsCdDEKXyjLT3QmpW7HYoVB13dFKenK2uwyR65", "5Kju7hDTh3uCZqpzb5VWAdCp7cA1fAiEd94zdNhU59WNaQMQQmE", "5K6ZCUpk2jn1munFdiADgKgfAqcpGMHKCoJUue65p99xKX9WWCW", "5KAyefwicvJyxDaQ1riCztiSgVKiH37VV9JdSRcrqi88qQkV2gJ"],
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

class EosClient {
    constructor() {

    }

    extransfer() {
        eos.transaction(allowContract(nonadmin, pub, swapContract));
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
                        authorization: [`${nonadmin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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
                    pool_name: pool
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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
                    pool_name: pool
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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
        eos.transaction(allowContract(admin, admin_pub, swapContract));
        eos.contract(swapContract)
            .then((contract) => {
                contract.collect({
                    msg_sender: admin,
                    pool_name: pool
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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
                    pool_name: pool,
                    swapFee: 3000
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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
        eos.transaction(allowContract(admin, admin_pub, swapContract));
        eos.contract(swapContract)
            .then((contract) => {
                contract.bind({
                    msg_sender: admin,
                    pool_name: pool,
                    balance: balance,
                    denorm: to_wei(5)
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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


    joinpool() {
        eos.transaction(allowContract(nonadmin, pub, swapContract));
        eos.contract(swapContract)
            .then((contract) => {
                contract.joinpool({
                    msg_sender: nonadmin,
                    pool_name: pool,
                    poolAmountOut: to_wei(10),
                    maxAmountsIn: [Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER]
                },
                    {
                        scope: swapContract,
                        authorization: [`${nonadmin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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
        eos.transaction(allowContract(nonadmin, pub, swapContract));
        eos.contract(swapContract)
            .then((contract) => {
                contract.exitpool({
                    msg_sender: nonadmin,
                    pool_name: pool,
                    poolAmountIn: to_wei(10),
                    minAmountsOut: [0, 0]
                },
                    {
                        scope: swapContract,
                        authorization: [`${nonadmin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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


    swapamtin() {
        eos.transaction(allowContract(nonadmin, pub, swapContract));
        eos.contract(swapContract)
            .then((contract) => {
                contract.swapamtin({
                    msg_sender: nonadmin,
                    pool_name: pool,
                    tokenAmountIn: "250.0000 WETH@eoswap.token",
                    minAmountOut: "47500.0000 DAI@eoswap.token",
                    maxPrice: to_wei(200)
                },
                    {
                        scope: swapContract,
                        authorization: [`${nonadmin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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


    swapamtout() {
        eos.transaction(allowContract(nonadmin, pub, swapContract));
        eos.contract(swapContract)
            .then((contract) => {
                contract.swapamtout({
                    msg_sender: nonadmin,
                    pool_name: pool,
                    maxAmountIn: "300.0000 WETH@eoswap.token",
                    tokenAmountOut: "100.0000 MKR@eoswap.token",
                    maxPrice: to_wei(500)
                },
                    {
                        scope: swapContract,
                        authorization: [`${nonadmin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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

    newtoken(token) {
        eos.contract(swapContract)
            .then((contract) => {
                contract.newtoken({
                    msg_sender: admin,
                    token: token
                },
                    {
                        scope: swapContract,
                        authorization: [`${admin}@${process.env.ORACLE_PERMISSION || 'active'}`]
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

    mint(pubk,user, amount) {
        eos.transaction(allowContract(user, pubk, swapContract));
        eos.contract(swapContract)
            .then((contract) => {
                contract.mint({
                    msg_sender: user,
                    amt: amount
                },
                    {
                        scope: swapContract,
                        authorization: [`${user}@${process.env.ORACLE_PERMISSION || 'active'}`]
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

switch (arguments[0]) {
    case "p":
        new EosClient().newpool();
        break;
    case "s":
        new EosClient().setswapfee();
        break;
    case "n":
        new EosClient().newtoken("WETH@eoswap.token");
        new EosClient().newtoken("DAI@eoswap.token");
        break;
    case "m":
        new EosClient().mint(admin, "500.0000 WETH@eoswap.token");
        new EosClient().mint(admin, "20000.0000 DAI@eoswap.token");
        new EosClient().mint(nonadmin, "100.0000 WETH@eoswap.token");
        new EosClient().mint(nonadmin, "20000.0000 DAI@eoswap.token");
        break;
    case "b":
        new EosClient().bind("500.0000 WETH@eoswap.token");
        new EosClient().bind("20000.0000 DAI@eoswap.token");
        break;
    case "f":
        new EosClient().finalize();
        break;
    case "j":
        new EosClient().joinpool();
        break;
    case "x":
        new EosClient().exitpool();
        break;
    case "c":
        new EosClient().collect();
        break;
    case "e":
        new EosClient().extransfer();
        break;
    default:
        console.log("test option");
}

