/** 2014 Neil Edelman

 This is a printer simulation in POSIX.

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free rand */
#include <stdio.h>  /* fprintf */
#include <time.h>   /* clock */
#include <pthread.h>
#include "Job.h"
#include "Client.h"
#include "Printer.h"
#include "Spool.h"

/* constants */
static const char *programme   = "PrinterSimulation";
static const char *year        = "2014";
static const int versionMajor  = 1;
static const int versionMinor  = 0;

struct Spool {
	struct Job **job;
	int jobs_size;
	struct Printer **printer;
	int printers_size;
	struct Client **client;
	int clients_size;
};
/*static const int buffer_size = sizeof((struct Spool *)0)->buffer / sizeof(struct Job *);*/

static const int jobs        = 3;
static const int clients     = 4;
static const int printers    = 2;
static const int ms_per_page = 1000;

/* private */
static void usage(void);

/** entry point
 @param argc the number of arguments starting with the programme name
 @param argv the arguments
 @return     either EXIT_SUCCESS or EXIT_FAILURE */
int main(int argc, char **argv) {
	struct Spool *s;
	int i;

	if(argc > 1) {
		usage();
		return EXIT_SUCCESS;
	}

	srand(clock());

	s = Spool(jobs, printers, clients);
	for(i = 0; i < clients; i++) ClientRun(s->client[i]);
	for(i = 0; i < clients; i++) Client_(&s->client[i]);
	Spool_(&s);
	return EXIT_SUCCESS;
}

/* public */

/** constructor
 @return an object or a null pointer if the object couldn't be created */
struct Spool *Spool(const int jobs_size, const int printers_size, const int clients_size) {
	struct Spool *s;
	int i;

	if(jobs_size < 1 || printers_size < 1 || clients_size < 1) {
		fprintf(stderr, "Spool: invalid parameters.\n");
		return 0;
	}
	if(!(s = malloc(sizeof(struct Spool)
					+ sizeof(struct Job *) * jobs_size
					+ sizeof(struct Printer *) * printers_size
					+ sizeof(struct Client *) * clients_size))) {
		perror("Spool constructor");
		Spool_(&s);
		return 0;
	}
	s->job           = (struct Job **)(s + 1);
	s->jobs_size     = jobs_size;
	s->printer       = (struct Printer **)&s->job[jobs_size];
	s->printers_size = printers_size;
	s->client        = (struct Client **)&s->printer[printers_size];
	s->clients_size  = clients_size;
	for(i = 0; i <     jobs_size; i++) s->job[i]     = 0;
	for(i = 0; i < printers_size; i++) s->printer[i] = Printer(ms_per_page);
	for(i = 0; i <  clients_size; i++) s->client[i]  = Client();
	fprintf(stderr, "Spool: new, jobs %d, printers %d, clients %d, #%p.\n",
			jobs_size, printers_size, clients_size, (void *)s);

	return s;
}

/** destructor
 @param oo_ptr a reference to the object that is to be deleted */
void Spool_(struct Spool **s_ptr) {
	struct Spool *s;
	int i;

	if(!s_ptr || !(s = *s_ptr)) return;
	for(i = 0; i < s->jobs_size;     i++) Job_(&s->job[i]);
	for(i = 0; i < s->printers_size; i++) Printer_(&s->printer[i]);
	for(i = 0; i < s->clients_size;  i++) Client_(&s->client[i]);
	fprintf(stderr, "~Spool: erase, #%p.\n", (void *)s);
	free(s);
	*s_ptr = s = 0;
}

/* private */

/** prints command-line help */
static void usage(void) {
	fprintf(stderr, "Usage: %s\n", programme);
	fprintf(stderr, "Version %d.%d.\n\n", versionMajor, versionMinor);
	fprintf(stderr, "%s Copyright %s Neil Edelman\n", programme, year);
	fprintf(stderr, "This program comes with ABSOLUTELY NO WARRANTY.\n");
	fprintf(stderr, "This is free software, and you are welcome to redistribute it\n");
	fprintf(stderr, "under certain conditions; see copying.txt.\n\n");
}
