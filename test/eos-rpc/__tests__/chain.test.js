const { chain, wallet } = require('../index');
const w = wallet();
const c = chain();
const WALLET_NAME = String(Date.now())
const { PRIVATE_KEY_1 } = require('../config');


describe('CHAIN - #get_info() ', () => {
    it('should load chain info', async () => {
        const res = await c.get_info();
        // .then(res => {
        expect(res).toBeDefined()
        expect(res.server_version).toBeDefined()
        expect(res.head_block_num).toBeDefined()
        expect(res.last_irreversible_block_num).toBeDefined()
        expect(res.last_irreversible_block_id).toBeDefined()
        expect(res.head_block_id).toBeDefined()
        expect(res.head_block_time).toBeDefined()
        expect(res.head_block_producer).toBeDefined()
        expect(res.virtual_block_cpu_limit).toBeDefined()
        expect(res.virtual_block_net_limit).toBeDefined()
        expect(res.block_cpu_limit).toBeDefined()
        expect(res.block_net_limit).toBeDefined()
        expect(res.server_version_string).toBeDefined()
        expect(res.fork_db_head_block_num).toBeDefined()
        expect(res.fork_db_head_block_id).toBeDefined()
        expect(res.server_full_version_string).toBeDefined()
        // })
    })
})


describe('CHAIN - #get_block() ', () => {
    it('should load block info', async () => {
        const BLOCK = 4;
        const res = await c.get_block(BLOCK);
        // .then(res => {
        expect(res).toBeDefined()
        expect(res.timestamp).toBeDefined()
        expect(res.producer).toBeDefined()
        expect(res.previous).toBeDefined()
        expect(res.transaction_mroot).toBeDefined()
        expect(res.action_mroot).toBeDefined()
        expect(res.schedule_version).toBeDefined()
        expect(res.new_producers).toBeDefined()
        expect(res.producer_signature).toBeDefined()
        expect(res.transactions).toBeDefined()
        expect(res.id).toBeDefined()
        expect(res.block_num).toBe(BLOCK)
        expect(res.ref_block_prefix).toBeDefined()
        // })
    })
})


describe('CHAIN - #get_account() ', () => {
    it('should load account info', async () => {
        const ACCOUNT = 'useraaaaaaaa';
        const res = await c.get_account(ACCOUNT)
        // .then(res => {
        expect(res).toBeDefined()
        expect(res.account_name).toBe(ACCOUNT)
        expect(res.core_liquid_balance).toBeDefined()
        expect(res.last_code_update).toBeDefined()
        expect(res.net_weight).toBeDefined()
        expect(res.cpu_weight).toBeDefined()
        expect(res.permissions).toBeDefined()
        // })
    })
})

describe('CHAIN - #get_code_hash() ', () => {
    it('should load contract code hash', async () => {
        const ACCOUNT_NAME = 'eosio.token';
        const res = await c.get_code_hash(ACCOUNT_NAME)
        // .then(res => {
        expect(res).toBeDefined()
        expect(res.account_name).toBeDefined()
        expect(res.account_name).toBe(ACCOUNT_NAME)
        expect(res.code_hash).toBeDefined()
        // expect(res.abi).toBeDefined()
        // expect(res.wast).toBeDefined()
        // })
        // .catch(e => {
        //     console.log('eee ', e.message);
        // })
    })
})

describe('CHAIN - #get_table_rows() ', () => {
    it('should load table rows', async () => {
        const res = await c.get_table_rows('eosio', 'eosio.token', 'accounts', true)
        // .then(res => {
        expect(res).toBeDefined()
        expect(res.rows).toBeDefined()
        // expect(res.rows[0].key).toBe('account')
        expect(res.rows[0].balance).toBeDefined()
        // })
        // .catch(e => {
        //     console.log('eee ', e.message);
        // })
    })
})

describe('CHAIN - #abi_json_to_bin() ', () => {
    it('should load bin', async () => {
        const res = await c.abi_json_to_bin('eosio.token', 'transfer', { from: 'useraaaaaaab', to: 'useraaaaaaac', quantity: '10.0000 SYS', memo: '' })
        // .then(res => {
        expect(res).toBeDefined()
        expect(res.binargs).toBeDefined()
        // expect(res.required_scope).toBeDefined()
        // expect(res.required_auth).toBeDefined()
        // })
        // .catch(e => {
        //     console.log('eee ', e);
        // })
    })
})

describe('CHAIN - #get_required_keys() ', () => {
    it('should load required keys', async () => {
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

    })
})
