/*  Wallet API */
const myutil = require('./util');
let { chain, wallet } = require('../eos-rpc');
// Host defaults to 127.0.0.1, chain_port: 8888, wallet_port: 8888
const c = chain();
const w = wallet();
// const WALLET_NAME = "x";//String(Date.now());   PW5JUgVnkvL6TwRU1kVZc6VBPTfgXmVzDwgEcJ3utkt8oGox1ToAn
const DEFAULT_WALLET_NAME = '345';
let password = '';
const pwdfn = "._";
module.exports = () => {
    const api = {};
    api.put_pwd = async (wallet_name, password) => {
        let res = {};
        try {
            let pwdjson = await myutil.loadJson(pwdfn);
            pwdjson[wallet_name] = password;
            await myutil.writeFile(pwdfn, pwdjson);
        } catch (error) {
            console.log(JSON.stringify(error));
            res = error;
        }
        return res;
    };
    api.get_pwd = async (wallet_name) => {
        let res = {};
        try {
            let pwdjson = await myutil.loadJson(pwdfn);
            res = pwdjson[wallet_name];
        } catch (error) {
            console.log(JSON.stringify(error));
            res = error;
        }
        return res;
    };
    api.create_wallet = async (wallet_name) => {
        let res = {};
        try {

            let res = await w.create(wallet_name);
            console.log(JSON.stringify(res));
            password = res;
            await api.put_pwd(wallet_name, password);
        } catch (error) {
            console.log(JSON.stringify(error));
            res = error;
        }
        return res;
    };
    api.unlock_wallet = async (wallet_name, password) => {
        const name = wallet_name;
        let res = {};
        try {
            res = await w.unlock(name, password);
        } catch (error) {
            console.log(JSON.stringify(error));
            res = error;
        }
        return res;
    };
    api.import_keys_by_wallet_name = async (private_keys, wallet_name, wallet_password) => {
        await api.unlock_wallet(wallet_name, wallet_password);
        let res = {};
        for (const pvt of private_keys) {
            try {
                res = await w.import_key(wallet_name, pvt);
            } catch (error) {
                console.log("import_keys_by_wallet_name===", JSON.stringify(error));
                res = error;
            }
        }
        return res;
    };
    api.import_keys = async (private_keys) => {
        const name = DEFAULT_WALLET_NAME
        password = await api.get_pwd(name);
        let res = await api.import_keys_by_wallet_name(private_keys, name, password);
        return res;
    };
    api.create_default_wallet = async () => {
        const name = DEFAULT_WALLET_NAME
        let res = await api.create_wallet(name);
        return res;
    };
    api.unlock_default_wallet = async () => {
        const name = DEFAULT_WALLET_NAME
        password = await api.get_pwd(name);
        await api.unlock_wallet(name, password);
    };
    api.transaction = async (tx_data) => {
        console.log(JSON.stringify(tx_data));
        let actions = tx_data.actions;
        let pub_keys = tx_data.pub_keys;

        let signatures = [];

        // get last block num
        const info = await c.get_info();
        const ref_block_num = info.last_irreversible_block_num;

        // get info on that block.
        const b = await c.get_block(ref_block_num)
        const ref_block_prefix = b.ref_block_prefix;
        const expiration = new Date(new Date(b.timestamp).getTime() + ((8 * 60 + 2) * 60000)).toISOString().split('.')[0]; //"2018-01-09T10:28:49"

        await api.unlock_default_wallet();

        let bin = {};
        // get abi_json_to_bin
        for (let a of actions) {
            bin = await c.abi_json_to_bin(a.account, a.name, a.data);
            // set data in message
            a.data = bin.binargs;
        }


        // const available_keys = [
        //     'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq',
        //     'EOS7d9A3uLe6As66jzN8j44TXJUqJSK3bFjjEEqR4oTvNAB3iM9SA',
        //     'EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S'
        // ]
        let tx = {
            ref_block_num,
            ref_block_prefix,
            expiration,
            actions,
            signatures,
        };
        res = await c.get_required_keys(pub_keys, tx);

        // get signature
        const sig = await w.wallet_sign_trx([tx,
            res.required_keys,
            info.chain_id
        ]);
        const compression = "none";
        const transaction_extensions = [];
        const context_free_actions = [];
        // push the transaction
        const xAction = await c.push_transaction(compression, { expiration, ref_block_num, ref_block_prefix, context_free_actions, actions, transaction_extensions }, sig.signatures);

        return [bin, sig, xAction];
    };


    return api;

}
