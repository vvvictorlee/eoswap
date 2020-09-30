# /Users/lisheng/mygit/eosio/eosio.cdt/build/bin/eosio-cpp -abigen src/eoswap.cpp -o src/eoswap.wasm
# ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/transfer_tests -- --verbose
# ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose

#  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/approve_tests -- --verbose
case "$1" in
"f") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/finalize_tests -- --verbose;;
"j") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/joinpool_tests -- --verbose;;
"a")  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/approve_tests -- --verbose ;;
"b")  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/bind_tests -- --verbose ;;
"p")  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/newpool_tests -- --verbose ;;
"i")  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountIn_tests -- --verbose ;;
"o")  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountOut_tests -- --verbose ;;
*) ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose ;;
esac

