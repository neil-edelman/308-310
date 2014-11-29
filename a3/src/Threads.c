/** 2014 Neil Edelman

 (Obselete) threads library.

 @bugs -std=c89,99,etc, does NOT work with Linux's ucontext.h
 @author Neil
 @version 1.1
 @since 2014 */

#ifdef __APPLE__
#ifndef _XOPEN_SOURCE
#define HOME
#define _XOPEN_SOURCE /* MacOS (doesn't work anyway; nice try) */
#endif
#endif

#include <stdio.h>    /* fprintf */
#include <stdlib.h>   /* malloc free */
#include <string.h>   /* strncpy memset */

#include <unistd.h>   /* sleep */
#include <sys/time.h> /* itimerval */
#include <signal.h>   /* sigaction */

#include <fcntl.h>    /* posix */
#include <sys/types.h>/* posix */
#include <ucontext.h> /* obsolete posix2001; removed from posix2008 */

#include "Threads.h"

enum Run { R_RUNNING, R_BLOCKED, R_EXIT };

/* constants */
static const int default_us = 500;

struct Threads {
	struct sigaction action;
	struct itimerval timer;
	ucontext_t       this_context;
	struct sigaction old_action;
	struct itimerval old_timer;
	struct Thread    *active;
	struct Thread    *first_running;
	struct Thread    *first_blocked;
	struct Thread    *first_exit;
	struct Semaphore *first_semaphore;
};

/* doubly-linked: I can't get it to work */
struct Thread {
	struct Thread *prev, *next;
	ucontext_t context, destroy;
	char name[64];
	char *stack;
	int stack_size;
	enum Run run;
	int  time;
};
static const int name_size = sizeof((struct Thread *)0)->name / sizeof(char);

struct Semaphore {
	struct Semaphore *prev, *next;
	int count;
	int count_start;
	struct Thread *first_waiting;
};

/* global to make idempotent */
static struct Threads *threads;

/* private */
static void thread_print(FILE *out, const struct Thread *t, const char *which);
static void timer_callback(int signal);
static void destroy_callback(void);

/* public */

/** constructor; eg mytread_init()
 @return an object or a null pointer if the object couldn't be created */
struct Threads *Threads(void) {
	if(threads) return threads;
	if(!(threads = malloc(sizeof(struct Threads)))) {
		perror("Threads constructor");
		Threads_();
		return 0;
	}
	/* set up the signal callback thing */
	memset(&threads->action, 0, sizeof(struct sigaction));
	threads->action.sa_handler = &timer_callback;
	sigemptyset(&threads->action.sa_mask);
	threads->action.sa_flags   = SA_RESTART;
	/* clear timing */
	memset(&threads->timer, 0, sizeof(struct itimerval));
	/* active thread -- active is neccessarily a thread on the list first_running */
	threads->active          = 0;
	/* set up the lists (empty) */
	threads->first_running   = 0;
	threads->first_blocked   = 0;
	threads->first_exit      = 0;
	threads->first_semaphore = 0;
	/* set the default timing */
	ThreadsSetQuantum(default_us);

	fprintf(stderr, "Threads: new, #%p.\n", (void *)threads);

	return threads;
}

/** global destructor */
void Threads_(void) {
	struct Thread *t, *next;

	if(!threads) return;
	for(t = threads->first_running; t; t = next) {
		next = t->next;
		fprintf(stderr, "~Threads: terminating thread #%p while it was running.\n", (void *)t);
		free(t);
	}
	for(t = threads->first_blocked; t; t = next) {
		next = t->next;
		fprintf(stderr, "~Threads: terminating thread #%p while it was blocked.\n", (void *)t);
		free(t);
	}
	for(t = threads->first_exit; t; t = next) {
		next = t->next;
		fprintf(stderr, "~Threads: freeing thread #%p from graveyard.\n", (void *)t);
		free(t);
	}
	fprintf(stderr, "~Threads: erase, #%p.\n", (void *)threads);
	free(threads);
	threads = 0;
}

/** "A newly created thread is in the RUNNABLE state when it is inserted into
 the system." Pointers are your friends.
 @param name  name of your thread (bounded by name_size - 1)
 @param func  the fuction to start
 @param stack stack byte size
 @return the created thread or null */
struct Thread *ThreadsCreate(const char *name, void (*func)(int), const int arg, const int stack_size) {
	struct Thread *t;

	if(!threads || !name || !func || stack_size <= 0) {
		fprintf(stderr, "Threads::create: invalid.\n");
		return 0;
	}
	if(!(t = malloc(sizeof(struct Thread) + sizeof(char) * stack_size))) {
		perror("Thread::create constructor");
		return 0;
	}
	t->prev       = 0;
	t->next       = 0;
	strncpy(t->name, name, name_size - 1);
	t->name[name_size - 1] = '\0';
	t->stack      = (char *)(t + 1);
	t->stack_size = stack_size;
	t->run        = R_RUNNING;
	t->time       = 0; /* fixme: this is unused */

