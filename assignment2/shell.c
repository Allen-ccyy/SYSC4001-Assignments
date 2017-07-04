/* ASSIGNMENT2: Simple Shell */

/* LIBRARY SECTION */
#include <ctype.h>              /* Character types                       */
#include <stdio.h>              /* Standard buffered input/output        */
#include <stdlib.h>             /* Standard library functions            */
#include <string.h>             /* String operations                     */
#include <sys/types.h>          /* Data types                            */
#include <sys/wait.h>           /* Declarations for waiting              */
#include <unistd.h>             /* Standard symbolic constants and types */
//#include "shell.h"
#include "assignment2_tests.h"  /* Built-in test system                  */

/* DEFINE SECTION */
#define SHELL_BUFFER_SIZE 256   /* Size of the Shell input buffer        */
#define SHELL_MAX_ARGS 8        /* Maximum number of arguments parsed    */
#define SHELL_MAX_HISTORY 10    /* Maximum number of commands saved      */

/* VARIABLE SECTION */
enum { STATE_SPACE, STATE_NON_SPACE };	/* Parser states */
//char *PATH = getenv("PATH");
char program_path[124];
static int subshell = 0;
int imthechild(const char *path_to_exec, char *const args[])
{
	//If the path passed can be executed as is, do so.
	if (access(path_to_exec, X_OK) != -1 ) {
		return execv(path_to_exec, args) ? -1 : 0;
	} 	
	
	//If it gets here then search the PATH var to see if we can find the executable we are trying to run. 	
	//getenv("PATH") will read in the current users path variable, it is : seperated.
	//strtok will tokenize the PATH based on the :, the loop will go through every directory in the path. 
	for (char *p = strtok(getenv("PATH"), ":"); p != NULL; p = strtok(NULL, ":")){
		//create a temp string to build up a path to the potential exe.
		char var_path_to_exec[1024];
		strcpy(var_path_to_exec, p);
		strcat(var_path_to_exec, "/");
		strcat(var_path_to_exec, path_to_exec);
		//check if this is an executable path, if so run it.
		if (access(var_path_to_exec, X_OK) != -1 ) {
			return execv(var_path_to_exec, args) ? -1 : 0;
		} 
	}
	//if nothing is found, attempt to run in pwd and return the error
	return execv(path_to_exec, args) ? -1 : 0;
}

void imtheparent(pid_t child_pid, int run_in_background)
{
	int child_return_val, child_error_code;

	/* fork returned a positive pid so we are the parent */
	fprintf(stderr,
	        "  Parent says 'child process has been forked with pid=%d'\n",
	        child_pid);
	if (run_in_background) {
		fprintf(stderr,
		        "  Parent says 'run_in_background=1 ... so we're not waiting for the child'\n");
		return;
	}
	wait(&child_return_val);
	/* Use the WEXITSTATUS to extract the status code from the return value */
	child_error_code = WEXITSTATUS(child_return_val);
	fprintf(stderr,
	        "  Parent says 'wait() returned so the child with pid=%d is finished.'\n",
	        child_pid);
	if (child_error_code != 0) {
		/* Error: Child process failed. Most likely a failed exec */
		fprintf(stderr,
		        "  Parent says 'Child process %d failed with code %d'\n",
		        child_pid, child_error_code);
	}
}

