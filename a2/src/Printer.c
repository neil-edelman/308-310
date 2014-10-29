/** 2014 Neil Edelman

 This is a simulation of a printer.

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* strerror */
#include <errno.h>  /* strerror */

#include <pthread.h>
#include <semaphore.h>
/*#include <time.h> timeval isn't portable */
#include <unistd.h> /* sleep for hack */

#include "Job.h"
#include "Client.h"
#include "Spool.h"
#include "Printer.h"

struct Printer {
	pthread_t thread;
	int is_running;
	int id;
};

static const int s_shutdown = 1;

extern sem_t *empty, *full;

static void *thread(struct Printer *p);

/* private */

static int unique = 1; /* counter to assign id */

/* public */

/** constructor
 @param  ms_per_page speed of the printer
 @return             an object or a null pointer if the object couldn't be created */
struct Printer *Printer(void) {
	struct Printer *printer;

	if(!(printer = malloc(sizeof(struct Printer)))) {
		perror("Printer constructor");
		Printer_(&printer);
		return 0;
	}
	printer->is_running  = 0;
	printer->id          = unique++;
	fprintf(stderr, "Printer: new, %d with %fs/page #%p.\n",
			printer->id,
			/*printer->ms_per_page / 1000.0*/1000.0,
			(void *)printer);

	return printer;
}

/** destructor
 @param printer_ptr a reference to the object that is to be deleted */
void Printer_(struct Printer **printer_ptr) {
	struct Printer *printer;

	if(!printer_ptr || !(printer = *printer_ptr)) return;
	if(printer->is_running) {
		void *value;

		pthread_join(printer->thread, &value);
		printer->is_running = 0;
		fprintf(stderr, "~Printer: %d thread return #%p, erase #%p.\n", printer->id, value, (void *)printer);
	} else {
		fprintf(stderr, "~Client: %d (not running) erase #%p.\n", printer->id, (void *)printer);
	}
	free(printer);
	*printer_ptr = printer = 0;
}

/** @return id */
int PrinterGetId(const struct Printer *p) { return p ? p->id : 0; }

/** run the printer
 @return non-zero on success */
int PrinterRun(struct Printer *p) {
	if(!p || p->is_running) return 0;
	if(pthread_create(&p->thread, 0, (void *(*)(void *))&thread, p)) {
		fprintf(stderr, "Printer %d: broken.\n", p->id);
		return 0;
	}
	p->is_running = -1;
	return -1;
}

/* private */

/** run the printer
 @return the number of pages it has printed */
static void *thread(struct Printer *p) {
	struct Job *job;

	for( ; ; ) {
		fprintf(stderr, "Printer %d waiting.\n", p->id);
		/* clock_gettime(CLOCK_MONOTONIC, &ts) doesn't work on my OS so
		 sem_timedwait(full) doesn't work; hack: the printer always waits
		 s_shutdown s and then it polls wheater it has jobs */
		sleep(s_shutdown);
		if(sem_trywait(full) == -1) {
			if(errno == EAGAIN) {
				printf("No request in buffer, Printer %d sleeps.\n", p->id);
			} else {
				perror("full");
			}
			/*fprintf(stderr, "Printer %d: exiting; %s.\n", p->id,
					errno == EAGAIN ? "no jobs" : strerror(errno));*/
			return 0;
		}
		fprintf(stderr, "Printer %d go!\n", p->id);
		/* yes! this does get called about 1% of the time; I made it sleep 1s;
		 I have not tested it because it's non-deterministic */
		/* it happend again; this doesn't fix it */
		while(!(job = SpoolPopJob())) {
			fprintf(stderr, "Printer %d: found nothing to print;\n", p->id);
			fprintf(stderr, "this could be caused by multiple instances, signals from past runs interfering\n");
			fprintf(stderr, "with the semaphores, or gremlins; try again.\n");
			/*sleep(1);*/
			exit(EXIT_FAILURE); /* FIXME!!! */
		}
		{
			const char *name = ClientGetName(JobGetClient(job));
			int pp           = JobGetPages(job);
			int buf          = JobGetBuffer(job);
			printf("Printer %d starts printing %d pages from buffer[%d] (from %s)\n",
					p->id, pp, buf, name);
			sleep(pp);
			JobPrintPages(job, pp);
			Job_(&job);
		}
		if(sem_post(empty) == -1) perror("empty");
	}

	return 0; /* fixme */
}