	/* create thread context out of func */
	if(getcontext(&t->context) == -1
	   || getcontext(&t->destroy) == -1) {
		perror("getcontext");
		free(t);
		return 0;
	}
	t->context.uc_stack.ss_sp   = t->stack;
	t->context.uc_stack.ss_size = t->stack_size;
	t->context.uc_link          = &t->destroy;
	t->destroy.uc_stack.ss_sp   = t->stack;
	t->destroy.uc_stack.ss_size = t->stack_size;
	t->destroy.uc_link          = 0;
	makecontext(&t->destroy, destroy_callback, 0);
	/* this is so messy; makecontext is prototyped incorrectly; it should be
	 (void (*)())) */
	makecontext(&t->context, (void (*)(void))func, 1, arg);

	if((t->next = threads->first_running)) t->next->prev = t;
	threads->first_running = t;

	fprintf(stderr, "Thread: new, \"%s\" #%p (#%p next #%p.)\n", name, (void *)t, (void *)&t->context, (void *)t->context.uc_link);

	return t;
}

/** "The runthreads() switches control from the main thread to one of the
 threads in the runqueue."
 @return true if there are threads which are waiting */
int ThreadsRun(void) {
	struct Thread *t, *next, *new;

	/* we have at least one thread */
	if(!threads || !(t = threads->first_running)) return 0;

	/* pick off the exited threads */
	fprintf(stderr, "Threads::run: list ");
	do {
		fprintf(stderr, "<%s:%s>", t->name, t->run == R_RUNNING ? "running" : "cleanup");
		next = t->next;
		if(t->run == R_RUNNING) continue;
		/* remove */
		if(threads->first_running == t) threads->first_running = t->next;
		if(t->prev) t->prev->next = t->next;
		if(t->next) t->next->prev = t->prev;
		/* insert */
		t->prev = 0;
		if((t->next = threads->first_exit)) t->next->prev = t;
		threads->first_exit = t;
	} while((t = next));
	fprintf(stderr, ".\n");

	/* cleanup has not got any threads left? */
	if(!(threads->first_running)) return 0;

	/* set up sigaction and timer */
	if(sigaction(SIGALRM, &threads->action, &threads->old_action) == -1
	   || setitimer(ITIMER_REAL, &threads->timer, &threads->old_timer) == -1) {
		perror("Threads::run");
		abort(); /* fixme: undo? */
	}

	/* the swap function timer_callback, restores the context upon exiting */
	new = threads->active = threads->first_running;
	fprintf(stderr, "Threads::run: swap(%s, %s#%p).\n", "parent", new->name, (void *)new);
	swapcontext(&threads->this_context, &new->context);

	return -1;
}

/** "the state of the thread control block entry should be set to an EXIT
 state." This basically force-exits. Note: allowing threads to exit gracefully
 is okay
 @param       the thread */
void ThreadsExit(struct Thread *t/*exit*/) {
	/*struct Thread *prev, *thiz;*/

	if(!threads) return;
#if 0 /* singly linked list: more stable (haven't work out bugs,) but slower, O(n) vs O(1) */
	/* set up thiz and prev */
	for(thiz = threads->first_running, prev = 0; thiz; prev = thiz, thiz = thiz->next) {
		if(thiz == exit) break;
	}
	if(!thiz) {
		fprintf(stderr, "Threads::exit: #%p does not match any in the active queue; ignoring.\n", (void *)exit);
		return;
	}
	/* remove t from active list */
	if(prev) prev->next             = thiz->next;
	else     threads->first_running = thiz->next;
	/* insert t to the exit list */
	thiz->next          = threads->first_exit;
	threads->first_exit = thiz;
#else
	if(threads->active) {
		fprintf(stderr, "Threads::exit: called when Threads::run active. Ignored.\n");
		return;
	}
	/* remove */
	if(     threads->first_running == t) threads->first_running = t->next;
	else if(threads->first_blocked == t) threads->first_blocked = t->next;
	else if(threads->first_exit    == t) return;
	if(t->prev) t->prev->next = t->next;
	if(t->next) t->next->prev = t->prev;
	/* insert */
	t->prev = 0;
	t->next = threads->first_exit;
	t->next->prev = threads->first_exit = t;
#endif

	fprintf(stderr, "Threads::exit: #%p has (force) exited.\n", (void *)t);
}

/** "This function creates a semaphore and sets its initial value to the given
 parameter."
 @param value semaphore value >= 0
 @return semaphore */
struct Semaphore *ThreadsSemaphore(const int value) {
	struct Semaphore *s;
	/*sigset_t sset, oldset;*/

