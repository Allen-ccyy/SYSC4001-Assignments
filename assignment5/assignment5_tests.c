/*************** YOU SHOULD NOT MODIFY ANYTHING IN THIS FILE ***************/
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include "testrunner.h"
#include "fileio.h"
#include "util.h"

/* CONSTANTS */
/* These constants work in tandem with test data created by setup.sh */

#define TESTDATA_DIR "testdata/"
#define NEW_FILE "testdata/newfile.txt"
#define OLD_FILE "testdata/oldfile.txt"
#define HELLO_FILE  "testdata/hello.txt"
#define BIG_FILE  "testdata/bigfile"
#define NO_SUCH_FILE "testdata/no_such_file"
#define NEW_DIR "testdata/newdir"
#define SUBDIR1_DIR "testdata/1/2"

extern int bonnie_main(int, const char **);

/* Remove testdata subdirectory (assumes that the current directory has not
 changed. */
void teardown()
{
    if (system("./teardown.sh"))
	fprintf(stderr, "\nteardown failed\n");
}

/* Create test data and register the clean up function */
void setup()
{
    quit_if(system("./setup.sh"));
    quit_if(atexit(teardown));
    errno = 0;
}


/* ---------- Suite 1 Tests --------------- */
/* test file_read() */
int test_file_read(int argc, const char **argv)
{
    char b[255], dots[sizeof(b)];
    int bytes_read, i;
    setup();
    quit_if(IOERR_INVALID_ARGS != file_read(HELLO_FILE, 0, b, 0));
    quit_if(IOERR_INVALID_ARGS !=
	    file_read(HELLO_FILE, 0, NULL, sizeof(b)));
    quit_if(IOERR_INVALID_ARGS != file_read(HELLO_FILE, -1, b, sizeof(b)));
    quit_if(IOERR_INVALID_ARGS != file_read(NULL, 0, b, sizeof(b)));
    quit_if(IOERR_INVALID_PATH !=
	    file_read(NO_SUCH_FILE, 0, b, sizeof(b)));
    bytes_read = file_read(HELLO_FILE, 0, b, sizeof(b));
    quit_if(bytes_read != 10);
    quit_if(strncmp("HelloWorld", b, 10));
    bytes_read = file_read(HELLO_FILE, 5, b, sizeof(b));
    quit_if(bytes_read != 5);
    quit_if(strncmp("World", b, 5));
    bytes_read = file_read(HELLO_FILE, -1, b, sizeof(b));
    quit_if(bytes_read >= 0);

    for (i = 0; i < sizeof(dots); i++) {
	b[i] = dots[i] = i == sizeof(dots) - 1 ? '\0' : '.';
    }

    bytes_read = file_read(BIG_FILE, 1, b, sizeof(b));
    quit_if(bytes_read != sizeof(b));
    quit_if(!memcmp(b, dots, sizeof(b)));

    return 0;
}

/* Test file_info() */
int test_file_info(int argc, const char **argv)
{
    char b[255], ftype[2];
    int sz, params;
    long accessed, modified;
    char fmt[] = "Size:%d Accessed:%ld Modified:%ld Type %s";

    setup();
    // Test invalid arguments
    quit_if(IOERR_INVALID_ARGS != file_info(HELLO_FILE, b, 0));
    quit_if(IOERR_INVALID_ARGS != file_info(HELLO_FILE, NULL, sizeof(b)));
    quit_if(IOERR_INVALID_ARGS != file_info(NULL, b, sizeof(b)));

    // Test stat on old file
    quit_if(file_info(OLD_FILE, b, sizeof(b)));
    params = sscanf(b, fmt, &sz, &accessed, &modified, &ftype);
    quit_if(params != 4);

    int one_day = 86400;
    int jan1_2005 = 1104534000;	// seconds since epoch
    int jan3_2005 = jan1_2005 + one_day + one_day;

    quit_if(modified < jan1_2005 || modified > jan1_2005 + one_day);
    quit_if(accessed < jan3_2005 || accessed > jan3_2005 + one_day);
    return 0;
}

/* Test file_write()  */
int test_file_write(int argc, const char **argv)
{
    char b[] = "qwertyuiop", b2[255];
    setup();
    // Test invalid arguments
    quit_if(IOERR_INVALID_ARGS !=
	    file_write(NEW_FILE, 0, NULL, sizeof(b)));
    quit_if(IOERR_INVALID_ARGS != file_write(NULL, 0, b, sizeof(b)));
    // Test new file
    quit_if(sizeof(b) != file_write(NEW_FILE, 0, b, sizeof(b)));

    quit_if(strlen(b) + 1 != file_read(NEW_FILE, 0, b2, sizeof(b2)));
    quit_if(strncmp(b, b2, strlen(b)));
    return 0;
}

/* ---------- Suite 3 Tests --------------- */
/* Test the higher level function file_create. */
int test_file_create(int argc, const char **argv)
{
    char pattern[] = "abc", b[255];
    int repeat = 5;
    char expected[] = "abcabcabcabcabc";
    setup();
    quit_if(file_create(NEW_FILE, pattern, repeat));
    quit_if(strlen(expected) != file_read(NEW_FILE, 0, b, sizeof(b)));
    quit_if(strncmp(expected, b, strlen(expected)));
    return 0;
}

