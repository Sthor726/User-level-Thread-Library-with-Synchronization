make clean
make
echo "================================"
echo "Running coondition variable test"
echo "================================"
echo ""
./cond_var_test
echo ""
echo "===================================================================="
echo "Running async test with 5000 bytes for message length and 50 threads"
echo "===================================================================="
echo ""
./async_test 5000 50
echo ""
echo "=================================================================="
echo "Running async test with 10 bytes for message length and 50 threads"
echo "=================================================================="
echo ""
./async_test 10 50
echo ""
echo "==================================================================="
echo "Running async test with 5000 bytes for message length and 3 threads"
echo "==================================================================="
echo ""
./async_test 10 3
echo ""
echo "=================================================================="
echo "Running async test with 10 bytes for message length and 3 threads"
echo "=================================================================="
echo ""
./async_test 10 3
echo ""
echo "=================================================================================="
echo "Running lock test with 20 consumers, 20 producers and "
echo "1000 for time wasting amount"
echo "Might take a while to run"
echo "=================================================================================="
echo ""
./lock_test 20 20 1000
echo ""
echo "================================================================================"
echo "Running lock test with 20 consumers, 20 producers and 10 for time wasting amount"
echo "================================================================================"
echo ""
./lock_test 20 20 10