	if(!threads || value < 0) {
		fprintf(stderr, "Threads::semaphore: invalid.\n");
		return 0;
	}

	/* block SIGALRM while making a change to the semaphores */
	/* (not needed, initialisation occurs outside of Threads::run)
	 if(sigemptyset(&sset) == -1
	   || sigaddset(&sset, SIGALRM) == -1
	   || sigprocmask(SIG_BLOCK, &sset, &oldset) == -1) {
		perror("Threads::Semaphore hide");
		return 0;
	}*/

	if(!(s = malloc(sizeof(struct Semaphore)))) {
		perror("Thread::semaphore constructor");
		return 0;
	}
	s->next          = threads->first_semaphore;
	s->prev          = 0;
	s->count = s->count_start = value;
	s->first_waiting = 0;

	if(threads->first_semaphore) threads->first_semaphore->prev = s;
	threads->first_semaphore = s;

	/* unblock SIGALRM */
	/*if(sigprocmask(SIG_SETMASK, &oldset, 0) == -1) {
		perror("Threads::Semaphore::down reset");
	}*/

	fprintf(stderr, "Thread::semaphore: new %d, #%p\n", value, (void *)s);

	return s;
}

/** "void destroy_semaphore(int semaphore); This function removes a semaphore
 from the system. A call to this function while threads are waiting on the
 semaphore should fail. That is the removal process should fail with an
 appropriate error message. If there are no threads waiting, this function will
 proceed with the removal after checking whether the current value is the same
 as the initial value of the semaphore." */
void ThreadsSemaphore_(struct Semaphore **sptr) {
	struct Semaphore *s;
	/*sigset_t sset, oldset;*/

	if(!sptr || !(s = *sptr)) return;

	/* block SIGALRM while reading from the semaphores */
	/*if(sigemptyset(&sset) == -1
	   || sigaddset(&sset, SIGALRM) == -1
	   || sigprocmask(SIG_BLOCK, &sset, &oldset) == -1) {
		perror("Threads::~Semaphore hide");
		return;
	}*/
	
	if(s->first_waiting) {
		fprintf(stderr, "Thread::~Semaphore: #%p had threads waiting, cannot erase.\n", (void *)s);
		return;
	}

	if(threads->first_semaphore == s) threads->first_semaphore = s->next;
	if(s->prev) s->prev->next = s->next;
	if(s->next) s->next->prev = s->prev;

	fprintf(stderr, "Thread::~Semaphore: erase %d (was %d,) #%p\n", s->count, s->count_start, (void *)s);
	free(s);
	*sptr = s = 0;

	/* unblock SIGALRM */
	/*if(sigprocmask(SIG_SETMASK, &oldset, 0) == -1) {
		perror("Threads::Semaphore::down reset");
	}*/
}

/** "When a thread calls this function, the value of the semaphore is
 decremented. If the value goes below 0, the thread is put into a WAIT state.
 That means calling thread is taken out of the runqueue if the value of the
 semaphore goes below 0."
 @param s a valid semaphore */
void ThreadsSemaphoreDown(struct Semaphore *s) {
	struct Thread *act/*, *thiz, *last = 0*/;
	sigset_t sset, oldset;

	if(!s || !(act = threads->active)) return;

	/* block SIGALRM */
	if(sigemptyset(&sset) == -1
	   || sigaddset(&sset, SIGALRM) == -1
	   || sigprocmask(SIG_BLOCK, &sset, &oldset) == -1) {
		perror("Threads::Semaphore::down hide");
		return;
	}

	s->count--;
	fprintf(stderr, "Threads::Semaphore::down: <%s> with %d < 0 ? %s.\n", act->name, s->count, s->count < 0 ? "true" : "false");
	if(s->count < 0) {
		/* enqueue(s->queue, CurrentThread); switch the lists */
#if 0
		for(thiz = threads->first_running; thiz && thiz != act; last = thiz, thiz = thiz->next);
		if(!thiz) return; /* should not happen! encasulation */
		if(last) {
			last->next = thiz->next;
		} else {
			threads->first_running = thiz->next;
		}
		thiz->next = s->first_waiting;
		s->first_waiting = thiz;
#else
		if(threads->first_running == act) threads->first_running = act->next;
		if(act->prev) act->prev->next = act->next;
		if(act->next) act->next->prev = act->prev;
		/* insert in waiting */
		act->prev = 0;
		act->next = threads->first_blocked;
		act->next->prev = threads->first_blocked = act;
#endif
	}

	/* unblock SIGALRM */
	if(sigprocmask(SIG_SETMASK, &oldset, 0) == -1) {
		perror("Threads::Semaphore::down reset");
	}

	/* switch threads right now by explicity;
	 fixme: reset the timer for a new quantum! */
	if(s->count < 0) timer_callback(-1);
}