/* Test file_removal. */
int test_file_remove(int argc, const char **argv)
{
    setup();
    quit_if(IOERR_INVALID_ARGS != file_remove(NULL));
    // Test correct removal
    quit_if(file_remove(OLD_FILE));
    // Cant remove twice
    quit_if(IOERR_INVALID_PATH != file_remove(OLD_FILE));
    return 0;
}

/* ---------- Suite 2 Tests --------------- */
/* Test dir_create (directory create)*/
int test_dir_create(int argc, const char **argv)
{
    setup();
    quit_if(IOERR_INVALID_ARGS != dir_create(NULL));
    quit_if(IOERR_INVALID_PATH != dir_create(HELLO_FILE));

    quit_if(dir_create(NEW_DIR));
    struct stat s;
    quit_if(stat(NEW_DIR, &s) == -1);
    quit_if(!S_ISDIR(s.st_mode))
	return 0;
}

/* Test dir_list (directory listing) */
int test_dir_list(int argc, const char **argv)
{
    char b[10000], *next_line, *last, names_found[10000];
    int entries;
    setup();
    quit_if(IOERR_INVALID_ARGS != dir_list(NULL, b, sizeof(b)));
    quit_if(IOERR_INVALID_ARGS != dir_list(HELLO_FILE, NULL, sizeof(b)));
    quit_if(IOERR_INVALID_ARGS != dir_list(HELLO_FILE, b, 0));
    quit_if(IOERR_INVALID_PATH != dir_list(NO_SUCH_FILE, b, sizeof(b)));
    quit_if(IOERR_INVALID_PATH != dir_list(HELLO_FILE, b, sizeof(b)));
    quit_if(IOERR_BUFFER_TOO_SMALL != dir_list(TESTDATA_DIR, b, 2));

    quit_if(dir_list(TESTDATA_DIR, b, sizeof(b)));
    quit_if(!memchr(b, 0, sizeof(b)));  // check terminating nul byte
   // printf("\nMAIN:\n%s\n", b);
    entries = 0;
    names_found[0] = '|';
    names_found[1] = '\0';
    next_line = strtok_r(b, "\n", &last);
    while (next_line) {
        int args_read;
        char name[256];
        entries++;
        args_read = sscanf(next_line, "%255s", name);

        quit_if(args_read != 1);
        quit_if(strlen(names_found) + strlen(name) + 2 >=
            sizeof(names_found));
        strcat(names_found, name);
        strcat(names_found, "|");
        next_line = strtok_r(NULL, "\n", &last);
    }
    quit_if(!strstr(names_found, "|1|"));
    quit_if(!strstr(names_found, "|.|"));
    quit_if(!strstr(names_found, "|..|"));
    quit_if(!strstr(names_found, "|hello.txt|"));
    quit_if(!strstr(names_found, "|hello2.txt|"));
    quit_if(!strstr(names_found, "|oldfile.txt|"));
    quit_if(!strstr(names_found, "|bigfile|"));
    quit_if(entries != 7);
    return 0;
}

/* ---------- Suite 4 Tests --------------- */
/* Test file_checksum */
int test_file_checksum(int argc, const char **argv)
{
    setup();
    char hello[] = "HelloWorld";
    unsigned short cksum = checksum(hello, strlen(hello), 0);
    quit_if(IOERR_INVALID_ARGS != file_checksum(NULL));
    quit_if(cksum != file_checksum(HELLO_FILE));
    return 0;
}

/* in setup.sh()
mkdir -p ./testdata/1/2/3/4/5/6/7/8/9/10/11/12/13/14/15/16/17/18/19/20
cp ./testdata/hello.txt ./testdata/1/2/3/4/5/6/7/8/9/10/
cp ./testdata/oldfile.txt ./testdata/1/2/3/4/5/6/7/8/9/

This checksum algorithm commutes so we can calculate the the final checksum as follows...
*/

/* Test dir_checksum */
int test_dir_checksum(int argc, const char **argv)
{
    setup();
    char dname[10];
    int i;
    unsigned short cksum;
    int cksum2;

    quit_if(IOERR_INVALID_ARGS != dir_checksum(NULL));
    cksum = checksum("OldFile", 7, checksum("HelloWorld", 10, 0));

    for (i = 3; i <= 20; i++) {
    	sprintf(dname, "%d", i);
    	cksum = checksum(dname, strlen(dname), cksum);
    }

    cksum2 = dir_checksum(SUBDIR1_DIR);
    quit_if(0 > cksum2);
    quit_if(cksum != cksum2);

    return 0;
}


/*
 * Main entry point for test harness
 */
int run_assignment5_tests(int argc, const char **argv)
{
    /* Tests can be invoked by matching their name or their suite name or 'all' */
    testentry_t tests[] = {
	{"read", "suite1", test_file_read},
	{"info", "suite1", test_file_info},
	{"write", "suite1", test_file_write},

	{"dirlist", "suite2", test_dir_list},
	{"dircreate", "suite2", test_dir_create},

	{"remove", "suite3", test_file_remove},
	{"create", "suite3", test_file_create},

	{"filechecksum", "suite4", test_file_checksum},
	{"dirchecksum", "suite4", test_dir_checksum}
    };
    return run_testrunner(argc, argv, tests,
			  sizeof(tests) / sizeof(testentry_t));
}

/* The real main function.  */
int main(int argc, const char **argv)
{
    if (argc > 1 && !strcmp(argv[1], "-test")) {
	return run_assignment5_tests(argc - 1, argv + 1);
    } else {
	return bonnie_main(argc, argv);
    }
}
