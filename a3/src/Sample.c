/** 2014 Neil Edelman

 This is a sample of the use of the Threads 'library.'

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include "Threads.h"
#include "Sample.h"

/* constants */
static const char *programme   = "Sample";
static const char *year        = "2014";
static const int versionMajor  = 1;
static const int versionMinor  = 0;

static void usage(void);
static void foo(const int exit);

/** entry point
 @param argc the number of arguments starting with the programme name
 @param argv the arguments
 @return     either EXIT_SUCCESS or EXIT_FAILURE */
int main(int argc, char **argv) {
	if(argc != 1) {
		usage();
		return EXIT_SUCCESS;
	}

	ThreadsPrintState(stdout);
	fprintf(stderr, "Oops, we forgot to initialise!\n\n");

	Threads();

	fprintf(stderr, "\nInitial state:\n");
	ThreadsPrintState(stdout);
	fprintf(stderr, "\n");

	ThreadsCreate("foo", &foo, 100);
	ThreadsCreate("bar", &foo, 100);

	ThreadsPrintState(stdout);

	fprintf(stderr, "\nNow we run.\n");
	ThreadsRun();

	printf("It works.\n");

	fprintf(stderr, "\nDon't forget exit cleanup!\n");
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

static void foo(const int exit) {
	int i;

	for(i = 0; i < 10; i++) {
		printf("foo!\n");
		sleep(1);
	}
	printf("foo exit!\n");

	ThreadsExit(exit);
}
