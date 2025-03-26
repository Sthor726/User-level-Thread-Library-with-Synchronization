make clean
make
echo "Running tests..."


# ./cond_var_test
# ./async_test 5000 50
./lock_test 20 20 1000


# ./uthread-sync-demo 20 20