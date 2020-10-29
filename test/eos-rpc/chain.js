/*  Chain API */
const request = require('superagent');

module.exports = (HOST, CHAIN_PORT) => {
    const api = {};
    const genURL = (PATH) => `${HOST}:${CHAIN_PORT}${PATH}`;

    /* get_info */
    api.get_info = async () => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/get_info`;
        request
            .get(genURL(PATH))
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.get_block = async (block_num_or_id) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/get_block`;
        request
            .post(genURL(PATH))
            .send({ block_num_or_id })
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.get_account = async (account_name) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/get_account`;
        request
            .post(genURL(PATH))
            .send({ account_name })
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.get_code_hash = async (account_name) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/get_code_hash`;
        request
            .post(genURL(PATH))
            .send({ account_name })
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.get_table_rows = async (scope, code, table, json) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/get_table_rows`;
        request
            .post(genURL(PATH))
            .send({ scope, code, table, json })
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.abi_json_to_bin = async (code, action, args) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/abi_json_to_bin`;
        request
            .post(genURL(PATH))
            .send({ code, action, args })
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.get_required_keys = async (available_keys, transaction) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/get_required_keys`;
        request
            .post(genURL(PATH))
            .send({ available_keys, transaction })
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                console.log(err);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.push_transaction = async (compression, transaction, signatures) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/push_transaction`;
        request
            .post(genURL(PATH))
            .send({
                compression,
                transaction,
                signatures
            })
            .set('accept', 'json')
            .end((err, res) => {
                // console.log(res.text);
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    api.push_transactions = async (xActions) => new Promise((resolve, reject) => {
        const PATH = `/v1/chain/push_transaction`;
        request
            .post(genURL(PATH))
            .send(xActions)
            .set('accept', 'json')
            .end((err, res) => {
                if (res && res.body) return resolve(res.body);
                return reject(err ? err : new Error('no err or res'))
            });
    })

    return api;
}
