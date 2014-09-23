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
#include <string.h> /* strtok memcpy */
#include <time.h>   /* clock */
#include <sys/types.h>/* "The include file <sys/types.h> is necessary." (POSIX) */
#include <unistd.h> /* fork, etc (POSIX) */
#include <sys/wait.h>/* waitpid (POSIX) */
#include "Command.h"
#include "Simple.h"

/* constants */
static const char *programme   = "A1";
static const char *year        = "2014";
static const int versionMajor  = 1;
static const int versionMinor  = 0;

/* private */
enum Result {
	R_INVALID = 1,
	R_BACKGROUND = 2,
	R_BUILTIN = 4,
	R_SUCCESS = 8,
	R_FORK_ERROR = 16,
	R_ABNORMAL = 32,
	R_EXEC_ERROR = 64,
	R_FAILURE = 128
};

/* private */
struct Input {
	int  no;              /* no = 0: this is not valid */
	char inputBuffer[81]; /* "80 chars per line, per command, should be enough." lol */
	char *args[40];       /* pointers to inputBuffer, null-terminated */
	int  pid_child;
	enum Result result;
};
static const int input_size = sizeof((struct Input *)0)->inputBuffer / sizeof(char);
static const int input_args = sizeof((struct Input *)0)->args / sizeof(char *);

/* private */
struct Job {
	int pid;
	char name[16]; /* > 16 because called on random_name() */
};

/* public */
struct Simple {
	struct Input input;
	struct Input history[10];
	int          command_no;  /* running total */
	int          no_jobs;
	struct Job   job[40];
};
static const int job_size     = sizeof((struct Simple *)0)->job / sizeof(struct Job);
static const int history_size = sizeof((struct Simple *)0)->history / sizeof(struct Input);

static const char *delimiters = " \t\n\f\r";
static const char background  = '&';

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

/* static function prototypes */
static int setup(struct Simple *s);
static void setup_redo(struct Simple *s, const struct Input *selected);
static int execute_input(struct Input *input);
static int wait_child(const int pid_child, const int bg, int *running_ptr, int *exit_condition_ptr);
static void check_background_processes(struct Simple *s);
static void make_history(struct Simple *s);
static void usage(void);
static void random_name(char *name, const char *seed);

/* authoritative, used for when we guess (but really there's only one) */
struct Simple *simple;

/* public */

/** @return an Simple or a null pointer if the object couldn't be created */
struct Simple *Simple(void) {
	int i;
	struct Simple *s;

	if(!(s = malloc(sizeof(struct Simple)))) {
		perror("Simple constructor");
		Simple_(&s);
		return 0;
	}
	s->command_no     = 1;
	s->input.no             = s->command_no;
	s->input.inputBuffer[0] ='\0';
	s->input.args[0]        = 0; /* null-terminated */
	s->input.pid_child      = 0;
	s->input.result         = 0;
	for(i = 0; i < history_size; i++) {
		s->history[i].no = 0;
		s->history[i].result = R_INVALID;
	}
	s->no_jobs        = 0;
	fprintf(stderr, "Simple: new, cmd no %d #%p. This is %sthe authoritative copy.\n",
			s->command_no, (void *)s, simple ? "not " : "");
	if(!simple) simple = s;

	return s;
}

/** @param s_ptr a reference to the Simple that is to be deleted */
void Simple_(struct Simple **s_ptr) {
	struct Simple *s;
	
	if(!s_ptr || !(s = *s_ptr)) return;
	fprintf(stderr, "~Simple: erase, #%p.\n", (void *)s);
	if(s == simple) simple = 0;
	free(s);
	*s_ptr = s = 0;
}

/** enumerates the history in stdout of the authoritative Simple */
void SimpleHistory(void) {
	int i, j, first;
	struct Input *h;

	if(!simple) return;
	printf("no\tid\tbackgrd\tcommand\n");
	for(i = 0; i < history_size; i++) {
		h = &simple->history[i];
		if(!h->no) continue;
		printf("%d:\t%d\t%s\t", i, h->no, h->result & R_BACKGROUND ? "yes" : "no");
		first = -1;
		for(j = 0; j < input_args; j++) {
			if(!first) {
				printf(" ");
			} else {
				first = 0;
			}
			if(!h->args[j]) break;
			printf("%s", h->args[j]);
		}
		printf("\n");
	}
}

