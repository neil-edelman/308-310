/** 2014 Neil Edelman

 (Useless obselete) threads library.

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free */
#define _POSIX_SOURCE /* need fileno from stdio */
#include <stdio.h>  /* fprintf */

#include <fcntl.h>    /* posix */
#include <ucontext.h> /* deprecated posix */
#include <sys/types.h>/* posix? */
#include <sys/time.h> /* posix? */

#include "Threads.h"

enum Run { RUNNING, BLOCKED, EXIT };

/* constants */
static const char *programme   = "Threads";
static const char *year        = "2014";
static const int versionMajor  = 0;
static const int versionMinor  = 1;

struct Threads {
	int ms;
	struct Thread    *active;
	struct Thread    *first_blocked;
	struct Thread    *first_exit;
	struct Semaphore *first_semaphore;
};

/* "The table itself needs to be directly addressable and can have an upper
 limit. So this naturally lends to an array type structure. The thread ID is an
 index into this table . . . mythread_control_block"
 mmmmm; instead of a thread id, let's have a pointer directly to the thread,
 then we could grow and shrink dynamically; the thread_id becomes the pointer
 itself, saving the headache of having two unique ids; this helps with types,
 too; 'everything is an int' . . . why obfuscate? also, encapsulation is good */
struct Thread {
	struct Thread *next;
	ucontext_t context;
	int time;
	enum Run run;
	char name[64];
};

static const int name_size = sizeof((struct Thread *)0)->name / sizeof(char);

struct Semaphore {
	struct Semaphore *next; /* for cleanup */
	struct Thread *thread;
	int count;
};

static const int default_quantum_ms = 100;

/* global to make idempotent */
struct Threads *threads;

/* private */
static void thread_print(FILE *out, const struct Thread *t, const char *which);
static void usage(void);

/* public */

/** constructor, eg mytread_init()
 "This function initializes all the global data structures for the thread
 system. Mythreads package will maintain many global data structures such as
 the runqueue, a table for thread control blocks. It is your responsibility to
 define the actual data structures. One of the constraints you have the need to
 accommodate ucontext_t inside the data structures."
 @return true or false if the object couldn't be created */
int Threads(void) {
	if(threads) {
		fprintf(stderr, "Threads: threads library initialised already.\n");
		return 0;
	}
	if(!(threads = malloc(sizeof(struct Threads)))) {
		perror("Threads constructor");
		Threads_();
		return 0;
	}
	threads->ms              = default_quantum_ms;
	threads->active          = 0;
	threads->first_blocked   = 0;
	threads->first_exit      = 0; /* hmm, memory! */
	threads->first_semaphore = 0;
	fprintf(stderr, "Threads: new, #%p.\n", (void *)threads);

	return -1;
}

/** destructor */
void Threads_(void) {
	if(!threads) {
		fprintf(stderr, "Threads~: threads library not initialised.\n");
		return;
	}
	fprintf(stderr, "~Threads: erase, #%p.\n", (void *)threads);
	free(threads);
	threads = 0;
}

/** "This function creates a new thread. It returns an integer that points to
 the thread control block that is allocated to the newly created thread in the
 thread control block table. If the library is unable to create the new thread,
 it returns -1 and prints out an error message. This function is responsible
 for allocating the stack and setting up the user context appropriately. The
 newly created thread starts running the threadfunc function when it starts.
 The threadname is stored in the thread control block and is printed for
 information purposes. A newly created thread is in the RUNNABLE state when it
 is inserted into the system. Depending on your system design, the newly
 created thread might be included in the runqueue."
 @return the created thread or null */
struct Thread *ThreadsCreate(const char *name,
							 void (*func)(void),
							 const int stack) {
	if(!name || !func || stack <= 0) {
		fprintf(stderr, "Threads::create: invalid.\n");
		return 0;
	}
	
	return 0;
}

/** "This function is called at the end of the function that was invoked by the
 thread. This function will remove the thread from the runqueue (i.e., the
 thread does not need to run any more). However, the entry in the thread
 control block table could be still left in there. However, the state of the
 thread control block entry should be set to an EXIT state."
 @param t the thread to exit */
void ThreadsExit(struct Thread *t) {
}

/** "In MyThreads, threads are created by the mythread_create() function. The
 mythread_create() function needs to be executed by the main thread (the
 default thread of process – the one running before MyThreads created any
 threads). Even after all threads are created, the main thread will still keep
 running. To actually run the threads, you need to run the runthreads(). The
 runthreads() switches control from the main thread to one of the threads in
 the runqueue.
 
 "In addition to switching over to the threads in the runqueue, the
 runthreads() function activates the thread switcher. The thread switcher is an
 interval timer that triggers context switches every quantum nanoseconds." */
void ThreadsRun(void) {
	/* mmm */
}

/** "This function creates a semaphore and sets its initial value to the given
 parameter. The mythread_init() function would have initialized the semaphores
 table and set the total number of active semaphore count to zero. You insert
 an entry into this table. Each entry of this table will be a structure that
 defines the complete state of the semaphore. It should also have a queue to
 hold the threads that will be waiting on the semaphore."
 @param value semaphore value >= 0
 @return semaphore
 */
