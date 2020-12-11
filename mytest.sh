# /Users/lisheng/mygit/eosio/eosio.cdt/build/bin/eosio-cpp -abigen src/eosdos.cpp -o src/eosdos.wasm
# ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/transfer_tests -- --verbose
# ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests -- --verbose

#  ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/approve_tests -- --verbose
case "$1" in
"n") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/neworacle_tests -- --verbose ;;
"p") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/setprice_tests -- --verbose ;;
"k") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/set* -- --verbose ;;
"e") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/no_* -- --verbose ;;
"b") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/buy_tiny_base_token_tests -- --verbose ;;
"s") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/sell_token_to_eth_tests -- --verbose ;;
"w") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/withdraw_all_eth_as_quote_tests -- --verbose ;;
"bs") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/buy_base_token_tests -- --verbose ;;
"se") ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests/sell_base_token_tests -- --verbose ;;
*) ./build/tests/unit_test --log_level=test_suite --run_test=eosdos_tests -- --verbose ;;
esac


# cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000  set account permission useraaaaaaab  active '{"threshold": 1,"keys": [{"key": "'EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S'","weight": 1}],"accounts": [{"permission":{"actor":"'eosdoseosdos'","permission":"eosio.code"},"weight":1}]}' owner -p useraaaaaaab@owner

# cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000 push action eosdoseosdos extransfer '["useraaaaaaab","useraaaaaaac",{quantity : "1.0000 SYS",contract : "eosio.token"},""]' -p useraaaaaaab@active

# python3 bios-boot-tutorial.py --cleos="cleos --wallet-url http://127.0.0.1:6666 " --nodeos=nodeos --keosd=keosd --contracts-dir="EOSIO_CONTRACTS_DIRECTORY" --old-contracts-dir="EOSIO_OLD_CONTRACTS_DIRECTORY" -w -a

