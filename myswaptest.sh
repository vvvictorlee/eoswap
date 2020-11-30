case "$1" in
"c") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/create* -- --verbose ;;
"j") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/joinpool_tests -- --verbose ;;
"i") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountIn_tests -- --verbose ;;
"is") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/issue_tests -- --verbose ;;
"t") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/transfer_tests -- --verbose ;;
"r") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/retire_tests -- --verbose ;;
"o") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/open_tests -- --verbose ;;
"close") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/close_tests -- --verbose ;;
*) ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose ;;
esac