/** redoes a command from the authoritative Simple
 @param arg      the first few letters of the command
 @param exec_ptr if it gets to this, whatever execute_input returned
 @return         non-zero on success */
int SimpleRedo(const char *arg, int *exec_ptr) {
	int i, no, exec;
	struct Input *selected;

	if(!simple) return 0;
	/* select the operation */
	if(!arg) {
		/* select the greatest command no */
		int greatest = -1;
		no = 0;
		for(i = 0; i < history_size; i++) {
			if(simple->history[i].no <= no) continue;
			no = simple->history[i].no;
			greatest = i;
		}
		if(greatest == -1) return 0;
		selected = &simple->history[greatest];
	} else {
		int len = strlen(arg);
		int found = -1;

		no = 0;
		/* fixme: also make an index */
		for(i = 0; i < history_size; i++) {
			/* flag 0 not a history */
			if(!simple->history[i].no) continue;
			/* not a partial match */
			if(strncmp(arg, simple->history[i].args[0], len)) continue;
			/* no is already pointing to a newer one */
			if(no >= simple->history[i].no) continue;
			/* it's good */
			no = simple->history[i].no;
			found = i;
		}
		if(found < 0) {
			fprintf(stderr, "Didn't find '%s.'\n", arg);
			return 0;
		}
		selected = &simple->history[found];
	}
	fprintf(stderr, "Redo: %s\n", selected->args[0]);

	setup_redo(simple, selected);

	exec = execute_input(&simple->input);
	if(exec_ptr) *exec_ptr = exec;

	return -1;
}

/** prints out jobs of the authoritative Simple (fixme: make callback, really) */
void SimpleJobs(void) {
	int i;

	if(!simple) return;
	printf("pid\tname\n");
	for(i = 0; i < simple->no_jobs; i++) {
		printf("%d\t%s\n", simple->job[i].pid, simple->job[i].name);
	}
}

/** tries to put it in the fg
 @param arg a pid or name or null in which case it will do the last one */
void SimpleForground(const char *arg) {
	/* ------------------- fixme!!! ----------------- */
	fprintf(stderr, "Simple::Forground: Check back later.\n");
}

/* private */

/** entry point
 @param argc the number of arguments starting with the programme name
 @param argv the arguments
 @return     either EXIT_SUCCESS or EXIT_FAILURE */
int main(int argc, char **argv) {
	int exit;
	struct Simple *simple;

	/* no command line switches */
	if(argc > 1) {
		usage();
		return EXIT_SUCCESS;
	}

	if(!(simple = Simple())) return EXIT_FAILURE;

	/* so we have different random names */
	srand(clock());

	for( ; ; ) { /* Program terminates normally inside setup */
		int i;

		/** prints history debug aaarrrrggh */
		{
			int i;

			printf("history: ");
			for(i = 0; i < history_size; i++) {
				if(simple->history[i].no) printf("%s:", simple->history[i].args[0]);
			}
			printf("\n");
		}
		printf(";}-> ");

		/* get next command */
		if(!setup(simple)) break;

		/* we'll just check them every time a command is entered;
		 seems like a good time */
		check_background_processes(simple);

		/* if it's a blank line */
		if(!simple->input.args[0]) continue;

		/* debug */
		for(i = 0; simple->input.args[i]; i++) fprintf(stderr, "args[%d] <%s>\n", i, simple->input.args[i]);

		/* execute! */
		if(!execute_input(&simple->input)) break;

		/* keep track of children */
		if(simple->input.pid_child) {
			if(simple->no_jobs < job_size) {
				struct Job *j = &simple->job[simple->no_jobs++];

				random_name(j->name, simple->input.args[0]);
				j->pid = simple->input.pid_child;
				fprintf(stderr, "Child buffer stored pid %d dubbed %s.\n", j->pid, j->name);
			} else {
				fprintf(stderr, "Error: hit maximum %d processes running in background; the process was forgotten!\n", job_size);
			}
		}

		/* add history */
		if((simple->input.result & R_SUCCESS)) make_history(simple);

	}

	/* specifically, R_SUCCESS is set by "exit" */
	exit = (simple->input.result & R_SUCCESS) ? EXIT_SUCCESS : EXIT_FAILURE;

	Simple_(&simple);

	return exit;
}

