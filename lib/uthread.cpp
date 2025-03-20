#include "uthread.h"
#include "uthread_private.h"
#include "TCB.h"
#include <vector>
#include <queue>
#include <ucontext.h>
#include <stdlib.h>
#include <map>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <cstdlib>

using namespace std;

#define NUM_OF_QUEUE 3
#define RED_Q 0
#define ORANGE_Q 1
#define GREEN_Q 2
#define FAIL -1
#define SUCCESS 0
#define MAIN_THREAD 0
#define MICRO_TO_SECOND 1000000
#define THREAD_ERROR "thread library error: "
#define SYS_ERROR "system error: "
#define NOT_FOUND_ID 0
#define SUSPEND_MAIN 1
#define SET_TIME_ERROR 2
#define WRONG_INPUT 3
#define SIGNAL_ACTION_ERROR 4
#define TOO_MANY_THREADS 5
#define BOOST_PRIORITY_FREQUENCY 10

static vector<TCB *> ready[NUM_OF_QUEUE]; // An array of size 3 of queues, which each represents a queue of priority.
static map<int, TCB *> _threads; // All threads together
static int yield_count = 0;

// Finished queue entry type
typedef struct finished_queue_entry
{
	TCB *tcb;    // Pointer to TCB
	void *result; // Pointer to thread result (output)
} finished_queue_entry_t;

// Join queue entry type
typedef struct join_queue_entry
{
	TCB *tcb;           // Pointer to TCB
	int waiting_for_tid; // TID this thread is waiting on
} join_queue_entry_t;



static deque<TCB *> ready_queue;
static deque<TCB *> blocked_queue;
static deque<join_queue_entry_t *> join_queue;
static deque<finished_queue_entry_t> finished_queue;
//static runningTCB *currentTCB;



struct itimerval timer;
struct sigaction sa;



static int mostRecentTid = 0;
static int total_quantums = 0;



/**
 * function responsible for printing each kind of error
 */
void printError(int type, string pre)
{
    // TODO
}

/*
 * This function finds & returns the next smallest nonnegative integer not already taken by an existing thread,
 * or -1 if there are no available free ids.
 */
int getNextId()
{
    // TODO
    return 0;  // return statement added only to allow compilation (replace with correct code)
}

/**
 * set time and check if set is done correctly
 */
static void setTime()
{
    // TODO
}

/*
 * Translates the priority of the given tid from Enum to int.
 * Returns the int value of the priority.
 */
int translatePriority(int tid)
{
    // TODO
    return 0;  // return statement added only to allow compilation (replace with correct code)
}


/*
 * Moves any threads that have joined on tid to the ready queue
 */
void moveFromJoinToReady(int tid)
{
    // TODO
}

// Switch to the thread provided
void switchToThread(TCB *next)
{
    // TODO
}


/*=================================================================================================
 * ======================================Library Functions=========================================
 * ================================================================================================
 */



static void boost_priorities()
{
    // TODO
}

// Internal handler for increasing a thread's priority
// NOTE: Assumes interrupts are already disabled
static void _uthread_increase_priority(TCB *tcb)
{
    // TODO 
}

/* Increase the thread's priority by one level */
int uthread_increase_priority(int tid)
{
    // TODO
    return SUCCESS;  // return statement added only to allow compilation (replace with correct code)
}

// Internal handler for decreasing a thread's priority
// NOTE: Assumes interrupts are already disabled
static void _uthread_decrease_priority(TCB *tcb)
{
    // TODO
}

/* Decrease the thread's priority by one level */
int uthread_decrease_priority(int tid)
{
    // TODO
    return SUCCESS;  // return statement added only to allow compilation (replace with correct code)
}



static void startInterruptTimer()
{
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = timer.it_interval.tv_usec;
	
	//cout << "Starting timer" << uthread_self() << "\n";
	
	if (setitimer(ITIMER_VIRTUAL, &timer, NULL) == -1) {
		perror("setitimer failed");
		exit(EXIT_FAILURE);
	}
	//get_time_slice();
}

void disableInterrupts()
{
	// cout << "Disabling interrupts " << uthread_self() << "\n";
	//get_time_slice()
	if (sigprocmask(SIG_BLOCK, &sa.sa_mask, nullptr) == -1) {
		perror("sigprocmask failed");
		exit(EXIT_FAILURE);
	}
}

