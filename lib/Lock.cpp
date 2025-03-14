#include "Lock.h"
#include "uthread_private.h"
#include <cassert>

Lock::Lock() : held(false)
{
    // Nothing to do
}

// Attempt to acquire lock. Grab lock if available, otherwise thread is
// blocked until the lock becomes available
void Lock::lock()
{
    disableInterrupts();
    /*
    Code from Lecture Notes:
    if (value == BUSY){
    waiting.add(myTCB);
    myTCB->state = WAITING;
    next = readyList.remove();
    switch(myTCB, next);
    myTCB->state = RUNNING;
    }
    else{
    value = BUSY;
    }
    */

    if (held)
    {
        // Add the thread to the waiting queue
        // change state to WAITING
        entrance_queue.push(running);
        changeState(running, State::WAITING);

        // Switch to the next thread
        // Remove from ready queue, switch and change state to RUNNING - switchThreads() does all the work ?
        switchThreads();
    }
    else
    {
        held = true;
    }
}

// Unlock the lock. Wake up a blocked thread if any is waiting to acquire the
// lock and hand off the lock
void Lock::unlock()
{
    disableInterrupts();
    /*
    Code from Lecture Notes:
    if (!waiting.Empty()) {
    next = waiting.remove();
    next->state = READY;
    readyList.add(next);
    } else {
    value = FREE;
    }
    */

    if (!entrance_queue.empty())
    {
        TCB *next = entrance_queue.front();
        entrance_queue.pop();

        changeState(next, State::READY);

        addToReady(next);
    }
    else
    {
        held = false;
    }
   enableInterrupts();

}

// Unlock the lock while interrupts have already been disabled
// NOTE: This function should NOT be used by user code. It is only to be used
//       by uthread library code
void Lock::_unlock()
{

    // Is this the same ??
    if (!entrance_queue.empty())
    {
        TCB *next = entrance_queue.front();
        entrance_queue.pop();

        changeState(next, State::READY);

        addToReady(next);
    }
    else
    {
        held = false;
    }
}

// Let the lock know that it should switch to this thread after the lock has
// been released (following Mesa semantics)
void Lock::_signal(TCB *tcb)
{
    /*
    Mesa Semantics:
    – Signal puts waiter on ready list
    – Signaller keeps lock and processor
    */
    disableInterrupts();
    TCB* waiter = entrance_queue.front();
    entrance_queue.pop();
    addToReady(waiter);
    // Thats it ??
    enableInterrupts();
}