/** "setup() reads in the next command line, separating it into distinct tokens
 * using whitespace as delimiters. setup() sets the args parameter as a
 * null-terminated string."
 resets the input on a Simple and reads stdin into it
 @param s a valid Simple
 @return non-zero on success */
static int setup(struct Simple *s) {
	int i;
	char *tok;

	/* "examine every character in the inputBuffer" */
	/* no. */

	/* clear the possibly used bits from last time */
	s->input.result    = 0;
	s->input.pid_child = 0;
	/* yay ANSI */
	if(!fgets(s->input.inputBuffer, input_size, stdin)) {
		s->input.result = R_INVALID;
		return 0;
	}
	/* strsep the command into args; ignores args >= input_args */
	for(tok = strtok(s->input.inputBuffer, delimiters), i = 0;
		tok && i < input_args - 1;
		tok = strtok(0, delimiters)) {
		s->input.args[i++] = tok;
	}
	s->input.args[i]    = 0; /* null-terminated */
	/* if the last char is an '&,' set background */
	if(i > 0) {
		int len;
		char *last, *ch;

		last = s->input.args[i - 1];
		len  = strlen(last);
		/* assert(len > 0); */
		ch   = last + len - 1;
		if(*ch == background) {
			if(len <= 1) { /* foo & */
				i--;
				s->input.args[i] = 0;
			} else {       /* foo& */
				*ch = '\0';
			}
			s->input.result |= R_BACKGROUND;
		}
		/* advance the counter */
		s->input.no = s->command_no++;
	}

	return -1;
}

/** sets up a new command to be from the history
 @param s        a valid simple
 @param selected a history value */
static void setup_redo(struct Simple *s, const struct Input *selected) {
	int delta, i;
	struct Input *input = &s->input;

	/* fixme: should create a new id, right? */

	memcpy(input, selected, sizeof(struct Input));
	/* only keep these */
	input->result &= R_BACKGROUND | R_INVALID;
	input->pid_child = 0;
	/* this is a hack to get the pointers adjusted */
	delta = selected->inputBuffer - input->inputBuffer;
	for(i = 0; i < input_args; i++) {
		if(input->args[i]) input->args[i] -= delta;
	}
}

/** "the steps are:
 (1) fork a child process using fork()
 (2) the child process will invoke execvp()
 (3) if background == 1, the parent will wait,
 otherwise returns to the setup() function."
 I assume the other way around; this is overly-complex? why would you
 fork if you didn't need to run in the background? whatever
 @param input           a valid input
 @return                non-zero on success */
static int execute_input(struct Input *input) {
	int pid_child;
	int (*cmd)(char **, int *);

	if(input->result & R_INVALID) {
		fprintf(stderr, "Tried to run input that's flagged as invalid.\n");
		return 0;
	} else if((cmd = CommandSearch(input->args[0]))) {
		int exit = 0;

		/* check for built-in commands with binary search */
		input->result |= R_BUILTIN;
		input->result &= ~R_BACKGROUND;
		if((*cmd)(input->args, &exit)) input->result |= R_SUCCESS; 
		return exit ? 0 : -1;
	} else if((pid_child = fork()) == -1) {
		/* messed up -- get out */
		perror(input->args[0]);
		input->result |= R_FORK_ERROR;
		return 0;
	} else if(pid_child) {
		int exit_status;
		int running;

		/* this is the parent */
		fprintf(stderr, "Parent: created child, pid %d, %sground.\n",
				pid_child, input->result & R_BACKGROUND ? "back" : "fore");
		if(!(input->result & R_BACKGROUND)) {
			/* foreground */
			if(!wait_child(pid_child, 0, &running, &exit_status)) {
				input->result |= R_ABNORMAL;
				return 0;
			}
			input->result |= (exit_status == EXIT_SUCCESS) ? R_SUCCESS : R_FAILURE;
		} else {
			/* background -- don't know whether it's good, but at least we
			 could check if it's stoped immediately and not keep track;
			 probably the programme exited already */
			if(!wait_child(pid_child, -1, &running, &exit_status)) {
				fprintf(stderr, "Ignoring background request.\n");
				input->result |= R_SUCCESS;
				return -1;
			}
			if(running) {
				input->pid_child = pid_child;
				input->result |= R_SUCCESS;
				fprintf(stderr, "Job will be buffered.\n");
			} else {
				input->result |= R_ABNORMAL;
				fprintf(stderr, "Not running.\n");
			}
		}
	} else {
		/* this is the child */
		fprintf(stderr, "Child: execute %s.\n", simple->input.args[0]);
		/* execvp does not return exept if error (horrible design) */
		/* fixme: memcpy(in, simple->input, sizeof(struct Input));*/
		if(execvp(simple->input.args[0], simple->input.args) == -1) {
			perror(simple->input.args[0]);
			input->result |= R_EXEC_ERROR;
			return 0;
		}
	}
	return -1;
}