void enableInterrupts()
{
	// cout << "Enabling interrupts " << uthread_self() << "\n";
	//get_time_slice();
	if (sigprocmask(SIG_UNBLOCK, &sa.sa_mask, nullptr) == -1) {
		perror("sigprocmask failed");
		exit(EXIT_FAILURE);
	}

}

void handler(int signum)
{
	// disableInterrupts();
	//cout << "Timer expired! Yielding thread " << uthread_self() << endl;
	
	uthread_yield();
	enableInterrupts();
}

void addToReadyQueue(TCB *tcb)
{
	ready_queue.push_back(tcb);
}

TCB *popFromReadyQueue()
{
	if(!ready_queue.empty()){
		TCB *ready_queue_head = ready_queue.front();
		ready_queue.pop_front();
		return ready_queue_head;
	}else{
		return nullptr;
	}

}

TCB* removeFromReadyQueue(int tid)
{
	for (auto iter = ready_queue.begin(); iter != ready_queue.end(); ++iter)
	{
		if (tid == (*iter)->getId())
		{
			TCB *tcb = (*iter);
			ready_queue.erase(iter);
			return tcb;
		}
	}
	return nullptr;
}

TCB* removeFromBlockQueue(int tid)
{
	for (auto iter = blocked_queue.begin(); iter != blocked_queue.end(); ++iter)
	{
		if (tid == (*iter)->getId())
		{
			TCB* tcb = (*iter);
			blocked_queue.erase(iter);
			return tcb;
		}
	}
	return nullptr;
}

TCB* removeFromJoinQueue(int tid)
{
	for (auto iter = join_queue.begin(); iter != join_queue.end(); ++iter)
	{
		if (tid == (*iter)->tcb->getId())
		{
			TCB* tcb = (*iter)->tcb;
			join_queue.erase(iter);
			return tcb;
		}
	}
	return nullptr;
}

TCB* removeFromFinishedQueue(int tid)
{
	for (auto iter = finished_queue.begin(); iter != finished_queue.end(); ++iter)
	{
		if (tid == (*iter).tcb->getId())
		{
			TCB* tcb = (*iter).tcb;
			finished_queue.erase(iter);
			return tcb;
		}
	}
	return nullptr;
}


// asume that inerputs are disabled before we enter switch threads
void switchThreads()
{
	disableInterrupts();
    int currentThread = uthread_self();
	volatile int flag = 0;
    int ret_val = getcontext(&currentTCB->tcb->_context);
	
	if (flag == 0) {
		flag = 1;
		// we are leaving this thread
        currentTCB->tcb->increaseQuantum();
        total_quantums++;

        TCB* new_thread = popFromReadyQueue();
        if (new_thread == nullptr) {
			cout << "No more threads in ready queue\n";
            enableInterrupts();
            return;
        }

        currentTCB = new runningTCB{new_thread->getId(), new_thread};
        new_thread->setState(State::RUNNING);

		enableInterrupts(); // enable interupts before switching context
		startInterruptTimer();
        setcontext(&new_thread->_context); 
		// should not ever reach here because setcontext does not return
		perror("setcontext failed");
		exit(EXIT_FAILURE);
	}

	// this code is running in the new thread/when we switch back to this thread
	cout << "Switched back to thread " << uthread_self() << endl;
	//startInterruptTimer();
	enableInterrupts();
}

void stub(void *(*start_routine)(void *), void *arg)
{
	enableInterrupts();

	if (!start_routine) {
        std::cerr << "start_routine is NULL" << std::endl;
        exit(EXIT_FAILURE);
    }

    void *retval = start_routine(arg); // Store the return value

    uthread_exit(retval);
}

int uthread_init(int quantum_usecs)
{

	ready_queue.clear();
	join_queue.clear();
	finished_queue.clear();


	timer.it_interval.tv_usec = quantum_usecs;
	timer.it_value.tv_usec = quantum_usecs;
	timer.it_interval.tv_sec = 0;
	timer.it_value.tv_sec = 0;

	sa.sa_handler = handler;
	sa.sa_flags = 0;
	sigaddset(&sa.sa_mask, SIGVTALRM);
	

	if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
		perror("sigaction failed");
		exit(EXIT_FAILURE);
	}

	TCB *thread = new TCB(mostRecentTid++, GREEN, nullptr, nullptr, RUNNING);
	currentTCB = new runningTCB{thread->getId(), thread};

	startInterruptTimer();

	return 0;
}

