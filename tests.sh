make clean
make
echo "Running tests..."
./async_test 20 20
# ./uthread-sync-demo 20 20
# ./lock_test 20 20