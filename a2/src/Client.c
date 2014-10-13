/** 2014 Neil Edelman
 
 This is the simulation of a client.
 
 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* str* */
#include <ctype.h>  /* toupper */

#include <unistd.h> /* sleep */
#include <pthread.h>
/*#include <semaphore.h>*/

#include "Spool.h"
#include "Job.h"
#include "Client.h"

struct Client {
	pthread_t thread;
	int is_running;
	int id;
	char name[16];
	int ms_idle;
	int pages_min;
	int pages_max;
	int prints;
};

/*extern sem_t *mutex, *empty, *full;*/

static const int min_page = 1;
static const int max_page = 10;
static const int time_between_prints = 5000; /* ms */

/* private */

static int unique = 1; /* counter to assign id */

/* these are loosely based on orcish from Smaug1.8 */
static const char *words[] = { /* max chars 4 */
	"uk", "all", "uk", "ul", "um", "orc", "uruk", "ee", "ou", "eth", "om",
	"ith", "uuk", "ath", "ohk", "uth", "um", "th", "gn", "oo", "uu", "ar",
	"arg", "en", "yth", "ion", "um", "es", "ac", "ch", "k", "ul", "um", "ick",
	"uk", "of", "tuk", "ove", "aah", "ome", "ask", "my", "mal", "me", "mok",
	"to", "sek", "hi", "come", "vak", "bat", "buy", "muk", "kham", "kzam"
};
static const int words_size = sizeof(words) / sizeof(char *);
static const char *suffixes[] = { /* max chars 7 */
	"agh", "ash", "bag", "ronk", "bubhosh", "burz", "dug", "durbat", "durb",
	"ghash", "gimbat", "gimb", "-glob", "glob", "gul", "hai", "ishi", "krimpat",
	"krimp", "lug", "nazg", "nazgul", "olog", "shai", "sha", "sharku", "snaga",
	"thrakat", "thrak", "gorg", "khalok", "snar", "kurta", "ness"
};
static const int suffixes_size = sizeof(suffixes) / sizeof(char *);

static void *thread(struct Client *client);
static int random_int(const int a, const int b);
static void random_name(char *name);

/* public */

/** constructor
 @return an object or a null pointer if the object couldn't be created */
struct Client *Client(void) {
	struct Client *client;

	if(!(client = malloc(sizeof(struct Client)))) {
		perror("Client constructor");
		Client_(&client);
		return 0;
	}
	client->is_running= 0;
	client->id        = unique++;
	client->ms_idle   = time_between_prints;
	client->pages_min = min_page;
	client->pages_max = max_page;
	client->prints    = 1; /* the client is only sending one print job, ala example */
	random_name(client->name);
	fprintf(stderr, "Client: new, %s (%d) #%p.\n", client->name, client->id, (void *)client);
	/*fixme! post(empty);*/

	return client;
}

/** destructor
 @param client_ptr a reference to the object that is to be deleted */
void Client_(struct Client **client_ptr) {
	struct Client *client;

	if(!client_ptr || !(client = *client_ptr)) return;
	if(client->is_running) {
		void *value;

		pthread_join(client->thread, &value);
		client->is_running = 0;
		fprintf(stderr, "~Client: %s thread return #%p, erase #%p.\n", client->name, value, (void *)client);
	} else {
		fprintf(stderr, "~Client: %s (not running) erase #%p.\n", client->name, (void *)client);
	}
	free(client);
	*client_ptr = client = 0;
}

/** @return name */
const char *ClientGetName(const struct Client *c) { return c ? c->name : "(null)"; }

/** run the client
 @return non-zero on success */
int ClientRun(struct Client *c) {
	if(!c || c->is_running) return 0;
	pthread_create(&c->thread, 0, (void *(*)(void *))&thread, c);
	c->is_running = -1;
	return -1;
}

/* private */

/** the client thead
 @param  client
 @return null */
static void *thread(struct Client *client) {
	struct Job *job;

	if(!client) return 0;
	for( ; client->prints; client->prints--) {
		job = Job(client, random_int(client->pages_min, client->pages_max));
		/*fprintf(stderr, "%s has %d pages to print\n", client->name, JobGetPages(job));*/
		if(!SpoolPushJob(job)) {
			printf("%s: couldn't push job; full spool.\n", client->name);
			Job_(&job);
			/*wait(empty);*/
		}
		/*sleep(client->ms_idle);*/
	}
	fprintf(stderr, "%s signing off.\n", client->name);
	return 0;
}

/**
 @param a
 @param b
 @return  random number in [a, b], uniformly distributed */
static int random_int(const int a, const int b) {
	/* not thread safe (just be careful) */
	int r = (int)(((float)rand() / RAND_MAX) * (float)(b - a + 1)) + a;
	/* in case rand returned RAND_MAX */
	return r > b ? b : r;
}

/** come up with random names
 @param name the string that's going to hold it, must be at least
 2*max(words4) + max(suffixes7) + 1 long (cur 16) */
static void random_name(char *name) {
	int i;
	
	/* fixme: there's no gauratee that these are unique, but it doesn't really
	 matter */
	i = rand() % words_size;
	strcpy(name, words[i]);
	i = rand() % words_size;
	strcat(name, words[i]);
	i = rand() % suffixes_size;
	strcat(name, suffixes[i]);
	name[0] = toupper(name[0]);
}
