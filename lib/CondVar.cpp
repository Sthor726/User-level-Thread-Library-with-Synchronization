#include "CondVar.h"
#include "uthread_private.h"
#include "uthread.h"
#include <cassert>

CondVar::CondVar()
{
    // Nothing to do
}

// Release the lock and block this thread atomically. Thread is woken up when
// signalled or broadcasted
void CondVar::wait(Lock &lock)
{
    // no way to  assert lock is held
    TCB* currentTCB = running;

    queue.push(currentTCB);

    uthread_suspend(uthread_self());
    lock.unlock();
    uthread_yield();
    lock.lock();
}

// Wake up a blocked thread if any is waiting
void CondVar::signal()
{
    // TODO
    if (!queue.empty())
    {
        TCB* tcb = queue.front();
        queue.pop();
        uthread_resume(tcb->getId());
    }
}

void CondVar::broadcast()
{
    // TODO
    while (!queue.empty())
    {
        TCB* tcb = queue.front();
        queue.pop();
        uthread_resume(tcb->getId());
    }
}
