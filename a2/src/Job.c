/** 2014 Neil Edelman

 This is a print job for the printer simulation.

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free rand */
#include <stdio.h>  /* fprintf */
#include "Client.h"
#include "Job.h"

struct Job {
	struct Client *client;
	int pages;
	int done;
};

/* public */

/** constructor
 @return an object or a null pointer if the object couldn't be created */
struct Job *Job(struct Client *c, int pages) {
	struct Job *job;

	if(!c || pages <= 0) {
		fprintf(stderr, "Job: invalid parameters.\n");
		return 0;
	}
	if(!(job = malloc(sizeof(struct Job)))) {
		perror("Job constructor");
		Job_(&job);
		return 0;
	}
	job->client    = c;
	job->pages     = pages;
	job->done      = 0;
	fprintf(stderr, "Job: new, %d pages for %s #%p.\n", pages, ClientGetName(c), (void *)job);

	return job;
}

/** destructor
 @param job_ptr a reference to the object that is to be deleted */
void Job_(struct Job **job_ptr) {
	struct Job *job;

	if(!job_ptr || !(job = *job_ptr)) return;
	fprintf(stderr, "~Job: erase, print job for %s, done %d / %d, #%p.\n", ClientGetName(job->client), job->done, job->pages, (void *)job);
	free(job);
	*job_ptr = job = 0;
}

/** @return the number of pages left */
int JobGetPages(const struct Job *j) { return j ? j->pages - j->done : 0; }

/** @return client */
struct Client *JobGetClient(const struct Job *j) { return j ? j->client : 0; }

/**
 @param job
 @param printed the number of printed pages to augment */
void JobPrintPages(struct Job *job, int printed) {
	if(!job || printed <= 0) return;
	if(job->pages - job->done > printed) {
		fprintf(stderr, "Job: %s printed more pages then in the print job?\n", ClientGetName(job->client));
		job->done = job->pages;
	} else {
		job->done += printed;
	}
}

/* private */
