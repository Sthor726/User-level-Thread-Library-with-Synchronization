#include "TCB.h"
#include <cassert>
#include <ucontext.h>


// private:
// 	int _tid;               // The thread id number.
// 	Priority _pr;           // The priority of the thread (Red, orange or green)
// 	int _quantum;           // The time interval, as explained in the pdf.
// 	State _state;           // The state of the thread
// 	char* _stack;           // The thread's stack


TCB::TCB(int tid, Priority pr, void *(*start_routine)(void *arg), void *arg, State state)
{
    _tid = tid;
    _state = state;
    _quantum = 0;

    getcontext(&_context);
    
    _context.uc_stack.ss_sp = malloc(STACK_SIZE);
    if (_context.uc_stack.ss_sp == nullptr)
    {
        perror("Failed to allocate stack");
        exit(1);
    }
    _stack = (char *)_context.uc_stack.ss_sp;
    _context.uc_stack.ss_size = STACK_SIZE;
    _context.uc_stack.ss_flags = 0;
    
    // Set uc_link to a cleanup function to prevent crashes
    _context.uc_link = nullptr; // Change this if needed

    // Call stub() as a wrapper function to properly call start_routine
    makecontext(&_context, (void (*)())stub, 2, start_routine, arg);
}

TCB::~TCB()
{
    free (_stack);
}

void TCB::setState(State state)
{
    _state = state;
}

State TCB::getState() const
{
    return _state;

}

int TCB::getId() const
{
    return _tid;
}

void TCB::increaseQuantum()
{
    _quantum++;
}

int TCB::getQuantum() const
{
    return _quantum;
}


void TCB::increaseLockCount()
{
    // TODO
}

void TCB::decreaseLockCount()
{
    // TODO
}

int TCB::getLockCount()
{
    // TODO
    return 0; // return statement added only to allow compilation (replace with correct code)
}

void TCB::increasePriority()
{
    // TODO
}

void TCB::decreasePriority()
{
    // TODO
}
