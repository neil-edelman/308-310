/** These are the built-in commands.

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free */
#include <stdio.h>  /* fprintf */
#include <string.h> /* strcmp */
#include "Simple.h"
#include "Command.h"

static int cmd_comp(const void *key, const void *elem);
static int cmd_cd(char *args[], int *);
static int cmd_echo(char *args[], int *);
static int cmd_exit(char *args[], int *);
static int cmd_fg(char *args[], int *);
static int cmd_history(char *args[], int *);
static int cmd_jobs(char *args[], int *);
static int cmd_pwd(char *args[], int *);
static int cmd_redo(char *args[], int *);

/* ascii-betical */
static const struct Command {
	char *name;
	int  (*fn)(char **, int *);
} builtin[] = {
	{ "cd",     &cmd_cd },
	{ "echo",   &cmd_echo },
	{ "exit",   &cmd_exit },
	{ "fg",     &cmd_fg },
	{ "history",&cmd_history },
	{ "jobs",   &cmd_jobs },
	{ "pwd",    &cmd_pwd },
	{ "r",      &cmd_redo }
};

/** (static) search
 @param  cmd what your searching for
 @return fn  what it corresponds to, or null */
int (*CommandSearch(const char *cmd))(char **, int *) {
	struct Command *c = bsearch(cmd, builtin,
								sizeof(builtin) / sizeof(struct Command),
								sizeof(struct Command),
								&cmd_comp);
	return c ? c->fn : 0;
}

/* private */

/** bsearch: compares the string with the strings from builtin */
static int cmd_comp(const void *key, const void *elem) {
	return strcmp((char *)key, ((struct Command *)elem)->name);
}

/* these are elements of builtin */

static int cmd_cd(char *args[], int *exit_ptr) {
	printf("cd!\n");
	/* chdir() */
	return -1;
}

static int cmd_echo(char *args[], int *exit_ptr) {
	int i;

	for(i = 0; args[i]; i++) {
		printf("%s\n", args[i]);
	}

	return -1;
}

static int cmd_exit(char *args[], int *exit_ptr) {
	printf("Goodday to you!\n");
	*exit_ptr = -1;
	return -1;
}

static int cmd_fg(char *args[], int *exit_ptr) {
	return -1;
}

static int cmd_history(char *args[], int *exit_ptr) {
	if(args[1]) {
		fprintf(stderr, "usage: history\n");
	} else {
		SimpleHistory();
	}
	return -1;
}

static int cmd_jobs(char *args[], int *exit_ptr) {
	printf("Not implemented.\n");
	return -1;
}

static int cmd_pwd(char *args[], int *exit_ptr) {
	printf("pwd!\n");
	/* getcwd() */
	return -1;
}

/* FIXME: AVOID GETTING INTO AN INFINITE LOOP! */
static int cmd_redo(char *args[], int *exit_ptr) {
	int exec;

	if(args[2]) {
		fprintf(stderr, "usage: r [first letters] (%s?)\n", args[2]);
		return 0;
	} else if(!SimpleRedo(args[1], &exec)) {
		fprintf(stderr, "Could not run re-do again.\n");
		return 0;
	}

	/* pass the execute error status to the main programme */
	return exec;
}
