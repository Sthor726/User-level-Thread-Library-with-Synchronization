make clean
make
echo "Running tests..."
./async_test 5000 50
./async_test 10 50
./async_test 5000 3
./async_test 10 3
# ./uthread-sync-demo 20 20
# ./lock_test 20 20