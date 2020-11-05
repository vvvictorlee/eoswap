/*  Wallet API */
let { chain, wallet } = require('../eos-rpc');
// Host defaults to 127.0.0.1, chain_port: 8888, wallet_port: 8888
const c = chain();
const w = wallet();
const WALLET_NAME = String(Date.now());
let password = '';
module.exports = () => {
    const api = {};
    api.import_keys = async (private_keys) => {
        const name = `${WALLET_NAME}`
        let res = await w.create(name)
        password = res;
        for (const pvt of private_keys) {
            res = await w.import_key(name, pvt)
        }
        return res;
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

        return [xAction,bin,sig];
    };


    return api;

}
