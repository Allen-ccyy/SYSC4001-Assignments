/*************** YOU SHOULD NOT MODIFY ANYTHING IN THIS FILE ***************/

#define _GNU_SOURCE
#include <stdio.h>
#undef _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "testrunner.h"
#include "assignment1_test.h"

#define quit_if(cond) do {if (cond) exit(EXIT_FAILURE);} while(0)

/* test of -h switch behavior (B3) */
int test_help_switch(int argc, char **argv)
{
    char *args[] = {"./main", "-h", NULL};
    FILE *out, *err, *tmp;
    char buffer[100];

    freopen("/dev/null", "r", stdin);
    freopen("assignment1.out", "w", stdout);
    freopen("assignment1.err", "w", stderr);
    quit_if(main(2, args) != EXIT_FAILURE);
    fclose(stdout);
    fclose(stderr);

    out = fopen("assignment1.out", "r");
    err = fopen("assignment1.err", "r");
    if (fgets(buffer, 100, out) != NULL && !strncmp(buffer, "usage:", 6)) {
      tmp = out;
    }
    else {
      quit_if(fgets(buffer, 100, err) == NULL);
      quit_if(strncmp(buffer, "usage:", 6));
      tmp = err;
    }
    if (fgets(buffer, 100, tmp) != NULL) {
      quit_if(!strcmp(buffer, "./main: Invalid option -h. Use -h for help.\n"));
    }
    fclose(out);
    fclose(err);
    return EXIT_SUCCESS;
}

/* test of basic functionality (B4, B5) */
int test_basic_functionality(int argc, char **argv)
{
    char *args[] = {"./main", "cat", "dog", "nap", NULL};
    char *result[] = {"Looking for 3 words\n",
      "Result:\n",
      "cat:1\n",
      "dog:0\n",
      "nap:0\n"};
    FILE *out;
    int i;
    char buffer[100];

    out = fopen("assignment1.in", "w");
    fprintf(out, "cat\n");
    fprintf(out, ".\n");
    fclose(out);
    freopen("assignment1.in", "r", stdin);
    freopen("assignment1.out", "w", stdout);
    quit_if(main(4, args) != EXIT_SUCCESS);
    fclose(stdin);
    fclose(stdout);

    out = fopen("assignment1.out", "r");
    for (i = 0; i < 5; i++) {
      quit_if(fgets(buffer, 100, out) == NULL);
      quit_if(strcmp(buffer, result[i]));
    }
    fclose(out);
    return EXIT_SUCCESS;
}

/* test of stderr output support (C1) */
int test_stderr_output(int argc, char **argv)
{
    char *args[] = {"./main", "-wrong", NULL};
    char *result[] = {"./main: Invalid option -wrong. Use -h for help.\n",
      "./main: Please supply at least one word. Use -h for help.\n"};
    FILE *err;
    int i;
    char buffer[100];

    freopen("/dev/null", "r", stdin);
    freopen("/dev/null", "w", stdout);
    freopen("assignment1.err", "w", stderr);
    quit_if(main(2, args) != EXIT_FAILURE);
    fclose(stderr);

    err = fopen("assignment1.err", "r");
    for (i = 0; i < 2; i++) {
      quit_if(fgets(buffer, 100, err) == NULL);
      quit_if(strcmp(buffer, result[i]));
    }
    fclose(err);
    return EXIT_SUCCESS;
}

/* test of -fFILENAME switch behavior (C2) */
int test_file_output(int argc, char **argv)
{
    char *args[] = {"./main", "-fassignment1.out", "cat", "dog", "nap", NULL};
    char *result[] = {"Looking for 3 words\n",
      "Result:\n",
      "cat:1\n",
      "dog:0\n",
      "nap:0\n"};
    FILE *out;
    int i;
    char buffer[100];

    out = fopen("assignment1.in", "w");
    fprintf(out, "cat\n");
    fprintf(out, ".\n");
    fclose(out);
    freopen("/dev/null", "w", stdout);
    freopen("assignment1.in", "r", stdin);
    quit_if(main(5, args) != EXIT_SUCCESS);
    fcloseall();

    quit_if((out = fopen("assignment1.out", "r")) == NULL);
    for (i = 0; i < 5; i++) {
      quit_if(fgets(buffer, 100, out) == NULL);
      quit_if(strcmp(buffer, result[i]));
    }
    fclose(out);
    return EXIT_SUCCESS;
}

/* test of supporting an arbitrary number of words (C3) */
int test_malloc(int argc, char **argv)
{
    char *args[] = {"./main", "cat", "dog", "nap", "c", "a", "t", NULL};
    char *result[] = {"Looking for 6 words\n",
      "Result:\n",
      "cat:1\n",
      "dog:0\n",
      "nap:0\n",
      "c:0\n", "a:0\n", "t:0\n"};
    FILE *out;
    int i;
    char buffer[100];
    quit_if(system("grep malloc main.c > /dev/null"));
    out = fopen("assignment1.in", "w");
    fprintf(out, "cat\n");
    fprintf(out, ".\n");
    fclose(out);
    freopen("assignment1.in", "r", stdin);
    freopen("assignment1.out", "w", stdout);
    quit_if(main(7, args) != EXIT_SUCCESS);
    fclose(stdin);
    fclose(stdout);
    out = fopen("assignment1.out", "r");
    for (i = 0; i < 8; i++) {
      quit_if(fgets(buffer, 100, out) == NULL);
      quit_if(strcmp(buffer, result[i]));
    }
    fclose(out);
    return EXIT_SUCCESS;
}

/* test of fgets usage (C4) */
int test_fgets(int argc, char **argv)
{
    quit_if(system("grep fgets main.c > /dev/null"));
    return EXIT_SUCCESS;
}

/* test of multiple words per line support (C5) */
int test_strtok(int argc, char **argv)
{
    char *args[] = {"./main", "cat", "dog", "nap", NULL};
    char *result[] = {"Looking for 3 words\n",
      "Result:\n",
      "cat:1\n",
      "dog:2\n",
      "nap:1\n"};
    FILE *out;
    int i;
    char buffer[100];
    out = fopen("assignment1.in", "w");
    fprintf(out, "cat\n");
    fprintf(out, "dog dog nap\n");
    fprintf(out, ".\n");
    fclose(out);
    freopen("assignment1.in", "r", stdin);
    freopen("assignment1.out", "w", stdout);
    quit_if(main(4, args) != EXIT_SUCCESS);
    fclose(stdin);
    fclose(stdout);
    out = fopen("assignment1.out", "r");
    for (i = 0; i < 5; i++) {
      quit_if(fgets(buffer, 100, out) == NULL);
      quit_if(strcmp(buffer, result[i]));
    }
    fclose(out);
    return EXIT_SUCCESS;
}

void delete_temp_files()
{
  unlink("assignment1.in");
  unlink("assignment1.out");
  unlink("assignment1.err");
}

/*
 * Main entry point for ASSIGNMENT1 test harness
 */
int run_assignment1_test(int argc, char **argv)
{
    /* Tests can be invoked by matching their name or their suite name
       or 'all'*/
    testentry_t tests[] = {
      {"help_switch", "suite1", test_help_switch},
      {"basic_functionality", "suite1",
        test_basic_functionality},
      {"stderr_output", "suite1", test_stderr_output},
      {"file_output", "suite1", test_file_output},
      {"malloc", "suite1", test_malloc},
      {"fgets", "suite1", test_fgets},
      {"strtok", "suite1", test_strtok}};
    atexit(delete_temp_files);
    return run_testrunner(argc, argv, tests, sizeof(tests) / sizeof (testentry_t));
}
