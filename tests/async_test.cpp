#include "../lib/uthread.h"
#include "../lib/SpinLock.h"
#include "../lib/async_io.h"
#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

using namespace std;

#define UTHREAD_TIME_QUANTUM 10000
#define MAX_THREADS 50

int MESSAGELEN;
int NUM_THREADS;

void *sync_io(void *arg) {
    char buffer[MESSAGELEN];
    int read_fd = open("long.txt", O_RDONLY);
    if (read_fd == -1) {
        cerr << "Error opening long.txt: " << strerror(errno) << endl;
        return nullptr;
    }
    
    int write_fd = open("sync.txt", O_RDWR | O_CREAT, 0644);
    if (write_fd == -1) {
        cerr << "Error opening sync.txt: " << strerror(errno) << endl;
        close(read_fd);
        return nullptr;
    }

    if (read(read_fd, buffer, MESSAGELEN) == -1) {
        cerr << "Error reading long.txt: " << strerror(errno) << endl;
    }

    if (write(write_fd, buffer, MESSAGELEN) == -1) {
        cerr << "Error writing to sync.txt: " << strerror(errno) << endl;
    }

    close(read_fd);
    close(write_fd);
    return nullptr;
}

void *async_io(void *arg) {
    char buffer[MESSAGELEN];
    int read_fd = open("long.txt", O_RDONLY);
    if (read_fd == -1) {
        cerr << "Error opening long.txt: " << strerror(errno) << endl;
        return nullptr;
    }
    
    int write_fd = open("async.txt", O_RDWR | O_CREAT | O_NONBLOCK, 0644);
    if (write_fd == -1) {
        cerr << "Error opening async.txt: " << strerror(errno) << endl;
        close(read_fd);
        return nullptr;
    }

    if (async_read(read_fd, buffer, MESSAGELEN, 0) == -1) {
        cerr << "Error reading long.txt: " << strerror(errno) << endl;
    }

    if (async_write(write_fd, buffer, MESSAGELEN, 0) == -1) {
        cerr << "Error writing to async.txt: " << strerror(errno) << endl;
    }

    close(read_fd);
    close(write_fd);
    return nullptr;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <message_length> <num_threads>" << endl;
        return 1;
    }

    MESSAGELEN = atoi(argv[1]);
    NUM_THREADS = atoi(argv[2]);
    if (MESSAGELEN <= 0 || NUM_THREADS <= 0 || NUM_THREADS > MAX_THREADS) {
        cerr << "Error: message_length must be a positive integer and num_threads must be between 1 and " << MAX_THREADS << "." << endl;
        return 1;
    }

    cout << "=============================" << endl;
    cout << " Message Length: " << MESSAGELEN << " bytes" << endl;
    cout << " Number of Threads: " << NUM_THREADS << endl;
    cout << "=============================" << endl;

    uthread_init(UTHREAD_TIME_QUANTUM);

    auto start = chrono::high_resolution_clock::now();
    int sync_threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        sync_threads[i] = uthread_create(sync_io, nullptr);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_join(sync_threads[i], nullptr);
    }
    auto sync_end = chrono::high_resolution_clock::now();

    auto async_start = chrono::high_resolution_clock::now();
    int async_threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        async_threads[i] = uthread_create(async_io, nullptr);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        uthread_join(async_threads[i], nullptr);
    }
    auto async_end = chrono::high_resolution_clock::now();

    long sync_time = chrono::duration_cast<chrono::microseconds>(sync_end - start).count();
    long async_time = chrono::duration_cast<chrono::microseconds>(async_end - async_start).count();

    cout << "Sync I/O time: " << sync_time << " microseconds" << endl;
    cout << "Async I/O time: " << async_time << " microseconds" << endl;
    cout << "Percent speedup for asynchronous IO: " << (1 - (async_time / (double)sync_time)) * 100 << "%" << endl;

    return 0;
}