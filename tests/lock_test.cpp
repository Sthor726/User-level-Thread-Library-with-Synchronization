#include "../lib/uthread.h"
#include "../lib/Lock.h"
#include "../lib/CondVar.h"
#include "../lib/SpinLock.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define SHARED_BUFFER_SIZE 10
#define PRINT_FREQUENCY 100000
#define RANDOM_YIELD_PERCENT 50

// Shared buffer
static int buffer[SHARED_BUFFER_SIZE];
static int head = 0;
static int tail = 0;
static int item_count = 0;

// Shared buffer synchronization
static Lock buffer_lock;
static SpinLock spinlock;
// static CondVar need_space_cv;
// static CondVar need_item_cv;

// Bookkeeping
static int produced_count = 0;
static int consumed_count = 0;
static bool producer_in_critical_section = false;
static bool consumer_in_critical_section = false;

// Verify the buffer is in a good state
void assert_buffer_invariants()
{
    assert(item_count <= SHARED_BUFFER_SIZE);
    assert(item_count >= 0);
    assert(head < SHARED_BUFFER_SIZE);
    assert(head >= 0);
    assert(tail < SHARED_BUFFER_SIZE);
    assert(tail >= 0);

    if (head > tail)
    {
        assert((head - tail) == item_count);
    }
    else if (head < tail)
    {
        assert(((SHARED_BUFFER_SIZE - tail) + head) == item_count);
    }
    else
    {
        assert((item_count == SHARED_BUFFER_SIZE) || (item_count == 0));
    }

    assert(produced_count >= consumed_count);
}

void *producer(void *arg)
{

    bool spinlockflag = *(static_cast<bool *>(arg));
    bool done = false;
    while (!done)
    {
        if (spinlockflag) {
            spinlock.lock();
        }
        else {
            buffer_lock.lock();
        }
        

        // Wait for room in the buffer if needed
        // NOTE: Assuming Mesa semantics
        while (item_count == SHARED_BUFFER_SIZE)
        {
            if (spinlockflag) {
                spinlock.unlock();
            }
            else {
                buffer_lock.unlock();
            }
            uthread_yield();
            if (spinlockflag) {
                spinlock.lock();
            }
            else {
                buffer_lock.lock();
            }
        }

        // Make sure synchronization is working correctly
        assert(!producer_in_critical_section);
        producer_in_critical_section = true;
        assert_buffer_invariants();

        // Place an item in the buffer
        buffer[head] = uthread_self();
        head = (head + 1) % SHARED_BUFFER_SIZE;
        item_count++;
        produced_count++;

    
        if (produced_count >= 1000000)
        {
            done = true;
        }

        producer_in_critical_section = false;
        if (spinlockflag) {
            spinlock.unlock();
        }
        else {
            buffer_lock.unlock();
        }

        // Randomly give another thread a chance
        if ((rand() % 100) < RANDOM_YIELD_PERCENT)
        {
            uthread_yield();
        }
    }

    return nullptr;
}

