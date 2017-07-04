/*************** YOU SHOULD NOT MODIFY ANYTHING IN THIS FILE ***************/

#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "testrunner.h"
#include "assignment2_tests.h"

#define quit_if(cond) do {if (cond) exit(EXIT_FAILURE);} while(0)

/* Prepare input, reroute file descriptors, and run the program. */
void run_test(const char *input, int argc, char **argv)
{
	/* Prepare input */
	FILE *in = fopen("assignment2.in", "w");
	fprintf(in, input);
	fclose(in);
	/* Reroute standard file descriptors */
	freopen("assignment2.in",  "r", stdin );
	freopen("assignment2.out", "w", stdout);
	freopen("assignment2.err", "w", stderr);
	/* Run the program */
	quit_if(main(argc, argv) != EXIT_SUCCESS);
	fclose(stdout);
	fclose(stderr);
}

/* P5.1: Test of executing commands in the path */
int test_path(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out, *err;
	int pid_tmp;
	/* Run the test */
	run_test("ls\n/bin/ls\nexit\n", 1, args);
	/* Check output */
	err = fopen("assignment2.err", "r");
	quit_if(fscanf(err, "  Parent says 'child process has been forked with pid=%d'\n"
	                    "  Parent says 'wait() returned so the child with pid=%d is finished.'\n"
	                    "  Parent says 'child process has been forked with pid=%d'\n"
	                    "  Parent says 'wait() returned so the child with pid=%d is finished.'\n",
	               &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp) != 4);
	quit_if(!feof(err));
	fclose(err);
	return EXIT_SUCCESS;
}

/* P5.2: Test of command line counter */
int test_counter(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out, *err;
	int pid_tmp;
	/* Run the test */
	run_test("\n/bin/true\nexit\n", 1, args);
	/* Check output */
	out = fopen("assignment2.out", "r");
	quit_if(fscanf(out, "Shell(pid=%d)1> Shell(pid=%d)1> Shell(pid=%d)2> Exiting process %d\n", &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp) != 4);
	quit_if(!feof(out));
	fclose(out);
	return EXIT_SUCCESS;
}

/* P5.3: Test of re-executing earlier commands */
int test_rerun(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out, *err;
	int pid_tmp;
	/* Run the test */
	run_test("/bin/echo test\n!1\nexit\n", 1, args);
	/* Check output */
	out = fopen("assignment2.out", "r");
	quit_if(fscanf(out, "Shell(pid=%d)1> test\nShell(pid=%d)2> test\nShell(pid=%d)3> Exiting process %d\n", &pid_tmp, &pid_tmp, &pid_tmp, &pid_tmp) != 4);
	quit_if(!feof(out));
	fclose(out);
	return EXIT_SUCCESS;
}

/* P5.5: Test of depth-limited sub */
int test_sub(int argc, char **argv)
{
	char *args[] = { "./shell", NULL };
	FILE *out, *err;
	int pids[4], warned_too_deep;
	/* Run the test */
	run_test("sub\nsub\nsub\nexit\nexit\nexit\n", 1, args);
	/* Check output */
	out = fopen("assignment2.out", "r");
	err = fopen("assignment2.err", "r");
	/* First, check that the subshells were invoked. */
	fscanf(out, "Shell(pid=%d)1> Shell(pid=%d)1> Shell(pid=%d)1> Shell(pid=%d)2> ", &pids[0], &pids[1], &pids[2], &pids[3]);
	quit_if(!((pids[0] != pids[1]) && (pids[1] != pids[2]) && (pids[0] != pids[2]) && (pids[2] == pids[3])));
	/* Next, check for the "Too deep!" message: */
	warned_too_deep = 0;
	/* Use a while loop because multiple processes write to stderr concurrently. */
	while (!warned_too_deep && !feof(err)) {
		char too_deep[11];
		fgets(too_deep, 11, err);
		if (!strncmp(too_deep, "Too deep!\n", 10))
			warned_too_deep = 1;
	}
	quit_if(!warned_too_deep);
	fclose(out);
	fclose(err);
	return EXIT_SUCCESS;
}

/*
 * Main entry point for ASSIGNMENT2 test harness
 */
int run_assignment2_tests(int argc, char **argv)
{
	/* Tests can be invoked by matching their name or their suite name 
	   or 'all' */
	testentry_t tests[] = {
		{ "sub",     "assignment2", test_sub     },
		{ "rerun",   "assignment2", test_rerun   },
		{ "counter", "assignment2", test_counter },
		{ "path",    "assignment2", test_path    } };
	int result = run_testrunner(argc, argv, tests, sizeof(tests) / sizeof (testentry_t));
	unlink("assignment2.in");
	unlink("assignment2.out");
	unlink("assignment2.err");
	return result;
}
