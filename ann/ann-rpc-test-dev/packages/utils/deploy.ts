
const { upload, instantiate } = require("./code");
var path = require("path");
const users = ["Alice", "Bob", "Charlie", "Dave", "Eve", "Ferdie"];
async function deploy() {
    const basepath = "../hex-space-protocol-substrate/"

    let jsonfile = "./kv.json";

    let pathes = {
        "erc1155": basepath + "erc1155", "hex_space": basepath + "hex_space"
    };
    let args = { "erc1155": "", "hex_space": "'5CiPPseXPECbkjWCa6MnjNokrgYjMqmKndv2rSnekmSK2DjL 0xd5f768bbb70d4afc86b404fd767f01a64e4d0ab7f2bd6cd400895adbe79491c2'" }
    let keys = Object.keys(pathes);//.slice(0,7);

    let codehashes: any = [];
    let contractaddresses: any = [];
    let i = 0;
    for (let key of keys) {
        console.log(i, "======para======", users[i], pathes[key as keyof typeof pathes], key);

        if (i == 0) {
            const code_hash = await upload(users[i], pathes[key as keyof typeof pathes], key, jsonfile);
            console.log(i, "======code_hash======", code_hash);

            codehashes.push(code_hash);
            args["hex_space"] = "1 " + code_hash + "";
        } else {
            const contract_address = await instantiate(users[i], args[key as keyof typeof args], "code_hash", pathes[key as keyof typeof pathes], key, jsonfile);
            contractaddresses.push(contract_address);
            console.log(i, "======contract_address======", contract_address)
        }

        i++;

    }
}

const { execSysCmd } = require("./cmd");
let artjsonfile = path.resolve("../abi/art.json");
const artbasepath = "../../../sub-art-nft-marketplace/"

async function uploadArt() {
    let dirs = await execSysCmd("ls -d sub* erc20", artbasepath);
    console.log("dirs===", dirs.split("\n"));
    //  substrate_contract_node -- --dev
    let pathes = dirs.split("\n");
    pathes = pathes.filter((item:any) => item != "sub_contract_management");
    let hashes: any = []
    for (let i = 0; i < pathes.length; i++) {
        const code_hash = await upload(users[i % 6], artbasepath + pathes[i], pathes[i], artjsonfile);
        console.log(i, pathes[i], "======code_hash======", code_hash);
        hashes.push(code_hash)
    }
    console.log(pathes, "======hashes======", hashes);
    return hashes;
}

async function deployArt() {
    let filepath = "sub_contract_management"
    const address = await instantiate(users[0], "", artbasepath + filepath, filepath, artjsonfile);
    console.log("======address======", address);
}

(async function () {
    switch (process.argv[2]) {
        case "up": {
            await uploadArt();
            break;
        }
        case "art": {
            await deployArt();
            break;
        }
        default: {
            console.log(process.argv)
        }
    }

})();


