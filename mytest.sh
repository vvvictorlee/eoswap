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


# cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000  set account permission useraaaaaaab  active '{"threshold": 1,"keys": [{"key": "'EOS7yBtksm8Kkg85r4in4uCbfN77uRwe82apM8jjbhFVDgEgz3w8S'","weight": 1}],"accounts": [{"permission":{"actor":"'eoswapeoswap'","permission":"eosio.code"},"weight":1}]}' owner -p useraaaaaaab@owner

# cleos --wallet-url http://127.0.0.1:6666 --url http://127.0.0.1:8000 push action eoswapeoswap extransfer '["useraaaaaaab","useraaaaaaac",{quantity : "1.0000 SYS",contract : "eosio.token"},""]' -p useraaaaaaab@active

# python3 bios-boot-tutorial.py --cleos="cleos --wallet-url http://127.0.0.1:6666 " --nodeos=nodeos --keosd=keosd --contracts-dir="EOSIO_CONTRACTS_DIRECTORY" --old-contracts-dir="EOSIO_OLD_CONTRACTS_DIRECTORY" -w -a

