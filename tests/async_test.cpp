#include "../lib/uthread.h"
#include "../lib/Lock.h"
#include "../lib/CondVar.h"
#include "../lib/SpinLock.h"
#include "../lib/async_io.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>


using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define SHARED_BUFFER_SIZE 10
#define PRINT_FREQUENCY 1000
#define TOTAL_WORK 5000
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

int fd;
FILE* file;
const char* message = "Hello World\n";
const char* nullmessage = "XXXXXXXXXXX\n"; // same size as above
size_t MESSAGELEN = strlen(message);

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

    bool async = *(static_cast<bool *>(arg));
    bool done = false;
    while (!done)
    {
      
        spinlock.lock();
        // Wait for room in the buffer if needed
        // NOTE: Assuming Mesa semantics
        while (item_count == SHARED_BUFFER_SIZE)
        {
            spinlock.unlock();
            uthread_yield();
            spinlock.lock();   
        }

        // Make sure synchronization is working correctly
        assert(!producer_in_critical_section);
        producer_in_critical_section = true;
        assert_buffer_invariants();

        
        if (async)
        {
            std::cout<< "async producer\n";
            fd = open("async.txt", O_RDWR | O_CREAT, 0644);
            if (fd == -1){
                cout << "error opening file\n";
                cout << strerror(errno);
            }
            if (async_write(fd, &message, MESSAGELEN, item_count * MESSAGELEN) == -1){
                cout << "error writing to file\n";
                cout << strerror(errno);
            }

            if (close(fd) == -1){
                cout << "error closing file\n";
                cout << strerror(errno);
            }
        }
        else {
            file = fopen("sync.txt", "w");
            if (file == NULL){
                cout << "error opening file\n";
                cout << strerror(errno);
            }
            if (fprintf(file, "%s", message) == -1){
                cout << "error writing to file\n";
                cout << strerror(errno);
            }

            if (fclose(file) == -1){
                cout << "error closing file\n";
                cout << strerror(errno);
            }
        }

        // Place an item in the buffer
        buffer[head] = uthread_self();
        head = (head + 1) % SHARED_BUFFER_SIZE;
        item_count++;
        produced_count++;

    
        if (produced_count >= TOTAL_WORK)
        {
            done = true;
        }

        producer_in_critical_section = false;
        
        spinlock.unlock();
        

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
    bool async = *(static_cast<bool *>(arg));
    bool done = false;
    while (!done)
    {
        spinlock.lock();
        
        // Wait for an item in the buffer if needed
        // NOTE: Assuming Mesa semantics
        while (item_count == 0 && !done)
        {
            spinlock.unlock();
            uthread_yield();
            spinlock.lock();
            
        }
        if (done)
        {
            spinlock.unlock();
            break;
        }

        // Make sure synchronization is working correctly
        assert(!consumer_in_critical_section);
        consumer_in_critical_section = true;
        assert_buffer_invariants();

        char* buffer = new char[MESSAGELEN];

        if (async)
        {
            cout << "async consumer\n";
            fd = open("async.txt", O_RDWR | O_CREAT, 0644);
            if (fd == -1){
                cout << "error opening file\n";
                cout << strerror(errno);
            } 
            cout << "before read\n";
            if (async_read(fd, &buffer, MESSAGELEN, item_count * MESSAGELEN) ==-1) {
                cout << "error reading from file\n";
                cout << strerror(errno);
            }
            cout << "after read\n";
            if (async_write(fd, &nullmessage, MESSAGELEN, item_count * MESSAGELEN) == -1){
                cout << "error writing to file\n";
                cout << strerror(errno);
            }

            if (close(fd) == -1){
                cout << "error closing file\n";
                cout << strerror(errno);
            }
            cout << buffer;
        } else {
            file = fopen("sync.txt", "w");
            fgets(buffer, MESSAGELEN, file);
            // fwrite(&message, sizeof(message), 1, fd); 
            fclose(file);
        }

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

        if (consumed_count >= TOTAL_WORK)
        {
            done = true;
        }


        consumer_in_critical_section = false;
        spinlock.unlock();
        

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

    bool* async = new bool(false);
    // Create producer threads
    int *producer_threads = new int[producer_count];
    for (int i = 0; i < producer_count; i++)
    {
        producer_threads[i] = uthread_create(producer, async);
        if (producer_threads[i] < 0)
        {
            cerr << "Error: uthread_create producer" << endl;
        }
    }

    // Create consumer threads
    int *consumer_threads = new int[consumer_count];
    for (int i = 0; i < consumer_count; i++)
    {
        consumer_threads[i] = uthread_create(consumer, async);
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
    cout << "Time taken by sync: " << lockduration.count() << " microseconds" << endl;

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

    *async = true;
    // Create producer threads
    int *new_producer_threads = new int[producer_count];
    for (int i = 0; i < producer_count; i++)
    {
        new_producer_threads[i] = uthread_create(producer, async);
        if (new_producer_threads[i] < 0)
        {
            cerr << "Error: uthread_create producer" << endl;
        }
    }

    // Create consumer threads
    int *new_consumer_threads = new int[consumer_count];
    for (int i = 0; i < consumer_count; i++)
    {
        new_consumer_threads[i] = uthread_create(consumer, async);
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
