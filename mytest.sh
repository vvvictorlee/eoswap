# /Users/lisheng/mygit/eosio/eosio.cdt/build/bin/eosio-cpp -abigen src/eoswap.cpp -o src/eoswap.wasm
# ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/transfer_tests -- --verbose
# ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose

#  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/approve_tests -- --verbose
case "$1" in
"a") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/approve_tests -- --verbose ;;
"e") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/extransfer_tests -- --verbose ;;
"f") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/finalize_tests -- --verbose ;;
"j") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/joinpool_tests -- --verbose ;;
"b") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/bind_tests -- --verbose ;;
"p") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/newpool_tests -- --verbose ;;
"i") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountIn_tests -- --verbose ;;
"o") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountOut_tests -- --verbose ;;
*) ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose ;;
esac


# get_account ${contract_consumer}
# ${!cleos} set account permission eosio active '{"threshold": 1,"keys": [{"key": "EOS6MRyAjQq8ud7hVNYcfnVPJqcVpscN5So8BhtHuGYqET5GDW5CV","weight": 1}],"accounts": [{"permission":{"actor":"'${contract_consumer}'","permission":"eosio.code"},"weight":1}]}' owner -p eosio

# ${!cleos} set account permission ${contract_oracle}  active '{"threshold": 1,"keys": [{"key": "EOS7dUuGNbsh4x1kgv7vMrkgEDxKFwuGjZpixkjQrQGqJqwxpriSM","weight": 1}],"accounts": [{"permission":{"actor":"'${contract_consumer}'","permission":"eosio.code"},"weight":1}]}' owner -p ${contract_oracle}
# owner     1:    1 EOS7ZrkZFcZqzr2rxb2EV7xLeR1YRGcgfG3wwgPJfcf6dbaYCpoym
# active     1:    1 EOS7ZrkZFcZqzr2rxb2EV7xLeR1YRGcgfG3wwgPJfcf6dbaYCpoym

# ${!cleos} set account permission ${contract_consumer}  active '{"threshold": 1,"keys": [{"key": "EOS7uiCTbwNptteEUyMGjnbcsL9YezHnRnDRThSDZ8kAYzqw13","weight": 1}],"accounts": [{"permission":{"actor":"'${contract_oracle}'","permission":"eosio.code"},"weight":1}],"waits":[{"wait_sec":0,"weight":1}]}' owner -p ${contract_consumer}@owner

# ${!cleos}  set account permission ${contract_consumer}  active '{"threshold": 1,"keys": [{"key": "'${consumer_c_pubkey}'","weight": 1}],"accounts": [{"permission":{"actor":"'${contract_consumer}'","permission":"eosio.code"},"weight":1}]}' owner -p ${contract_consumer}@owner


# python3 bios-boot-tutorial.py --cleos="cleos --wallet-url http://127.0.0.1:6666 " --nodeos=nodeos --keosd=keosd --contracts-dir="EOSIO_CONTRACTS_DIRECTORY" --old-contracts-dir="EOSIO_OLD_CONTRACTS_DIRECTORY" -w -a

