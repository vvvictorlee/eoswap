case "$1" in
"n") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/neworacle_tests -- --verbose ;;
"s") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/setprice_tests -- --verbose ;;
"b") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/breeddodo_tests -- --verbose ;;
"u") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/buyethtoken_tests -- --verbose ;;
"o") curl -X POST --url http://10.11.5.37:8000/v1/chain/get_table_rows  -d '{
  "code":"eosdoseosdos",
  "scope": "eosdoseosdos",
  "table": "oracle",
  "json": true
}' | jq;;
*)  echo "o " ;;
esac

# python3 bios-boot-tutorial.py --cleos="cleos --wallet-url http://10.11.5.37:6666 " --nodeos=nodeos --keosd=keosd --contracts-dir="/Users/lisheng/mygit/vvvictorlee/eoswap/build/contracts" --old-contracts-dir="/Users/lisheng/testchaincontracts/eosio.contracts-1.8.x/build/contracts" -w -a


# cleos --wallet-url http://10.11.5.37:6666 --url http://10.11.5.37:8000 set contract eoswapeoswap /Users/lisheng/mygit/vvvictorlee/eoswap/build/contracts/eoswap/


# cleos --wallet-url http://10.11.5.37:6666 --url http://10.11.5.37:8000 push action eoswapeoswap extransfer '[["useraaaaaaab","useraaaaaaac",["1.0000 SYS","eosio.token"],""]]'

# cleos --wallet-url http://10.11.5.37:6666 --url http://10.11.5.37:8000  set account permission useraaaaaaab  active '{"threshold": 1,"keys": [{"key": "'EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S'","weight": 1}],"accounts": [{"permission":{"actor":"'eoswapeoswap'","permission":"eosio.code"},"weight":1}]}' owner -p useraaaaaaab@owner

# cleos --wallet-url http://10.11.5.37:6666 --url http://10.11.5.37:8000 push action eoswapeoswap extransfer '["useraaaaaaab","useraaaaaaac",{quantity : "1.0000 SYS",contract : "eosio.token"},""]' -p useraaaaaaab@active

# cleos --wallet-url http://10.11.5.37:6666 --url http://10.11.5.37:8000 get scope eosio.token stats


# curl -X POST --url http://10.11.5.37:8000/v1/chain/get_table_by_scope -d '{
#   "code": "eosio.token",
#   "table": "accounts"
# }'

# curl -X POST --url http://10.11.5.37:8000/v1/chain/get_table_rows  -d '{
#   "code":"eosdosxtoken",
#   "scope": "eosdosxtoken",
#   "table": "account",
#   "json": true
# }'