struct Semaphore *ThreadsSemaphore(const int value) {
	return 0;
}

/** "void destroy_semaphore(int semaphore); This function removes a semaphore
 from the system. A call to this function while threads are waiting on the
 semaphore should fail. That is the removal process should fail with an
 appropriate error message. If there are no threads waiting, this function will
 proceed with the removal after checking whether the current value is the same
 as the initial value of the semaphore. If the values are different, then a
 warning message is printed before the semaphore is destroyed." */
void ThreadsSemaphore_(struct Semaphore **sptr) {
	
}

/** "When a thread calls this function, the value of the semaphore is
 decremented. If the value goes below 0, the thread is put into a WAIT state.
 That means calling thread is taken out of the runqueue if the value of the
 semaphore goes below 0."
 @param s a valid semaphore */
void ThreadsSemaphoreDown(struct Semaphore *s) {
	if(!s || !s->thread /*|| !s->thread->status == */);
	s->count--;
	if(s->count < 0) {
		/* enqueue(s-­>queue, CurrentThread);
		 thread_switch(); */
	}
}

/** "When a thread calls this function, the value of the semaphore is
 incremented. If value is not greater than 0, then we should have at least one
 thread waiting on it. The thread at the top of the wait queue associated with
 the semaphore is dequeued from the wait queue and enqueued in the runqueue.
 The state of the thread is changed to RUNNABLE."
 @param s a valid semaphore */
void ThreadsSemaphoreUp(struct Semaphore *s) {
	s->count++;
	if(s->count <= 0) {
		/*enqueue(runqueue, dequeue(s-­‐>queue)); */
	}
}

/* "MyThreads has preemptive multithreading, which is implemented through
 signal based interrupts. You need to block the signals (more on the exact
 signal to block later) while manipulating the semaphore internal parameters.
 Remember to enable the signal based interrupts as soon as possible. Otherwise,
 the multithreading process will stop working!
 "Notice that the semaphore is denoted by an integer. It is actually an index
 into the active semaphore table maintained by the MyThreads library. The
 semaphore_wait() function needs to access the record corresponding to the
 given semaphore and then manipulate its contents." */

/** "void mythread_state(); This function prints the state of all threads that
 are maintained by the library at any given time. For each thread, it prints
 the following information in a tabular form: thread name, thread state (print
 as a string RUNNING, BLOCKED, EXIT, etc), and amount of time run on CPU." */
void ThreadsPrintState(FILE *out) {
	struct Thread *t;
	int fl, file;

	/* don't trust the file: stdio_ext.h: int __fwritable (FILE *stream) */
	if(!out) return;
	if((file = fileno(out)) == -1) {
		perror("fileno");
		return;
	}
	if((fl = fcntl(file, F_GETFL)) == -1) {
		perror("fcntl");
		return;
	}
	fl &= O_ACCMODE;
	if(fl != O_WRONLY && fl != O_RDWR) {
		/* on my computer, stdout is O_RDWR, so this check was a waste of time */
		fprintf(stderr, "Threads::PrintState: passed read only stream.\n");
		return;
	}
	/* whatever */

	if(!threads) {
		fprintf(out, "Threads is not active.\n");
		return;
	}
	fprintf(out, "unique address\tthread state\tcpu time\tname\n");
	if((t = threads->active))                            thread_print(out, t, "RUNNING");
	for(t = threads->first_blocked; t != 0; t = t->next) thread_print(out, t, "BLOCKED");
	for(t = threads->first_exit;    t != 0; t = t->next) thread_print(out, t, "EXITED");
	fprintf(out, "---\n");
}

/** "void set_quantum_size(int quantum); Sets the quantum size of the round
 robin scheduler. The round robin scheduler is pretty simple it just picks the
 next thread from the runqueue and appends the current to the end of the
 runqueue. Then it switches over to the new thread."
 @param ms the number of ms to run each thread */
void ThreadsSetQuantum(const int ms) {
	if(!threads || ms <= 0) return;
	threads->ms = ms;
}

/** @return version * 100 + minor */
int ThreadsGetVersion(void) { return versionMajor * 100 + versionMinor; }

/** prints useless stuff so I wouldn't get "defined but not used" */
void ThreadsPrintInfo(void) { usage(); }

/* private */

/** helper for print */
static void thread_print(FILE *out, const struct Thread *t, const char *which) {
	fprintf(out, "#%p\t%s\t%d\t%s\n", (void *)t, which, t->time, t->name);
}

/** prints command-line help */
static void usage(void) {
	fprintf(stderr, "Usage: %s\n", programme);
	fprintf(stderr, "Version %d.%d.\n\n", versionMajor, versionMinor);
	fprintf(stderr, "%s %s Neil Edelman\n", programme, year);
	fprintf(stderr, "This program comes with ABSOLUTELY NO WARRANTY.\n\n");
}