/* MAIN PROCEDURE SECTION */
int main(int argc, char **argv)
{
	pid_t shell_pid, pid_from_fork;
	int n_read, i, exec_argc, parser_state, run_in_background;
	/* buffer: The Shell's input buffer. */
	char buffer[SHELL_BUFFER_SIZE];
	/* exec_argv: Arguments passed to exec call including NULL terminator. */
	char *exec_argv[SHELL_MAX_ARGS + 1];
	//Counter for line numbers
	int counter = 1;
	//history stores the commands in the order they were entered
	char history[SHELL_MAX_HISTORY][SHELL_MAX_ARGS + 1][256];
	//historyc holds the number of arguments
	int historyc[SHELL_MAX_HISTORY];

	/* Entrypoint for the testrunner program */
	if (argc > 1 && !strcmp(argv[1], "-test")) {
		return run_assignment2_tests(argc - 1, argv + 1);
	}
	/* Allow the Shell prompt to display the pid of this process */
	shell_pid = getpid();
	while (1) {
	/* The Shell runs in an infinite loop, processing input. */

		fprintf(stdout, "Shell(pid=%d)%d> ", shell_pid, counter);
		fflush(stdout);
		
		/* Read a line of input. */
		if (fgets(buffer, SHELL_BUFFER_SIZE, stdin) == NULL)
			return EXIT_SUCCESS;
		n_read = strlen(buffer);
		run_in_background = n_read > 2 && buffer[n_read - 2] == '&';
		buffer[n_read - run_in_background - 1] = '\n';

		/* Parse the arguments: the first argument is the file or command *
		 * we want to run.                                                */

		parser_state = STATE_SPACE;
		for (exec_argc = 0, i = 0;
		     (buffer[i] != '\n') && (exec_argc < SHELL_MAX_ARGS); i++) {

			if (!isspace(buffer[i])) {
				if (parser_state == STATE_SPACE)
					exec_argv[exec_argc++] = &buffer[i];
				parser_state = STATE_NON_SPACE;
			} else {
				buffer[i] = '\0';
				parser_state = STATE_SPACE;
			}
		}

		/* run_in_background is 1 if the input line's last character *
		 * is an ampersand (indicating background execution).        */


		buffer[i] = '\0';	/* Terminate input, overwriting the '&' if it exists */

		/* If no command was given (empty line) the Shell just prints the prompt again */
		if (!exec_argc)
			continue;
		/* Terminate the list of exec parameters with NULL */
		exec_argv[exec_argc] = NULL;

		//check if the first char in the first argument is an !, if so we need to access history 
		 if (exec_argv[0][0] == '!')
		 {
		 	//figure out which command we are recalling from history
		 	int cmdnum = exec_argv[0][1] - 0x30;
			//check to ensure that this is a valid command, if not print 'Not Valid' and move on.
		 	if(cmdnum >= counter)
		 	{
		 		fprintf(stderr, "Not Valid\n");
		 		continue;
		 	}
		 	//set the exec_arg variables equal to the history command that was requested and continue to execute as normal
			exec_argc = historyc[cmdnum-1];
		 	for (int i = 0; i < historyc[cmdnum-1]; i++)
			 {
			 	exec_argv[i] = (char *) &history[cmdnum-1][i];
			 }
		 }
		//if < 10 lines have gone by then we need to add the command being run to the history variables
		if(counter < SHELL_MAX_HISTORY)
		{
			historyc[counter-1] = exec_argc;
			for (int i = 0; i < exec_argc; i++)
			{
				strcpy(history[counter-1][i], exec_argv[i]);
			}
		}
		//If we get here there is a command to be processed, increment the counter
		counter += 1;
		/* If Shell runs 'exit' it exits the program. */
		if (!strcmp(exec_argv[0], "exit")) {
			printf("Exiting process %d\n", shell_pid);
			return EXIT_SUCCESS;	/* End Shell program */

		} else if (!strcmp(exec_argv[0], "cd") && exec_argc > 1) {
		/* Running 'cd' changes the Shell's working directory. */
			/* Alternative: try chdir inside a forked child: if(fork() == 0) { */
			if (chdir(exec_argv[1]))
				/* Error: change directory failed */
				fprintf(stderr, "cd: failed to chdir %s\n", exec_argv[1]);	
			/* End alternative: exit(EXIT_SUCCESS);} */
		} else if (!strcmp(exec_argv[0], "sub")) {
			//special case for sub command, if it is <2 then we can run it it, otherwise echo error. 
			if(subshell < 2){
					subshell++; //increment the subshell counter variable
					pid_from_fork = fork();//fork, the parrent will be left waiting for the child, child will be the subshell 
				if (pid_from_fork < 0) {
					/* Error: fork() failed.  Unlikely, but possible (e.g. OS *
					 * kernel runs out of memory or process descriptors).     */
					fprintf(stderr, "fork failed\n");
					continue;
				}
				if (pid_from_fork == 0) {
					main(argc, argv);
					/* Exit from main. */
				} else {
					imtheparent(pid_from_fork, run_in_background);
					/* Parent will continue around the loop. */
				}
			} else {
				fprintf(stderr, "Too deep!\n");
			}
		} else {
		/* Execute Commands */
			pid_from_fork = fork();

			if (pid_from_fork < 0) {
				/* Error: fork() failed.  Unlikely, but possible (e.g. OS *
				 * kernel runs out of memory or process descriptors).     */
				fprintf(stderr, "fork failed\n");
				continue;
			}
			if (pid_from_fork == 0) {
				return imthechild(exec_argv[0], &exec_argv[0]);
				/* Exit from main. */
			} else {
				imtheparent(pid_from_fork, run_in_background);
				/* Parent will continue around the loop. */
			}
		} /* end if */
	} /* end while loop */

	return EXIT_SUCCESS;
} /* end main() */
