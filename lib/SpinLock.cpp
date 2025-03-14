#include "SpinLock.h"


/*
From writeup:
To implement a spinlock, we need the ability to run an atomic instruction. We will do this in C++ by
using the std::atomic_flag type. std::atomic_flag provides the test_and_set() method to perform an atomic
operation.
*/

SpinLock::SpinLock()
{
    atomic_value.clear(std::memory_order_release);
}

// Acquire the lock. Spin until the lock is acquired if the lock is already held
void SpinLock::lock()
{
    while(atomic_value.test_and_set(std::memory_order_acquire));
}

// Unlock the lock
void SpinLock::unlock()
{
    atomic_value.clear(std::memory_order_release);
}