void *consumer(void *arg)
{
    bool spinlockflag = *(static_cast<bool *>(arg));
    bool done = false;
    while (!done)
    {
        if (spinlockflag) {
            spinlock.lock();
        }
        else {
            buffer_lock.lock();
        }

        // Wait for an item in the buffer if needed
        // NOTE: Assuming Mesa semantics
        while (item_count == 0)
        {
            if (spinlockflag) {
                spinlock.unlock();
            }
            else {
                buffer_lock.unlock();
            }
            uthread_yield();
            if (spinlockflag) {
                spinlock.lock();
            }
            else {
                buffer_lock.lock();
            }
        }

        // Make sure synchronization is working correctly
        assert(!consumer_in_critical_section);
        consumer_in_critical_section = true;
        assert_buffer_invariants();

        // Grab an item from the buffer
        int item = buffer[tail];
        tail = (tail + 1) % SHARED_BUFFER_SIZE;
        item_count--;
        consumed_count++;

        // Print an update periodically
        if ((consumed_count % PRINT_FREQUENCY) == 0)
        {
            cout << "Consumed " << consumed_count << " items" << endl;
        }

        if (consumed_count >= 1000000)
        {
            done = true;
        }


        consumer_in_critical_section = false;
        if (spinlockflag) {
            spinlock.unlock();
        }
        else {
            buffer_lock.unlock();
        }

        // Randomly give another thread a chance
        if ((rand() % 100) < RANDOM_YIELD_PERCENT)
        {
            uthread_yield();
        }
    }

    return nullptr;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: ./uthread-sync-demo <num_producer> <num_consumer>" << endl;
        cerr << "Example: ./uthread-sync-demo 20 20" << endl;
        exit(1);
    }

    int producer_count = atoi(argv[1]);
    int consumer_count = atoi(argv[2]);

    if ((producer_count + consumer_count) > 99)
    {
        cerr << "Error: <num_producer> + <num_consumer> must be <= 99" << endl;
        exit(1);
    }

    // Init user thread library
    int ret = uthread_init(UTHREAD_TIME_QUANTUM);
    if (ret != 0)
    {
        cerr << "Error: uthread_init" << endl;
        exit(1);
    }

    // start timer
    auto start = chrono::high_resolution_clock::now();

    bool* spinlockflag = new bool(false);
    // Create producer threads
    int *producer_threads = new int[producer_count];
    for (int i = 0; i < producer_count; i++)
    {
        producer_threads[i] = uthread_create(producer, spinlockflag);
        if (producer_threads[i] < 0)
        {
            cerr << "Error: uthread_create producer" << endl;
        }
    }

    // Create consumer threads
    int *consumer_threads = new int[consumer_count];
    for (int i = 0; i < consumer_count; i++)
    {
        consumer_threads[i] = uthread_create(consumer, spinlockflag);
        if (consumer_threads[i] < 0)
        {
            cerr << "Error: uthread_create consumer" << endl;
        }
    }

    // Wait for all producers to complete
    for (int i = 0; i < producer_count; i++)
    {
        int result = uthread_join(producer_threads[i], nullptr);
        if (result < 0)
        {
            cerr << "Error: uthread_join producer" << endl;
        }
    }

    // Wait for all consumers to complete
    for (int i = 0; i < consumer_count; i++)
    {
        int result = uthread_join(consumer_threads[i], nullptr);
        if (result < 0)
        {
            cerr << "Error: uthread_join consumer" << endl;
        }
    }

    // end timer
    auto end = chrono::high_resolution_clock::now();
    auto lockduration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Time taken by lock: " << lockduration.count() << " microseconds" << endl;

    delete[] producer_threads;
    delete[] consumer_threads;


    //RESET BUFFER
    head = 0;
    tail = 0;
    item_count = 0;
    produced_count = 0;
    consumed_count = 0;


    // start timer
    start = chrono::high_resolution_clock::now();

    *spinlockflag = true;
    // Create producer threads
    int *new_producer_threads = new int[producer_count];
    for (int i = 0; i < producer_count; i++)
    {
        new_producer_threads[i] = uthread_create(producer, spinlockflag);
        if (new_producer_threads[i] < 0)
        {
            cerr << "Error: uthread_create producer" << endl;
        }
    }

    // Create consumer threads
    int *new_consumer_threads = new int[consumer_count];
    for (int i = 0; i < consumer_count; i++)
    {
        new_consumer_threads[i] = uthread_create(consumer, spinlockflag);
        if (new_consumer_threads[i] < 0)
        {
            cerr << "Error: uthread_create consumer" << endl;
        }
    }

    // Wait for all producers to complete
    for (int i = 0; i < producer_count; i++)
    {
        int result = uthread_join(new_producer_threads[i], nullptr);
        if (result < 0)
        {
            cerr << "Error: uthread_join producer" << endl;
        }
    }

    // Wait for all consumers to complete
    for (int i = 0; i < consumer_count; i++)
    {
        int result = uthread_join(new_consumer_threads[i], nullptr);
        if (result < 0)
        {
            cerr << "Error: uthread_join consumer" << endl;
        }
    }

    // end timer
    end = chrono::high_resolution_clock::now();
    auto spinlockduration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "Time taken by spinlock: "
         << spinlockduration.count() << " microseconds" << endl << endl;

    float percent = float(lockduration.count()) / spinlockduration.count() * 100;

    cout << "lock time: " << lockduration.count() << " microseconds" << endl;
    cout << "spinlock time: " << spinlockduration.count() << " microseconds" << endl;
    cout << "spinlock was faster by  " << percent << " percent" << endl;

    delete[] new_producer_threads;
    delete[] new_consumer_threads;

    return 0;
}
