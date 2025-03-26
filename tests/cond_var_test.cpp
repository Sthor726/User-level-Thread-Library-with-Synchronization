#include "../lib/uthread.h"
#include "../lib/Lock.h"
#include "../lib/CondVar.h"
#include <iostream>

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000

Lock lock;
CondVar condvar;
vector <int> tids; 
bool thread_awake = false;

void* worker_thread(void* arg) {
    lock.lock();
    cout << "Thread " << uthread_self() << " is waiting." << endl;
    condvar.wait(lock);
    
    cout << "Thread " << uthread_self() << " is awake." << endl;
    thread_awake = true;

    lock.unlock();

    uthread_yield();
    return nullptr;
}

int main() {
    cout << "Main thread starting." << endl;
    uthread_init(UTHREAD_TIME_QUANTUM);

    // Create worker threads
    for (int i = 0; i < 5; i++) {
        tids.push_back(uthread_create(worker_thread, nullptr));
    }

    uthread_yield(); // Allow worker threads to start and wait

    
    lock.lock();
    cout << "Main thread signaling one thread." << endl;
    condvar.signal();
    // Due to mesa semantics, the signaled thread may not wake up immediately
    // we need to wait block untill it has a chnase to run
    while (!thread_awake) {
        lock.unlock();
        uthread_yield();
        lock.lock();
    }
    lock.unlock();

  
    lock.lock();
    
    cout << "Main thread broadcasting to all threads." << endl;
    condvar.broadcast();
    lock.unlock();
    
    // Wait for all threads to complete
    for (int i = 0; i < 5; i++) {
        uthread_join(tids[i], nullptr);
    }
    
    return 0;
}