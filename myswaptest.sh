case "$1" in
"j") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/joinpool_tests -- --verbose ;;
"i") ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests/swapExactAmountIn_tests -- --verbose ;;
*) ./build/tests/unit_test --log_level=test_suite --run_test=eoswap_tests -- --verbose ;;
esac
