/** 2014 Neil Edelman

 This is a printer simulation in POSIX.

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free rand */
#include <stdio.h>  /* fprintf */
#include <time.h>   /* clock */

#include <pthread.h>
#include <semaphore.h>

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
	int head, tail;
	struct Printer **printer;
	int printers_size;
	struct Client **client;
	int clients_size;
	int is_mutex, is_empty, is_full;
	sem_t *mutex, *empty, *full;
};
/*static const int buffer_size = sizeof((struct Spool *)0)->buffer / sizeof(struct Job *);*/

static const int jobs        = 3;
static const int clients     = 4;
static const int printers    = 2;
static const int ms_per_page = 1000;
/* 644 */
static const int permission  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

const char *mutex_name = "/ps-mutex";
const char *empty_name = "/ps-empty";
const char *full_name  = "/ps-full";

/* private */
static void usage(void);

static struct Spool *the_spool;

/** entry point
 @param argc the number of arguments starting with the programme name
 @param argv the arguments
 @return     either EXIT_SUCCESS or EXIT_FAILURE */
int main(int argc, char **argv) {
	int i;

	if(argc > 1) {
		usage();
		return EXIT_SUCCESS;
	}

	srand(clock());

	if(!(the_spool = Spool(jobs, printers, clients))) return EXIT_FAILURE;
	for(i = 0; i < printers; i++) PrinterRun(the_spool->printer[i]);
	for(i = 0; i < clients;  i++) ClientRun(the_spool->client[i]);
	Spool_(&the_spool);

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
	s->head = s->tail = 0;
	s->printer       = (struct Printer **)&s->job[jobs_size];
	s->printers_size = printers_size;
	s->client        = (struct Client **)&s->printer[printers_size];
	s->clients_size  = clients_size;
	s->is_mutex = s->is_empty = s->is_full = 0;
	for(i = 0; i <     jobs_size; i++) s->job[i]     = 0;
	for(i = 0; i < printers_size; i++) s->printer[i] = Printer(ms_per_page);
	for(i = 0; i <  clients_size; i++) s->client[i]  = Client();

	/* initaialise the semaphores -- typedefs, gah; at least we could have been
	 given an invalid flag */
	/* "mutex: Function not implemented" */
	/*if(sem_init(&s->mutex, 0, 1) == -1) {
		perror("mutex");
		Spool_(&s);
		return 0;
	}
	s->is_mutex = -1;
	if(sem_init(&s->empty, 0, jobs_size) == -1) {
		perror("empty");
		Spool_(&s);
		return 0;
	}
	s->is_empty = -1;
	if(sem_init(&s->full, 0, 0) == -1) {
		perror("full");
		Spool_(&s);
		return 0;
	}
	s->is_full  = -1;*/
	/*
	#define SNAME "/mysem"
	sem_t *sem = sem_open(SNAME, O_CREAT, 0644, 3);
	sem_t *sem = sem_open(SEM_NAME, 0); */
	/* 644 */
	if((s->mutex = sem_open(mutex_name, O_CREAT | O_EXCL, permission, 1)) == SEM_FAILED) {
		perror("mutex");
		Spool_(&s);
	}
	s->is_mutex = -1;

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
	/* "Only a semaphore that was created using sem_init() may be destroyed
	 using sem_destroy()" while hmm */
	/*if(s->is_mutex) { sem_destroy(&s->mutex); s->is_mutex = 0; }
	if(s->is_empty) { sem_destroy(&s->empty); s->is_empty = 0; }
	if(s->is_full)  { sem_destroy(&s->full);  s->is_full = 0; }*/
	if(s->is_mutex) {
		if(sem_close(s->mutex) == -1) {
			perror("mutex");
		}
		if(sem_unlink(mutex_name) == -1) {
			perror("mutex");
		}
		s->is_mutex = -1;
	}
	if(s->is_empty) {
		if(sem_close(s->empty) == -1) {
			perror("empty");
		}
		if(sem_unlink(empty_name) == -1) {
			perror("empty");
		}
		s->is_empty = -1;
	}
	if(s->is_full) {
		if(sem_close(s->full) == -1) {
			perror("full");
		}
		if(sem_unlink(full_name) == -1) {
			perror("full");
		}
		s->is_full = -1;
	}
	fprintf(stderr, "~Spool: erase, #%p.\n", (void *)s);
	free(s);
	*s_ptr = s = 0;
}

/** attempts to spool the job to the printing queue in the_spool
 @param job
 @return non-zero on success */
int SpoolJob(const struct Job *job) {
	int new_head;

	if(!the_spool || !job || JobGetPages(job) <= 0) return 0;
	new_head = (the_spool->head + 1) % the_spool->jobs_size;
	printf("%s has %d pages to print, ",
		   ClientGetName(JobGetClient(job)),
		   JobGetPages(job));
	if(new_head == the_spool->tail) {
		/* spool is full */
		printf("buffer full, sleeps (not really)\n");
		return 0;
	}
	the_spool->head = new_head;
	the_spool->job[the_spool->head] = (struct Job *)job;
	printf("puts request in Buffer[%d]\n", the_spool->head);

	return -1;
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
