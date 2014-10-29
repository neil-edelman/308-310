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

	Threads();

	ThreadsPrintState(stdout);

	printf("It works.\n");

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
