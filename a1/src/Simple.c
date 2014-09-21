/** 110121860
 
 "A C program that provides the basic operations of a command line shell is
 supplied below. This program is composed of two functions: main() and setup().
 The setup() function reads in the user’s next command (which can be up to 80
 characters), and then parses it into separate tokens that are used to fill the
 argument vector for the command to be executed. (If the command is to be run
 in the background, it will end with ‘&’, and setup() will update the parameter
 background so the main() function can act accordingly. The program terminates
 when the user enters <Control><D>; setup() invokes exit().
 
 "The main() function presents the prompt COMMAND-> and then invokes setup(),
 which waits for the user to enter a command. The contents of the command
 entered by the user are loaded into the args array. For example, if the user
 enters ls –l at the COMMAND-> prompt, args[0] becomes equal to the string ls
 and args[1] is set to the string to –l. (By “string,” we mean a null-
 terminated, C-style string variable.)"

 @author Neil
 @version 1
 @since 2014 */

#include <stdlib.h> /* malloc free EXIT_SUCCESS */
#include <stdio.h>  /* fprintf */
#include <string.h> /* strtok */
#include <sys/types.h>/* "The include file <sys/types.h> is necessary." */
#include <unistd.h> /* fork, etc (POSIX) */
#include <sys/wait.h>/* waitpid (POSIX) */
#include "Command.h"
#include "Simple.h"

/* constants */
static const char *programme   = "A1";
static const char *year        = "2014";
static const int versionMajor  = 1;
static const int versionMinor  = 0;

struct Simple {
	char inputBuffer[81]; /* "80 chars per line, per command, should be enough." lol */
	char *argv[40];       /* pointers to inputBuffer, null-terminated */
	int background;
	int noChild;
	int pidChild[40];
};
static const int input_size = sizeof((struct Simple *)0)->inputBuffer / sizeof(char);
static const int input_args = sizeof((struct Simple *)0)->argv / sizeof(char *);
static const int max_child  = sizeof((struct Simple *)0)->pidChild / sizeof(int);

static const char *delimiters = " \t\n\f\r";
static const char background  = '&';

/* static function prototypes */
static int setup(struct Simple *s);
static int forground_child(const int pid_child, int *exit_condition_ptr);
static void check_background_processes(struct Simple *s);
static void usage(void);

/** entry point
 @param argc the number of arguments starting with the programme name
 @param argv the arguments
 @return     either EXIT_SUCCESS or EXIT_FAILURE */
int main(int argc, char **argv) {
	int child, exit_status, ret = EXIT_SUCCESS;
	struct Simple *simple;
	int (*cmd)(char **);

	/* no command line switches */
	if(argc > 1) {
		usage();
		return EXIT_SUCCESS;
	}

	if(!(simple = Simple())) return EXIT_FAILURE;

	for( ; ; ) { /* Program terminates normally inside setup */
		int i;

		printf(" COMMAND->\n");

		/* get next command */
		if(!setup(simple)) {
			check_background_processes(simple);
			break;
		}

		/* if it's a blank line, check for exit of bg proc */
		if(!simple->argv[0]) {
			check_background_processes(simple);
			continue;
		}

		/* check for built-in commands with binary search; can not run in bg */
		if((cmd = CommandSearch(simple->argv[0]))) {
			if(!(*cmd)(simple->argv)) break;
			/* fixme: put it in the list */
			continue;
		}

		for(i = 0; simple->argv[i]; i++) fprintf(stderr, "args[%d] <%s>\n", i, simple->argv[i]);

		/* "the steps are:
		 (1) fork a child process using fork()
		 (2) the child process will invoke execvp()
		 (3) if background == 1, the parent will wait,
		 otherwise returns to the setup() function."
		 I assume the other way around; this is overly-complex? why would you
		 fork if you didn't need to run in the background? whatever */

		if((child = fork()) == -1) {
			/* this is messed up */
			perror(simple->argv[0]);
			ret = EXIT_FAILURE;
			break;
		} else if(child) {
			/* this is the parent */
			fprintf(stderr, "Parent: created child, pid %d, %sground.\n",
					child, simple->background ? "back" : "fore");
			if(simple->background) {
				if(simple->noChild >= max_child) {
					fprintf(stderr, "Hit maximum %d processes running in background.\n", max_child);
				} else {
					simple->pidChild[simple->noChild++] = child;
				}
			} else {
				if(!forground_child(child, &exit_status)) continue;
			}
			/* fixme: do the entry into history if exit_status = EXIT_SUCCESS */
			/* every time it checks what processes have exited */
			check_background_processes(simple);
		} else {
			/* this is the child */
			fprintf(stderr, "Child: exec %s.\n", simple->argv[0]);
			if((ret = execvp(simple->argv[0], simple->argv)) == -1) {
				perror(simple->argv[0]);
			}
			break;
		}
	}

	Simple_(&simple);

	return ret;
}

