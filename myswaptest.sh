case "$1" in
"n") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/newpool_tests -- --verbose ;;
"m") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/mint_tests -- --verbose ;;
"c") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/create* -- --verbose ;;
"co") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/collect_tests -- --verbose ;;
"j") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/joinpool_tests -- --verbose ;;
"ex") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/exitpool_tests -- --verbose ;;
"i") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountIn_tests -- --verbose ;;
"o") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountOut_tests -- --verbose ;;
"is") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/issue_tests -- --verbose ;;
"t") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/transfer_tests -- --verbose ;;
"r") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/retire_tests -- --verbose ;;
"op") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/open_tests -- --verbose ;;
"close") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/close_tests -- --verbose ;;
"s") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swap_transfer_tests -- --verbose ;;
"b") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/bind_tests -- --verbose ;;
"bu") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/burn_tests -- --verbose ;;
"e") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/extransfer_tests -- --verbose ;;
"f") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/finalize_tests -- --verbose ;;
*) ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose ;;
esac
