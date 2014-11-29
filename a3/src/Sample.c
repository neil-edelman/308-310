/** 2014 Neil Edelman

 This is a sample of the use of the Threads 'library.'

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */

#include <unistd.h> /* sleep */
#include <time.h>   /* nanosleep */

#include "Threads.h"
#include "Sample.h"

/* constants */
static const char *programme   = "Sample";
static const char *year        = "2014";
static const int versionMajor  = 1;
static const int versionMinor  = 0;
static struct timespec tv      = { 0, 10000000 };

static void usage(void);
static void foo(int a);
static void bar(int a);
static void baz(int a);
static void qux(int a);

static struct Semaphore *mutex;
static int emu; /* EMU!! (= 0 implictly for global vars) */

/** entry point
 @param argc the number of arguments starting with the programme name
 @param argv the arguments
 @return     either EXIT_SUCCESS or EXIT_FAILURE */
int main(int argc, char **argv) {
	int i = 0;

	if(argc != 1) {
		usage();
		return EXIT_SUCCESS;
	}

	ThreadsPrintState(stdout);
	fprintf(stderr, "\n");

	Threads();

	fprintf(stderr, "Initial state:\n");
	ThreadsPrintState(stdout);
	fprintf(stderr, "\n");

	ThreadsCreate("foo", &foo, 1, 32000);
	ThreadsPrintState(stdout);
	fprintf(stderr, "\n");

	ThreadsCreate("bar", &bar, 2, 32000);
	ThreadsCreate("baz", &baz, 3, 32000);
	ThreadsCreate("qux", &qux, 4, 32000);
	ThreadsPrintState(stdout);
	fprintf(stderr, "\n");

	mutex = ThreadsSemaphore(1);

	fprintf(stderr, "Now we run, Emu = %d:\n", emu);
	do {
		fprintf(stderr, "Loop %d.\n", ++i);
		sleep(1);
	} while(ThreadsRun());
	printf("It works.\n\n");

	ThreadsSemaphore_(&mutex);

	fprintf(stderr, "Don't forget exit cleanup.\n");
	Threads_();

	return EXIT_SUCCESS;
}

/** prints command-line help */
static void usage(void) {
	fprintf(stderr, "Usage: %s\n", programme);
	fprintf(stderr, "Version %d.%d.\n\n", versionMajor, versionMinor);
	fprintf(stderr, "%s Copyright %s Neil Edelman\n", programme, year);
	fprintf(stderr, "This program comes with ABSOLUTELY NO WARRANTY.\n\n");
}

static void foo(int a) {
	int i;

	for(i = 0; i < 10; i++) {
		printf("foo: %d, %s.\n", a, ThreadsDebug());
		nanosleep(&tv, 0);
		ThreadsSemaphoreDown(mutex);
		printf("foo: augmenting Emu = %d.\n", ++emu);
		ThreadsSemaphoreUp(mutex);
	}
	printf("foo exit!\n");
}

static void bar(int a) {
	int i;
	
	for(i = 0; i < 10; i++) {
		printf("bar %d! %s\n", a, ThreadsDebug());
		nanosleep(&tv, 0);
		ThreadsSemaphoreDown(mutex);
		printf("bar: augmenting Emu = %d.\n", ++emu);
		ThreadsSemaphoreUp(mutex);
	}
	printf("bar exit!\n");
}

static void baz(int a) {
	int i;
	
	for(i = 0; i < 10; i++) {
		printf("baz %d! %s\n", a, ThreadsDebug());
		nanosleep(&tv, 0);
		ThreadsSemaphoreDown(mutex);
		printf("baz: augmenting Emu = %d.\n", ++emu);
		ThreadsSemaphoreUp(mutex);
	}
	printf("baz exit!\n");
}

static void qux(int a) {
	int i;
	
	for(i = 0; i < 10; i++) {
		printf("qux %d! %s\n", a, ThreadsDebug());
		nanosleep(&tv, 0);
		ThreadsSemaphoreDown(mutex);
		printf("qux: augmenting Emu = %d.\n", ++emu);
		ThreadsSemaphoreUp(mutex);
	}
	printf("qux exit!\n");
}
