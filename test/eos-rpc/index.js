const chain = require('./chain');
const wallet = require('./wallet');

let HOST = '10.11.5.37';
let CHAIN_PORT = '8000';
let WALLET_PORT = '6666';

module.exports = {
  chain: (host, chain_port) => chain(host ? host : HOST, chain_port ? chain_port : CHAIN_PORT),
  wallet: (host, wallet_port) => wallet(host ? host : HOST, wallet_port ? wallet_port : WALLET_PORT),
}
