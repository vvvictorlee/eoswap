const { chain, wallet } = require('../index');
const w = wallet();
const c = chain();

const { DEFAULT_WALLET_PRIVATE_KEY, PRIVATE_KEY_INITA, PRIVATE_KEY_2, PRIVATE_KEY_1 } = require('../config');

const WALLET_NAME = String(Date.now())
let WALLET_PASSWORD = '';

const WALLET_2 = `${String(Date.now())}233`
let WALLET_2_PASSWORD = '';


beforeAll(async () => {
    const res = await w.create(WALLET_2)
    // //.then(res => {
    return WALLET_2_PASSWORD = res;
    // })
});


describe('WALLET - #create() ', () => {
    it('should create a wallet', async () => {
        const res = await w.create(WALLET_NAME)
        //.then(res => {
        WALLET_PASSWORD = res;
        expect(res).toBeDefined()
        expect(res).toBeTruthy();
        // })
        // .catch(e => {
        //     console.log('wallet_create e ', e);
        // })
    })
})

describe('WALLET - #open() ', () => {
    it('should open a wallet', async () => {
        const res = await w.open(WALLET_NAME)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toBeTruthy();
        expect(res).toEqual({});
        // })
        // .catch(e => {
        //     console.log('open e ', e);
        // })
    })
})

describe('WALLET - #lock() ', () => {
    it('should lock a wallet', async () => {
        const res = await w.lock(WALLET_NAME)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toBeTruthy();
        expect(res).toEqual({});
        // })
        // .catch(e => {
        //     console.log('lock e ', e);
        // })
    })
})

describe('WALLET - #lock_all() ', () => {
    it('should lock all wallets', async () => {
        const res = await w.lock_all(WALLET_NAME)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toBeTruthy();
        expect(res).toEqual({});
        // })
        // .catch(e => {
        //     console.log('lock_all e ', e);
        // })
    })
})

describe('WALLET- #unlock', () => {
    const name = `${WALLET_NAME}1`
    let password = '';
    it('should unlock a wallet', async () => {
        let res = await w.create(name)
        //.then(res => {
        password = res;
        expect(res).toBeDefined()
        expect(res).toBeTruthy();

        /* Now lock it */
        res = await w.lock(name)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toBeTruthy();
        expect(res).toEqual({});


        /* Now unlock it */
        res = await w.unlock(name, password)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toBeTruthy();
        expect(res).toEqual({});
        //                 })
        //                 .catch(e => {
        //                     console.log('unlock e ', e.response.body);
        //                 })
        //         })
        //         .catch(e => {
        //             console.log('lock_all e ', e);
        //         })
        // })
        // .catch(e => {
        //     console.log('wallet_create e ', e);
        // })
    })
})

describe('WALLET - #list', () => {
    it('should list all wallets', async () => {
        const res = await w.list()
        //.then(res => {
        expect(res).toBeDefined()
        expect(Array.isArray(res)).toEqual(true);
        // })
        // .catch(e => {
        //     console.log('lock_all e ', e);
        // })
    })
})

describe('WALLET - #list_keys', () => {
    it('should list all keys in the wallet', async () => {
        const name = `${WALLET_NAME}13`
        let password = '';
        let res = await w.create(name)
        //.then(res => {
        password = res;
        expect(res).toBeDefined()
        expect(res).toBeTruthy();

        res = await w.import_key(name, PRIVATE_KEY_2)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toEqual({});
        res = await w.list_keys(name, password)
        //.then(res => {
        expect(res).toBeDefined()
        expect(Array.isArray(res)).toEqual(true);
        //                 })
        //                 .catch(e => {
        //                     console.log('lock_all e ', e);
        //                 })
        //         })
        //         .catch(e => {
        //             console.log('lock_all e ', e);
        //         })

        // })
        // .catch(e => {
        //     console.log('lock_all e ', e);
        // })


    })

})


describe('WALLET - #get_public_keys', () => {
    it('should list all public keys', async () => {
        const res = await w.get_public_keys()
        //.then(res => {
        expect(res).toBeDefined()
        expect(Array.isArray(res)).toEqual(true);
        // })
        // .catch(e => {
        //     console.log('lock_all e ', e);
        // })
    })
})

describe('WALLET - #wallet_set_timeout', () => {
    it('should set timeout', async () => {
        const res = await w.set_timeout(1)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toEqual({});
        // })
        // .catch(e => {
        //     console.log('lock_all e ', e);
        // })
    })
})


describe('WALLET - #wallet_sign_trx', () => {
    it('should set wallet timeout', async () => {

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
        const bin = await c.abi_json_to_bin("eosio.token", "transfer", { from: "useraaaaaaab", to: "useraaaaaaaa", quantity: '0.0004 SYS', memo: '' })
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
        res = await w.wallet_sign_trx([{
            ref_block_num,
            ref_block_prefix,
            expiration,
            actions,
            signatures,
        },
        ["EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S"],
        info.chain_id
        ])
        //.then(res => {
        expect(res).toBeDefined()
        expect(res.ref_block_num).toEqual(ref_block_num);
        expect(res.expiration).toEqual(expiration);
        expect(Array.isArray(res.actions)).toEqual(true);
        expect(res.signatures).toBeDefined()
        expect(Array.isArray(res.signatures)).toEqual(true);
        //         })
        //         .catch(e => {
        //             console.log('wallet_sign_trx e ', e.response.text);
        //         })
        // })
        // .catch(e => {
        //     console.log('e ', e.response.text);
        // })
    })
})

describe('WALLET - #import_key', () => {
    it('should import a key', async () => {
        let res = await w.unlock(WALLET_2, WALLET_2_PASSWORD)
        //.then(res => {
        expect(res).toBeDefined()
        expect(res).toBeTruthy();
        expect(res).toEqual({});
        res = await w.import_key(WALLET_2, PRIVATE_KEY_INITA)
        //.then(res => {
        console.log('RES', res)
        expect(res).toBeDefined()
        expect(res).toEqual({});
        //         })
        //         .catch(e => {
        //             console.log('lock_all e ', e);
        //         })
        // })
        // .catch(e => {
        //     console.log('unlock e ', e.response.body);
        // })
    })
})
