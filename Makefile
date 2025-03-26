CC = g++
CFLAGS = -g -lrt --std=c++14
# Remove lrt for MacOS
DEPS = TCB.h uthread.h uthread_private.h Lock.h CondVar.h SpinLock.h async_io.h
# OBJ = ./lib/TCB.o ./lib/uthread.o ./lib/Lock.o ./lib/CondVar.o ./lib/SpinLock.o ./lib/async_io.o
OBJ_SOLN = ./solution/TCB_soln.o ./solution/uthread_soln.o ./lib/Lock.o ./lib/CondVar.o ./lib/SpinLock.o ./lib/async_io.o
MAIN_OBJ_UTHRAD_SYNC = ./tests/uthread_sync_demo.o
MAIN_OBJ_LOCK_TEST = ./tests/lock_test.o
MAIN_OBJ_ASYNC_TEST = ./tests/async_test.o
MAIN_OBJ_COND_VAR_TEST = ./tests/cond_var_test.o

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

all: uthread-sync-demo-from-soln lock_test async_test cond_var_test

uthread-sync-demo-from-soln: $(OBJ_SOLN) $(MAIN_OBJ_UTHRAD_SYNC)
	$(CC) -o uthread-sync-demo $^ $(CFLAGS)

# uthread-sync-demo: $(OBJ) $(MAIN_OBJ_UTHRAD_SYNC)
# 	$(CC) -o $@ $^ $(CFLAGS)

lock_test: $(OBJ_SOLN) $(MAIN_OBJ_LOCK_TEST)
	$(CC) -o lock_test $^ $(CFLAGS)

async_test: $(OBJ_SOLN) $(MAIN_OBJ_ASYNC_TEST)
	$(CC) -o async_test $^ $(CFLAGS)

cond_var_test: $(OBJ_SOLN) $(MAIN_OBJ_COND_VAR_TEST)
	$(CC) -o cond_var_test $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f ./lib/*.o
	rm -f ./tests/*.o
	rm -f *.o uthread-sync-demo lock_test