int uthread_create(void *(*start_routine)(void *), void *arg)
{
	//std::cout<< "In create\n";

	disableInterrupts();
	TCB *new_thread = new TCB(mostRecentTid++, GREEN, start_routine, arg, READY);
	addToReadyQueue(new_thread);
	enableInterrupts();
	return new_thread->getId();
}

int uthread_join(int tid, void **retval)
{
	//std::cout<< "In join\n";

	disableInterrupts();
	if(retval == nullptr) return 0;


	for(auto &entry : finished_queue){
		if (entry.tcb->getId() == tid){
			//cout << "checking for new threads in finished queue \n";
			// cout << entry.tcb->getId() << "\n";
			*retval = entry.result;
			return 0;
		} 
	}
	// cout << "child not already finished\n";
	join_queue_entry_t *j_entry = new join_queue_entry_t{currentTCB->tcb, tid};
	join_queue.push_back(j_entry);

	finished_queue_entry_t *waiting_on = nullptr;
	// std::cout << "waiting on child\n";
	while(waiting_on == nullptr){
		// cout << "waiting, switching to new thread\n";
		uthread_yield();


		for(auto &entry : finished_queue){
			//cout << "checking for new threads in finished queue \n";
			//cout << entry.tcb->getId() << "\n";
			if (entry.tcb->getId() == tid) waiting_on = &entry;
		}
	}

	if(retval != nullptr){
		for(auto &entry : finished_queue){
			if (entry.tcb->getId() == tid) *retval = entry.result;
		}
	}
	enableInterrupts();

	return 0;
}

int uthread_yield(void)
{
	disableInterrupts();
	addToReadyQueue(currentTCB->tcb);

	
	currentTCB->tcb->setState(READY);
	enableInterrupts();
	switchThreads();

	startInterruptTimer();
	enableInterrupts();
	return 0;
}

void uthread_exit(void *retval)
{
	// cout << "in exit\n";
	disableInterrupts();
	if(uthread_self() == 0){
		exit(0);
	}

	for(int i = 0; i < join_queue.size(); i++){
		TCB* entry = join_queue.front()->tcb;
		join_queue.pop_front();
		addToReadyQueue(entry);
	}

	removeFromReadyQueue(uthread_self());
	finished_queue_entry_t f_entry;
	f_entry.tcb = currentTCB->tcb;
	f_entry.result = retval;
	finished_queue.push_back(f_entry);

	enableInterrupts();
	switchThreads();

}

int uthread_suspend(int tid)
{
	disableInterrupts();
	TCB* thread_to_suspend;
	thread_to_suspend = removeFromReadyQueue(tid);
	if(thread_to_suspend != nullptr){
		blocked_queue.push_back(thread_to_suspend);
		return 0;
	}
	thread_to_suspend = removeFromJoinQueue(tid);
	if(thread_to_suspend != nullptr){
		blocked_queue.push_back(thread_to_suspend);
		return 0;
	}
	thread_to_suspend = removeFromFinishedQueue(tid);
	if(thread_to_suspend != nullptr){
		blocked_queue.push_back(thread_to_suspend);
		return 0;
	}
	enableInterrupts();
	return -1;
}

int uthread_resume(int tid)
{
	disableInterrupts();
	for (TCB* tcb : ready_queue){
		if (tcb->getId() == tid){
			removeFromBlockQueue(tid);
			ready_queue.push_back(tcb);
			tcb->setState(State::READY);
		}
	}
	enableInterrupts();
	return 0;
}

int uthread_once(uthread_once_t *once_control, void (*init_routine)(void))
{
	disableInterrupts();

	if(once_control->execution_status == UTHREAD_ONCE_EXECUTED){
		return 1;
	}

	once_control->execution_status = UTHREAD_ONCE_EXECUTED;
	init_routine();
	enableInterrupts();
	return 0;
}

int uthread_self()
{
	return currentTCB->tid;
}

int uthread_get_total_quantums()
{
    return total_quantums;
}

int uthread_get_quantums(int tid)
{
	if (tid == currentTCB->tid) {
        return currentTCB->tcb->getQuantum();
    }

    for (TCB* tcb : ready_queue) {
        if (tcb->getId() == tid) {
            return tcb->getQuantum();
        }
    }

    for (TCB* tcb : blocked_queue) {
        if (tcb->getId() == tid) {
            return tcb->getQuantum();
        }
    }

    return -1; // Thread not found
}

void get_time_slice(){
    getitimer(ITIMER_VIRTUAL, &timer);
    cout << timer.it_value.tv_sec << "s " << timer.it_value.tv_usec << "µs is the timer value\n";
}