/** "When a thread calls this function, the value of the semaphore is
 incremented. If value is not greater than 0, then we should have at least one
 thread waiting on it. The thread at the top of the wait queue associated with
 the semaphore is dequeued from the wait queue and enqueued in the runqueue.
 The state of the thread is changed to RUNNABLE."
 @param s a valid semaphore */
void ThreadsSemaphoreUp(struct Semaphore *s) {
	struct Thread *torun;
	sigset_t sset, oldset;

	/* block SIGALRM */
	if(sigemptyset(&sset) == -1
	   || sigaddset(&sset, SIGALRM) == -1
	   || sigprocmask(SIG_BLOCK, &sset, &oldset) == -1) {
		perror("Threads::Semaphore::up hide");
		return;
	}

	s->count++;
	fprintf(stderr, "Threads::Semaphore::up: %d.\n", s->count);
	if(s->count >= 0 && (torun = s->first_waiting)) {
		/*enqueue(runqueue, dequeue(s-­‐>queue)); put a thread on running */
		/* this is a weak semaphore, ie, not fifo, actually stack */
#if 0
		s->first_waiting = torun->next;
		torun->next = threads->first_running;
		threads->first_running = torun;
#else
		if((s->first_waiting = torun->next)) torun->next->prev = 0;
		if((torun->next = threads->first_running)) torun->next->prev = torun;
		threads->first_running = torun;
#endif
		fprintf(stderr, "Threads::Semaphore::down: freeing <%s>.\n", torun->name);
	}

	/* unblock SIGALRM */
	if(sigprocmask(SIG_SETMASK, &oldset, 0) == -1) {
		perror("Threads::Semaphore::down reset");
	}
}

/** "void mythread_state(); This function prints the state of all threads that
 are maintained by the library at any given time. For each thread, it prints
 the following information in a tabular form: thread name, thread state (print
 as a string RUNNING, BLOCKED, EXIT, etc), and amount of time run on CPU."
 @param out the stream that it's printed on */
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
	for(t = threads->first_running; t; t = t->next) thread_print(out, t, "RUNNING");
	for(t = threads->first_blocked; t; t = t->next) thread_print(out, t, "BLOCKED");
	for(t = threads->first_exit;    t; t = t->next) thread_print(out, t, "EXITED");
}

/** "Sets the quantum size of the round robin scheduler."
 @param us the number of us to run each thread */
void ThreadsSetQuantum(const int us) {
	if(!threads || us <= 0) return;
	/* { value, interval, interval, interval, ... } */
	threads->timer.it_value.tv_usec    = us;
	threads->timer.it_interval.tv_usec = us;
}

/** call this at your own risk */
char *ThreadsDebug(void) {
	static char buf[512];
	struct Thread *t;

	if(!threads) return "NoThreads";
	if(!(t = threads->active)) return "NoActive";
	sprintf(buf, "#%p<%s>", (void *)t, t->name);
	return buf;
}

/* private */

/** helper for print */
static void thread_print(FILE *out, const struct Thread *t, const char *which) {
	fprintf(out, "#%p  \t%-12.12s\t%-8d\t%s\n", (void *)t, which, t->time, t->name);
}

/** callback for switching threads */
static void timer_callback(int signal) {
	struct Thread *old, *new;

	/* fixme: this should never happen, but it happens all the time */
	if(!(old = threads->active)) {
		fprintf(stderr, "timer_callback: missing active context.\n");
		return;
	}
	new = threads->active = old->next;
	/*fprintf(stderr, " old #%p<%s> new #%p<%s>", (void *)old, old->name, (void *)new, new ? new->name : "(back)");*/
	if(new) {
		fprintf(stderr, "timer_callback: swap(%s, %s).\n", old->name, new->name);
		swapcontext(&old->context, &new->context);
	} else {
		/* restore signal values set in ThreadsRun (hopefully!) */
		/*printf("Threads::run: stopping timer.\n");*/

		if(setitimer(ITIMER_REAL, &threads->old_timer, 0) == -1
		   || sigaction(SIGALRM, &threads->old_action, 0) == -1) {
			perror("Threads::run");
			abort(); /* fixme */
		}

		fprintf(stderr, "timer_callback: swap(%s, %s).\n", old->name, "parent");
		swapcontext(&old->context, &threads->this_context);
	}
}

/** callback for destroying a thread */
static void destroy_callback(void) {
	struct Thread *a;

	if(!(a = threads->active)) {
		fprintf(stderr, "destroy_callback was called without an active thread (should not happen.)\n");
		return;
	}
	a->run = R_EXIT;
	for( ; ; ) {
		fprintf(stderr, "destory_callback: <%s> chilling on run queue.\n", a->name);
		sleep(1);
	}
}