/** waits for the process to terminate on the child
 @param pid_child          the pid of the process
 @param running_ptr        if success, whether running
 @param exit_condition_ptr if not running, what the exit condition was
 @return                   non-zero on success */
static int wait_child(const int pid_child, const int bg, int *running_ptr, int *exit_condition_ptr) {
	int status;
	int wait;

	if((wait = waitpid(pid_child, &status, bg ? WNOHANG : 0)) == -1) {
		perror("waitpid");
		return 0;
	}

	*running_ptr = -1;

	/* pluging along in the background */
	if(bg && !status) return -1;

	*running_ptr = 0;

	/* fixme: it's a bit more complicated then this */
	if(!WIFEXITED(status)) {
		fprintf(stderr, "Parent: exit %d with no exit status.\n", pid_child);
		return 0;
	}
	*exit_condition_ptr = WEXITSTATUS(status);
	fprintf(stderr, "Parent: exit %d with status %d.\n", pid_child, *exit_condition_ptr);

	return -1;
}

/** checks for exits from background processes; cleans up
 @param s a valid Simple */
static void check_background_processes(struct Simple *s) {
	int i, max;
	int status;
	int wait;
	int pid_child;

	for(i = 0; i < s->no_jobs; i++) {
		pid_child = s->job[i].pid;
		fprintf(stderr, "Check background pid %d, %s.\n", pid_child, s->job[i].name);
		if((wait = waitpid(pid_child, &status, WNOHANG)) == -1) {
			perror("waitpid");
			continue;
		}
		/* I'm scetchy on this one; I think something on the internet says if
		 waitpid with WNOHANG detects nothing, this will always be zero */
		if(!wait) continue;
		/* signal due to anything */
		if(WIFEXITED(status)) {
			int exit_status = WEXITSTATUS(status);

			fprintf(stderr, "Background process %d exited with status %d.\n", pid_child, exit_status);
		} else {
			fprintf(stderr, "Background process %d exited with no exit status.\n", pid_child);
		}
		/* remove it from the list; apparently it exited */
		max = s->no_jobs - 1;
		if(i < max) memcpy(&s->job[i], &s->job[max], sizeof(struct Job));
		s->no_jobs--;
		i--;
	}
}

/** puts an entry in the history
 @param s a valid Simple */
static void make_history(struct Simple *s) {
	int delta;
	int i;
	int min_no = 0, cur_no;
	int cur_index = 0;
	struct Input *replace, *current;

	/* find the most appropreate spot, cur_index */
	for(i = 0; i < history_size; i++) {
		cur_no = s->history[i].no;
		if(cur_no == 0)     { cur_index = i; break; }
		if(cur_no < min_no) { cur_index = i; min_no = cur_no; }
	}
	replace = &s->history[cur_index];
	current = &s->input;
	memcpy(replace, current, sizeof(struct Input));
	/* this is a hack to get the pointers adjusted */
	delta = replace->inputBuffer - current->inputBuffer;
	for(i = 0; i < input_args; i++) {
		if(replace->args[i]) replace->args[i] += delta;
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

/** come up with random names
 @param name the string that's going to hold it, must be at least
             5 + 2*max(words) + max(suffixes) + 1 long (cur 16)
 @param seed inside the name */
static void random_name(char *name, const char *seed) {
	int i;

	i = rand() % words_size;
	strcpy(name, words[i]);
	strncat(name, seed, 2); /* 5 is too long */
	/*i = rand() % words_size;
	strcat(name, words[i]); too long */
	i = rand() % suffixes_size;
	strcat(name, suffixes[i]);
}
