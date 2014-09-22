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
static int cmd_cd(char *args[]);
static int cmd_echo(char *args[]);
static int cmd_exit(char *args[]);
static int cmd_history(char *args[]);
static int cmd_not(char *args[]);
static int cmd_pwd(char *args[]);
static int cmd_redo(char *args[]);

/* ascii-betical */
static const struct Command {
	char *name;
	int  (*fn)(char **);
} builtin[] = {
	{ "cd",     &cmd_cd },
	{ "echo",   &cmd_echo },
	{ "exit",   &cmd_exit },
	{ "fg",     &cmd_not },
	{ "history",&cmd_history },
	{ "jobs",   &cmd_not },
	{ "pwd",    &cmd_pwd },
	{ "r",      &cmd_redo }
};

/** (static) search
 @param  cmd what your searching for
 @return fn  what it corresponds to, or null */
int (*CommandSearch(const char *cmd))(char **) {
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

static int cmd_cd(char *args[]) {
	printf("cd!\n");
	return -1;
}

static int cmd_echo(char *args[]) {
	int i;

	for(i = 0; args[i]; i++) {
		printf("%s\n", args[i]);
	}

	return -1;
}

static int cmd_exit(char *args[]) {
	printf("Goodday to you!\n");
	return 0;
}

static int cmd_history(char *args[]) {
	if(args[1]) {
		fprintf(stderr, "usage: history\n");
	} else {
		SimpleHistory();
	}
	return -1;
}

static int cmd_not(char *args[]) {
	printf("Not implemented.\n");
	return -1;
}

static int cmd_pwd(char *args[]) {
	printf("pwd!\n");
	return -1;
}

static int cmd_redo(char *args[]) {
	if(args[2]) {
		fprintf(stderr, "usage: r [first letters]\n");
	} else if(!SimpleRedo(args[1])) {
		fprintf(stderr, "Could not run <%s> again.\n", args[0]);
	}
	return -1;
}
