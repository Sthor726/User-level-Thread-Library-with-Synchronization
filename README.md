# User-level-Thread-Library-with-Synchronization


###Lock vs Spinlock###
- Which lock provides better performance in your testing? Why do you think that is?
- How does the size of a critical section affect the performance of each type of lock? Explain with results.
uthread is a uniprocessor user-thread library. How might the performance of the lock types be affected if they could be used in parallel by a multi-core system?
- Are there any other interesting results from your testing?



###Asynchronus vs Synchronus IO###

- Which I/O type provides better performance in your testing? Why do you think that is?
Generally, asynchronos IO performed better in our testing. Instead of threads blocking during IO operations, the instead yield the processor while they are waiting. This allows for more work to be done when there are large amounts of IO and other available threads to perform their own work. 

- How does the amount of I/O affect the performance of each type of I/O? Explain with results.
As you can see from the first 2 tests with 50 threads. Asynchronos IO provides a greater speedup when there is a larger message length. When there is more IO, yielding provides a greater benefit, since there is more time spent waiting on our IO operations to finish.

- How does the amount of other available thread work affect the performance of each type of I/O?
Explain with results.
Asynchronos IO performs much better when there are many available threads, while synchronos IO performs better when there are few threads. You can see this in the tests, where tests 3 and 4 show poor asynchronos performance when there are only 3 threads, while in tests 1 and 2 it provides a speedup. When there are few threads, yielding while waiting for IO operations doesn't provide much benefit and instead adds the overhead of context switching to a new thread.

Running tests...
=============================
 Message Length: 5000 bytes
 Number of Threads: 50
=============================
Sync I/O time: 159265 microseconds
Async I/O time: 71773 microseconds
Percent speedup for asynchronous IO: 54.9349%
=============================
 Message Length: 10 bytes
 Number of Threads: 50
=============================
Sync I/O time: 139642 microseconds
Async I/O time: 72400 microseconds
Percent speedup for asynchronous IO: 48.1531%
=============================
 Message Length: 5000 bytes
 Number of Threads: 3
=============================
Sync I/O time: 8202 microseconds
Async I/O time: 16976 microseconds
Percent speedup for asynchronous IO: -106.974%
=============================
 Message Length: 10 bytes
 Number of Threads: 3
=============================
Sync I/O time: 7973 microseconds
Async I/O time: 16155 microseconds
Percent speedup for asynchronous IO: -102.621%