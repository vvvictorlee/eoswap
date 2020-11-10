
CLS=/data/roxe/test/cls

case "$1" in
"u") $CLS wallet unlock -n v --password PW5KSexTLPfZxhbFKTvjhV6MgyDLmsmMN6Vhp2bSGoRqoDkqFNfoD ;;
"n") 
 ;;
"a") 
 curl  http://10.100.1.10:8888/v1/chain/get_account -X POST -d '{"account_name":"eosdosoracle"}' |jq 
#  curl  http://10.100.1.10:8888/v1/chain/get_account -X POST -d '{"account_name":"alice1111111"}' |jq
;;
"g") 
# curl  http://10.100.1.10:8888/v1/chain/get_table_rows -X POST -d '{"scope":"eosdoseosdos", "code":"eosdoseosdos", "table":"oracle", "json": true}' |jq
curl  http://10.100.1.10:8888/v1/chain/get_table_rows -X POST -d '{"scope":"eosdoseosdos", "code":"eosdoseosdos", "table":"dodo", "json": true}' |jq
# curl  http://10.100.1.10:8888/v1/chain/get_table_rows -X POST -d '{"scope":"roxe1", "code":"roxe.token", "table":"accounts", "json": true}'
# $ curl  http://127.0.0.1:8888/v1/chain/get_table_rows -X POST -d '{"scope":"inita", "code":"currency", "table":"account", "json": true, "lower_bound":0, "upper_bound":-1, "limit":10}'
;;
"s") 
;;
*) echo "u --unlock \n n --new account \n d  --depoly contract";;
esac

# $ curl  http://10.100.1.10:8888/v1/chain/get_account -X POST -d '{"account_name":"inita"}'
# http://10.100.1.10:8889/v1/wallet/list_wallets
# http://10.100.1.10:8889/v1/wallet/create_key
# curl http://localhost:6666/v1/wallet/create_key -X POST -d '["default","K1"]' default 为钱包名




# node ./eosrpc_client.js c
# node ./eosrpc_client.js i
# node ./eosrpc_client.js a
# node ./eosrpc_client.js n
# node ./eosrpc_client.js ms
# node ./eosrpc_client.js o
# node ./eosrpc_client.js sp
# node ./eosrpc_client.js s



# buy sell
# node ./eosrpc_client.js bbt
# node ./eosrpc_client.js sbt

# npm config set registry https://registry.npm.taobao.org --global
 
# npm config set disturl https://npm.taobao.org/dist --global

# npm i @types/node --save-dev
# npm i ts-node --save-dev
# npm i typescript --save-dev
# npm i nodemon --save-dev


# nodemon
# 或者
# npm run dev


# npm config set prefix "D:\ProgramFile\nodejs\node_modules\node_global"
 
# npm config set cache "D:\ProgramFile\nodejs\node_modules\node_cache"