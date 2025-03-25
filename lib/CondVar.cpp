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

    lock.unlock();
    if (uthread_suspend(uthread_self()) == -1)
    {
        std::cout << "Error: uthread_suspend" << std::endl;
        assert(false);
    }
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