/* public */

/** @return an Simple or a null pointer if the object couldn't be created */
struct Simple *Simple(void) {
	struct Simple *s;

	if(!(s = malloc(sizeof(struct Simple)))) {
		perror("Simple constructor");
		Simple_(&s);
		return 0;
	}
	s->inputBuffer[0] = '\0';
	s->argv[0]        = 0; /* null-terminated */
	s->background     = 0; /* default not starting a process running in background */
	s->noChild        = 0;
	fprintf(stderr, "Simple: new, #%p.\n", (void *)s);

	return s;
}

/** @param s_ptr a reference to the Simple that is to be deleted */
void Simple_(struct Simple **s_ptr) {
	struct Simple *s;
	
	if(!s_ptr || !(s = *s_ptr)) return;
	fprintf(stderr, "~Simple: erase, #%p.\n", (void *)s);
	free(s);
	*s_ptr = s = 0;
}

/* private */

/** setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string.
 @param s a valid Simple
 @return non-zero on success */
static int setup(struct Simple *s) {
	int i;
	char *tok;

	/* read what the user enters on the command line */
	/* examine every character in the inputBuffer */
	/* no. -Neil */

	/* yay ANSI */
	if(!fgets(s->inputBuffer, input_size, stdin)) return 0;
	/* strsep the command into args; ignores args >= input_args */
	for(tok = strtok(s->inputBuffer, delimiters), i = 0;
		tok && i < input_args - 1;
		tok = strtok(0, delimiters)) {
		s->argv[i++] = tok;
	}
	s->argv[i] = 0; /* null-terminated */
	s->background = 0;
	/* if the last char is an '&,' set background */
	if(i > 0) {
		int len;
		char *last, *ch;

		last = s->argv[i - 1];
		len  = strlen(last);
		/* assert(len > 0); */
		ch   = last + len - 1;
		if(*ch == background) {
			*ch = '\0';
			s->background = -1;
			if(len <= 1) i--;   /* & on it's own */
		}
	}
	/*s->argc = i;*/
	/* fixme: built-in commands! */

	return -1;
}

/** waits for the process to terminate on the child
 @param pid_child          the pid of the process
 @param exit_condition_ptr if success, what the exit condition was
 @return                   non-zero on success */
static int forground_child(const int pid_child, int *exit_condition_ptr) {
	int status;
	int wait;

	if((wait = waitpid(pid_child, &status, 0)) == -1) {
		perror("waitpid");
		return 0;
	}
	/* fixme: it's a bit more complicated then this */
	if(!WIFEXITED(status)) {
		fprintf(stderr, "Parent: abnormal exit from %d.\n", pid_child);
		return 0;
	}
	*exit_condition_ptr = WEXITSTATUS(status);
	fprintf(stderr, "Parent: %d child exited %d.\n", pid_child, *exit_condition_ptr);

	return -1;
}

/** checks for exits from background processes; cleans up
 @param s a valid Simple */
static void check_background_processes(struct Simple *s) {
	int i, max;
	int status;
	int wait;
	int pid_child;

	for(i = 0; i < s->noChild; i++) {
		pid_child = s->pidChild[i];
		fprintf(stderr, "Check background %d.\n", pid_child);
		if((wait = waitpid(pid_child, &status, WNOHANG)) == -1) {
			perror("waitpid");
		}
		/* I'm scetchy on this one; I think something on the internet says if
		 waitpid with WNOHANG detects nothing, this will always be zero */
		if(!wait) continue;
		/* signal due to anything */
		if(WIFEXITED(status)) {
			int exit_status = WEXITSTATUS(status);

			fprintf(stderr, "Background process %d exited with status %d.\n", pid_child, exit_status);
		} else {
			fprintf(stderr, "Background process %d exited abnormally.\n", pid_child);
		}
		/* remove it from the list; apparently it exited */
		max = s->noChild - 1;
		if(i < max) s->pidChild[i] = s->pidChild[max];
		s->noChild--;
		i--;
	}
}

/** prints command-line help */
static void usage(void) {
	fprintf(stderr, "Usage: %s\n", programme);
	fprintf(stderr, "Version %d.%d.\n\n", versionMajor, versionMinor);
	fprintf(stderr, "%s Copyright %s Neil Edelman\n", programme, year);
	fprintf(stderr, "This program comes with ABSOLUTELY NO WARRANTY.\n");
	fprintf(stderr, "This is classwork for McGill, 308-310.\n\n");
}
