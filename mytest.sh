# /Users/lisheng/mygit/eosio/eosio.cdt/build/bin/eosio-cpp -abigen src/eoswap.cpp -o src/eoswap.wasm
# ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/transfer_tests -- --verbose
# ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose

#  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/approve_tests -- --verbose
case "$1" in
"t") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/transfer_tests -- --verbose;;
"t5") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/transfer5_tests -- --verbose;;
"a")  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/approve_tests -- --verbose ;;
"b")  ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/bind_tests -- --verbose ;;
*) ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose ;;
esac

