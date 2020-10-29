const { chain, wallet } = require('../index');
const w = wallet();
const c = chain();
const WALLET_NAME = String(Date.now())

const { PRIVATE_KEY_1 } = require('../config');

describe('X-Action - #create() ', () => {
    it('should create a transaction', async () => {


        let actions = [{
            account: 'eosio.token',
            name: 'transfer',
            authorization: [{ actor: 'useraaaaaaab', permission: 'active' }],
            data: '',

        }];
        let signatures = [];
        const scope = ["useraaaaaaaa", "useraaaaaaab"];

        // get last block num
        const info = await c.get_info()
        // .then(info => {
        const ref_block_num = info.last_irreversible_block_num

        // get info on that block.
        const b = await c.get_block(ref_block_num)
        // .then(b => {
        const ref_block_prefix = b.ref_block_prefix;
        const expiration = new Date(new Date(b.timestamp).getTime() + ((8 * 60 + 2) * 60000)).toISOString().split('.')[0]; //"2018-01-09T10:28:49"

        // get abi_json_to_bin
        const bin = await c.abi_json_to_bin("eosio.token", "transfer", { from: "useraaaaaaab", to: "useraaaaaaaa", quantity: '0.0005 SYS', memo: '' })
        // .then(bin => {
        // set data in message
        actions[0].data = bin.binargs;

        const name = `${WALLET_NAME}13`
        let password = '';
        let res = await w.create(name)
        // .then(res => {
        password = res;
        expect(res).toBeDefined();
        expect(res).toBeTruthy();

        res = await w.import_key(name, PRIVATE_KEY_1)
        // .then(res => {
        expect(res).toBeDefined()
        expect(res).toEqual({});
        const available_keys = [
            'EOS4toFS3YXEQCkuuw1aqDLrtHim86Gz9u3hBdcBw5KNPZcursVHq',
            'EOS7d9A3uLe6As66jzN8j44TXJUqJSK3bFjjEEqR4oTvNAB3iM9SA',
            'EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S'
        ]
        res = await c.get_required_keys(available_keys, {
            ref_block_num,
            ref_block_prefix,
            expiration,
            actions,
            signatures,
        })
        // .then(res => {
        // console.log(res);
        expect(res).toBeDefined()
        expect(res.required_keys).toBeDefined();

        // get signature
        const sig = await w.wallet_sign_trx([{
            ref_block_num,
            ref_block_prefix,
            expiration,
            actions,
            signatures,
        },
        ["EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S"],
        info.chain_id
        ])
        // .then(sig => {
        // console.log(sig);
        const compression = "none";
        const transaction_extensions = [];
        const context_free_actions = [];
        // // get last block num
        // c.get_info()
        //     .then(info => {
        //         const ref_block_num = info.last_irreversible_block_num

        //         // get info on that block.
        //         c.get_block(ref_block_num)
        //             .then(b => {
        //                 const ref_block_prefix = b.ref_block_prefix;
        //                 console.log("b.timestamp==", b.timestamp.split('.')[0]);
        //                 const expiration = new Date(new Date(b.timestamp.split('.')[0]).getTime() + ((8 * 60 + 2) * 60000)).toISOString().split('.')[0]; //"2018-01-09T10:28:49"
        //                 console.log("expiration==", expiration);


        // push the transaction
        const xAction = await c.push_transaction(compression, { expiration, ref_block_num, ref_block_prefix, context_free_actions, actions, transaction_extensions }, sig.signatures)
        // .then(xAction => {
        expect(xAction).toBeDefined()
        expect(xAction.transaction_id).toBeDefined()
        expect(xAction.processed).toBeDefined()
        expect(xAction.processed.block_num).toBeDefined()
        expect(xAction.processed.block_time).toBeDefined()
        //                                                                 })
        //                                                                 .catch(e => {
        //                                                                     console.log('eee ', e);
        //                                                                 }) // push xaction
        //                                                             //         }) // get block
        //                                                             // }) // get info

        //                                                         }) // get sig
        //                                                         .catch(e => console.error('get sig err ', e.response.text))
        //                                                 })
        //                                                 .catch(e => {
        //                                                     console.log('eee ', e);
        //                                                 })
        //                                         })
        //                                         .catch(e => {
        //                                             console.log('lock_all e ', e);
        //                                         })

        //                                 })
        //                                 .catch(e => {
        //                                     console.log('lock_all e ', e);
        //                                 })


        //                         }) // abi
        //                 }) // get block
        //         }) // get info
    }) // it
}) // define
