/*
 * This is a file system benchmark which attempts to study bottlenecks -
 * it is named 'Bonnie' after Bonnie Raitt, who knows how to use one.
 *
 * Commentary on Bonnie's operations may be found at
 * http://www.textuality.com/bonnie/intro.html
 *
 * COPYRIGHT NOTICE:
 * Copyright (c) Tim Bray, 1990-1996.
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#if defined(SysV)
#include <limits.h>
#include <sys/times.h>
#else
#include <sys/resource.h>
#endif
#include "fileio.h"
#define IntSize (sizeof(int))

/*
 * N.B. in seeker_reports, CPU appears and Start/End time, but not Elapsed,
 *  so position 1 is re-used; icky data coupling.
 */
#define CPU (0)
#define Elapsed (1)
#define StartTime (1)
#define EndTime (2)
#define Seeks (4000)
#define UpdateSeek (10)
#define SeekProcCount (3)
#define Chunk (16384)

/* labels for the tests, used as an array index */
typedef enum {
    Putc, ReWrite, FastWrite, Getc, FastRead, Lseek, TestCount
} tests_t;

static double cpu_so_far();
static void doseek(off_t where, int fd, int update);
static void get_delta_t(tests_t test);
static void io_error(char *message);
static void newfile(char *name, int *fd, FILE * *stream, int create);

static void fill_file_char();
static void fill_file_block();
static void file_read_rewrite_block();
static void file_read_getc();
static void file_read_chunk();

#if defined(SysV)
/* System V wrappers for randomizers */
static long random();
static void srandom(int seed);
#endif

static void report(char *machine, off_t size);
static double time_so_far();
static void timestamp();
static void usage();

/*
 * Housekeeping variables to build up timestamps for the tests;
 *  global to make it easy to keep track of the progress of time.
 * all of this could have been done with non-global variables,
 *  but the code is easier to read this way and I don't anticipate
 *  much software engineering down the road
 */
static int basetime;		/* when we started */
static double delta[(int) TestCount][2];	/* array of DeltaT values */
static double last_cpustamp = 0.0;	/* for computing delta-t */
static double last_timestamp = 0.0;	/* for computing delta-t */

char name[Chunk];
FILE *stream;
int fd;
off_t words;
off_t size;
int buf[Chunk / IntSize];
int bufindex;
int chars[256];
int next;

/* entry point for ASSIGNMENT5 */
int bonnie_main(int argc, char **argv)
{
    int child;
    char *dir;
    double first_start;
    double last_stop;
    int lseek_count = 0;
    char *machine;
    int seek_control[2];
    int seek_feedback[2];
    char seek_tickets[Seeks + SeekProcCount];
    double seeker_report[3];
    fd = -1;
    basetime = (int) time((time_t *) NULL);
    size = 100;
    dir = ".";
    machine = "";

    /* parse the argument sent from the command line */
    for (next = 1; next < argc; next++) {
	if (strcmp(argv[next], "-s") == 0)
	    size = atol(argv[++next]);
	else if (strcmp(argv[next], "-m") == 0)
	    machine = argv[++next];
	else
	    usage();
    }

    if (size < 1) {
	usage();
    } 

    /*
       CHECK SYSTEM CAPABILITY:
       We proceed to check the size of files because 32-bit machines has a limit
     */
    if (sizeof(off_t) <= 4 && size > 2047) {
	fprintf(stderr, "File too large for 32-bit machine, sorry\n");
	exit(1);
    }

    sprintf(name, "%s/ASSIGNMENT5.%d", dir, getpid());

    /* The size is in Megabytes, and is rounded down to multiples of Chunk */
    size *= (1024 * 1024);
    size = Chunk * (size / Chunk);
    fprintf(stderr, "File '%s', size: %ld\n", name, size);

    fill_file_char(name, &fd, &stream);	/* For specific details please go down to the body of this function */
    file_read_rewrite_block(name, &fd, &stream);	/* For specific details please go down to the body of this function */
    fill_file_block(name, &fd, &stream);	/* For specific details please go down to the body of this function */
    file_read_getc(name, &fd, &stream);	/* For specific details please go down to the body of this function */

    /* use the frequency count */
    for (words = 0; words < 256; words++)
	sprintf((char *) buf, "%d", chars[words]);

    file_read_chunk(name, &fd, &stream);	/* For specific details please go down to the body of this function */

    /* use the frequency count */
    for (words = 0; words < 256; words++)
	sprintf((char *) buf, "%d", chars[words]);

    /*
     * Now test random seeks; first, set up for communicating with children.
     * The object of this e game is to do "Seeks" lseek() calls as quickly
     *  as possible.  So we'll farm them out among SeekProcCount processes.
     *  We'll control them by writing 1-byte tickets down a pipe which
     *  the children all read.  We write "Seeks" bytes with val 1, whichever
     *  child happens to get them does it and the right number of seeks get
     *  done.
     * The idea is that since the write() of the tickets is probably
     *  atomic, the parent process likely won't get scheduled while the
     *  children are seeking away.  If you draw a picture of the likely
     *  timelines for three children, it seems likely that the seeks will
     *  overlap very nicely with the process scheduling with the effect
     *  that there will *always* be a seek() outstanding on the file.
     * Question: should the file be opened *before* the fork, so that
     *  all the children are lseeking on the same underlying file object?
     */
    if (pipe(seek_feedback) == -1 || pipe(seek_control) == -1)
	io_error("pipe");
    for (next = 0; next < Seeks; next++)
	seek_tickets[next] = 1;
    for (; next < (Seeks + SeekProcCount); next++)
	seek_tickets[next] = 0;

    /* launch some parallel seek processes */
    for (next = 0; next < SeekProcCount; next++) {	/* for each seek proc */
	if ((child = fork()) == -1)
	    io_error("fork");
	else if (child == 0) {	/* child process */
	    /* set up and wait for the go-ahead */
	    close(seek_feedback[0]);
	    close(seek_control[1]);
	    newfile(name, &fd, &stream, 0);
	    srandom(getpid());
	    fprintf(stderr, "Seeker %d...", next + 1);

	    /* wait for the go-ahead */
	    if (read(seek_control[0], seek_tickets, 1) != 1)
		io_error("read ticket");
	    timestamp();
	    seeker_report[StartTime] = time_so_far();

	    /* loop until we read a 0 ticket back from our parent */
	    while (seek_tickets[0]) {	/* until Mom says stop */
		doseek((long) (random() % (size / Chunk)), fd,
		       ((lseek_count++ % UpdateSeek) == 0));
		if (read(seek_control[0], seek_tickets, 1) != 1)
		    io_error("read ticket");
	    }			/* until Mom says stop */
	    if (close(fd) == -1)
		io_error("close after seek");

	    /* report to parent */
	    get_delta_t(Lseek);
	    seeker_report[EndTime] = time_so_far();
	    seeker_report[CPU] = delta[(int) Lseek][CPU];
	    if (write
		(seek_feedback[1], seeker_report,
		 sizeof(seeker_report)) != sizeof(seeker_report))
		io_error("pipe write");
	    exit(0);
	}			/* child process */
    }				/* for each seek proc */

    /*
     * Back in the parent; in an effort to ensure the children get an even
     *  start, wait a few seconds for them to get scheduled, open their
     *  files & so on.
     */
    close(seek_feedback[1]);
    close(seek_control[0]);
    sleep(5);
    fprintf(stderr, "start 'em...");
    if (write(seek_control[1], seek_tickets, sizeof(seek_tickets)) !=
	sizeof(seek_tickets))
	io_error("write tickets");

    /* read back from children */
    for (next = 0; next < SeekProcCount; next++) {	/* for each child */
	if (read
	    (seek_feedback[0], (char *) seeker_report,
	     sizeof(seeker_report)) != sizeof(seeker_report))
	    io_error("pipe read");

	/*
	 * each child writes back its CPU, start & end times.  The elapsed time
	 *  to do all the seeks is the time the first child started until the
	 *  time the last child stopped
	 */
	delta[(int) Lseek][CPU] += seeker_report[CPU];
	if (next == 0) {	/* first time */
	    first_start = seeker_report[StartTime];
	    last_stop = seeker_report[EndTime];
	} /* first time */
	else {			/* not first time */
	    first_start = (first_start < seeker_report[StartTime]) ?
		first_start : seeker_report[StartTime];
	    last_stop = (last_stop > seeker_report[EndTime]) ?
		last_stop : seeker_report[EndTime];
	}			/* not first time */
	if (wait(&child) == -1)
	    io_error("wait");
	fprintf(stderr, "done...");
    }				/* for each child */
    fprintf(stderr, "\n");
    delta[(int) Lseek][Elapsed] = last_stop - first_start;

    report(machine, size);
    unlink(name);
    return EXIT_SUCCESS;
}

/*********************************************************************/
/*                     FUNCTION report                               */
/*********************************************************************/
/* INPUT: Machine description and size of file                       */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Print in the monitor the results from the         */
/*              operations the program supports.                     */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void report(char *machine, off_t size)
{
    printf("              ");
    printf
	("-------Sequential Output-------- ---Sequential Input-- --Random--\n");
    printf("              ");
    printf
	("-Per Char- --Block--- -Rewrite-- -Per Char- --Block--- --Seeks---\n");
    printf("Machine    MB ");
    printf("K/sec %%CPU K/sec %%CPU K/sec %%CPU K/sec %%CPU K/sec ");
    printf("%%CPU  /sec %%CPU\n");
    printf("%-8.8s %4ld ", machine, size / (1024 * 1024));

    printf("%5d %4.1f %5d %4.1f %5d %4.1f ",
	   (int) (((double) size) / (delta[(int) Putc][Elapsed] * 1024.0)),
	   delta[(int) Putc][CPU] / delta[(int) Putc][Elapsed] * 100.0,
	   (int) (((double) size) /
		  (delta[(int) FastWrite][Elapsed] * 1024.0)),
	   delta[(int) FastWrite][CPU] / delta[(int) FastWrite][Elapsed] *
	   100.0,
	   (int) (((double) size) /
		  (delta[(int) ReWrite][Elapsed] * 1024.0)),
	   delta[(int) ReWrite][CPU] / delta[(int) ReWrite][Elapsed] *
	   100.0);

    printf("%5d %4.1f %5d %4.1f ",
	   (int) (((double) size) / (delta[(int) Getc][Elapsed] * 1024.0)),
	   delta[(int) Getc][CPU] / delta[(int) Getc][Elapsed] * 100.0,
	   (int) (((double) size) /
		  (delta[(int) FastRead][Elapsed] * 1024.0)),
	   delta[(int) FastRead][CPU] / delta[(int) FastRead][Elapsed] *
	   100.0);

    printf("%5.1f %4.1f\n",
	   ((double) Seeks) / delta[(int) Lseek][Elapsed],
	   delta[(int) Lseek][CPU] / delta[(int) Lseek][Elapsed] * 100.0);
}

/*********************************************************************/
/*                     FUNCTION newfile                              */
/*********************************************************************/
/* INPUT: Name of the file, File Descriptor, Buffer, Create Flag     */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. If Create Flag is one                             */
/*		 i) Checks if the file is unlinked                   */
/*		  a) If it is not returns error                      */
/*		 ii) Obatain File Descriptor with proper parameters  */
/*		  and flags.NOTE: Use man pages to view the specific */
/*		  parameters open uses.  Make sure you understand    */
/*		  the return values, and see the errors the function */
/*		  returns)                                           */
/*		 Else Create flag is not equal to zero               */
/*	          Obtain file descriptor with proper parameters and  */
/*                flags.(NOTE: Use man pages to view the specific    */
/*		  parameters open uses.  Make sure you understand    */
/*		  the return values, and see the errors the function */
/*		  returns)                                           */
/*          2. Fill buffer when calling fdopen with proper           */
/*          parameters and flags.(NOTE: Use man pages to view the    */
/*          specific parameters open uses.  Make sure you understand */
/*	 	the return values, and see the errors the function   */
/*		returns)         	                             */
/*		3. Check stream					     */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void newfile(char *name, int *fd, FILE ** stream, int create)
{
    if (create) {
	if (unlink(name) == -1 && *fd != -1)
	    io_error("unlink");
	*fd = open(name, O_RDWR | O_CREAT | O_EXCL, 0777);
    } else
	*fd = open(name, O_RDWR, 0777);
    if (*fd == -1)
	io_error(name);
    *stream = fdopen(*fd, "r+");
    if (*stream == NULL)
	io_error("fdopen");
}


/*********************************************************************/
/*                     FUNCTION fill_file_char                       */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Open file                                         */
/*		2. Record timestamp                                  */
/*		3. For cycle to fill the file one character at a time*/
/*		until it reaches the EndOfFile(EOF)                  */
/*		4. Close file                                        */
/*		5. Call get_delta_t to find the performance of putc  */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void fill_file_char()
{
    fprintf(stderr, "FUNCTION fill_file_char() start...");
    newfile(name, &fd, &stream, 1);
    timestamp();
    for (words = 0; words < size; words++)
	if (putc(words & 0x7f, stream) == EOF)
	    io_error("putc");
    if (fclose(stream) == -1)
	io_error("fclose after putc");
    get_delta_t(Putc);
    fprintf(stderr, "...done FUNCTION file_fill_char()\n");
}

/*********************************************************************/
/*                     FUNCTION fill_file_block                      */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Open file                                         */
/*		2. Initialize every position of the buffer	     */
/*		3. Record timestamp                                  */
/*		4. FOR cycle so for each word you need to:           */
/*		 i) properly set the buffer index so that it points  */
/*		 at the correct position.                            */
/*		 ii) Call write.(NOTE: Use man pages to view the     */
/*		 specific parameters write uses.  Make sure you      */
/*		 understand the return values, and see the errors    */
/*		 the function returns)                    	     */
/*		4. Close file                                        */
/*		5. Call get_delta_t to find the performance of putc  */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/
static void fill_file_block()
{
    fprintf(stderr, "FUNCTION fill_file_block() start...");
    newfile(name, &fd, &stream, 1);
    for (words = 0; words < Chunk / IntSize; words++)
	buf[words] = 0;
    timestamp();
    for (words = bufindex = 0; words < (size / Chunk); words++) {
	if (bufindex == (Chunk / IntSize))
	    bufindex = 0;
	buf[bufindex++]++;
	if (write(fd, (char *) buf, Chunk) == -1)
	    io_error("write(2)");
    }
    if (close(fd) == -1)
	io_error("close after fast write");
    get_delta_t(FastWrite);
    fprintf(stderr, "...done FUNCTION fill_file_block()\n");
}

/*********************************************************************/
/*                  FUNCTION file_read_rewrite_block                 */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Open file                                         */
/*		2. Call lseek. (NOTE: Use man pages to view the      */
/*              specific parameters lseek uses.  Make sure you       */
/*              understand the return values, and see the errors the */
/*              function returns)                                    */
/*		3. Record timestamp    				     */
/*		4. Make sure the buffer index point to zero          */
/*		5. Call read. (NOTE: Use man pages to view the       */
/*              specific parameters read uses.  Make sure you        */
/*              understand the return values, and see the errors the */
/*              function returns)                                    */
/*		6. While we can read a block, increment the index to */
/*		point to the the next position.  Call lseek and call */
/*		write.(NOTE: Use man pages to view the specific      */
/*		parameters write uses.  Make sure you understand the */
/*		return values,and see the errors the function        */
/*              returns)                                             */
/*		Call read until the while condition is false.        */
/*		7. Close file                                        */
/*		8. Call get_delta_t to find the performance of putc  */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void file_read_rewrite_block()
{
    int old = 0;
    if (old) {
        fprintf(stderr, "FUNCTION file_read_rewrite() start...");
        newfile(name, &fd, &stream, 0);
        if (lseek(fd, (off_t) 0, 0) == (off_t) - 1)
    	io_error("lseek(2) before rewrite");
        fprintf(stderr, "Rewriting");
        timestamp();
        bufindex = 0;
        if ((words = read(fd, (char *) buf, Chunk)) == -1)
    	io_error("rewrite read");

        while (words == Chunk) {
    	if (bufindex == Chunk / IntSize)
    	    bufindex = 0;
    	buf[bufindex++]++;
    	if (lseek(fd, (off_t) - words, 1) == -1)
    	    io_error("relative lseek(2)");
    	if (write(fd, (char *) buf, words) == -1)
    	    io_error("re write(2)");
    	if ((words = read(fd, (char *) buf, Chunk)) == -1)
    	    io_error("rwrite read");
        }

        if (close(fd) == -1)
    	io_error("close after rewrite");
        get_delta_t(ReWrite);
        fprintf(stderr, "...done FUNCTION file_read_rewrite_block()\n");
    } else {
        timestamp();
        bufindex = 0;
        if ((words = file_read(name, 0, (char *) buf, Chunk)) == -1)
        io_error("rewrite read");

        //while (words == Chunk) {
          //  printf("This loop is not finite..");
            //if (bufindex == Chunk / IntSize)
             //   bufindex = 0;
            //buf[bufindex++]++;
            //if (lseek(fd, (off_t) - words, 1) == -1)
             //   io_error("relative lseek(2)");
            if (file_write(name, words, (char *) buf, words) == -1)
                io_error("re write(2)");
            if ((words = file_read(name, words, (char *) buf, Chunk)) == -1)
                io_error("rwrite read");
       // }

        //if (close(fd) == -1)
        //io_error("close after rewrite");
        get_delta_t(ReWrite);
        fprintf(stderr, "...done FUNCTION file_read_rewrite_block()\n");
    }
}


/*********************************************************************/
/*                     FUNCTION file_read_getc                       */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Open file                                         */
/*		2. Initialize every position of the buffer words     */
/*		3. Record timestamp    				     */
/*		4. FOR cycle to fill the file 1 byte at a time until */
/*		it reaches the EndOfFile(EOF).Make sure you notice   */
/*		the increment of the index pointing at chars         */
/*		7. Close file                                        */
/*		8. Call get_delta_t to find the performance of getc  */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void file_read_getc()
{
    fprintf(stderr, "FUNCTION fill_read_getc() start...");
    newfile(name, &fd, &stream, 0);
    for (words = 0; words < 256; words++)
	chars[words] = 0;
    timestamp();
    for (words = 0; words < size; words++) {
	if ((next = getc(stream)) == EOF)
	    io_error("getc(3)");
	chars[next]++;
    }
    if (fclose(stream) == -1)
	io_error("fclose after getc");
    get_delta_t(Getc);
    fprintf(stderr, "...done FUNCTION file_read_getc()\n");
}


/*********************************************************************/
/*                     FUNCTION file_read_chunk                      */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Open file                                         */
/*		2. Call lseek().(NOTE:Use man pages to view the      */
/*	        specific                                             */
/*		parameters write uses.  Make sure you understand the */
/*		return values,and see the errors the function        */
/*              returns)                                             */
/*		3. Record timestamp    				     */
/*		6. DO cycle read a block.(NOTE: Use man pages to view*/
/*          the specific parameters read uses.  Make sure you        */
/*          understand the return values, and see the errors the     */
/*          function returns) Call read until the while condition is */
/*          false.                                                   */
/*		7. Close file                                        */
/*		8. Call get_delta_t to find the performance of getc  */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void file_read_chunk()
{
    fprintf(stderr, "FUNCTION file_read_chunk() start...");
    newfile(name, &fd, &stream, 0);
    if (lseek(fd, (off_t) 0, 0) == -1)
	io_error("lseek before read");
    timestamp();
    do {
	if ((words = read(fd, (char *) buf, Chunk)) == -1)
	    io_error("read(2)");
	chars[buf[abs(buf[0]) % (Chunk / IntSize)] & 0x7f]++;
    } while (words);
    if (close(fd) == -1)
	io_error("close after read");
    get_delta_t(FastRead);
    fprintf(stderr, "...done FUNCTION file_read_chunk()\n");
}

/*********************************************************************/
/*                     FUNCTION usage                                */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Presents all the options the program has          */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void usage()
{
    fprintf(stderr, "usage: ./assignment5 [-s size-in-Mb] [-m machine-label]\n");
    exit(1);
}

/*********************************************************************/
/*                     FUNCTION timestamp                            */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Call time_so_far() and store it in last_timestamp */
/*		2. Call cpu_so_far() and store it in last_cpustamp   */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void timestamp()
{
    last_timestamp = time_so_far();
    last_cpustamp = cpu_so_far();
}

/*********************************************************************/
/*                     FUNCTION get_delta_t                          */
/*********************************************************************/
/* INPUT: Receives the structure tests_t                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Identifies the procedure that was executed        */
/*		2. Calls time_so_far() and deletes the value of      */
/*          of the global variable last_timestamp obtaining the time */
/*          the procedure spent.                                     */
/*		3. Calls cpu_so_far() and substracts the value of    */
/*          of the global variable last_cpustamp obtaining the       */
/*          amount of CPU procedure spent.                           */
/*********************************************************************/
/* OUTPUT: doesn´t return any value because it is void               */
/*********************************************************************/

static void get_delta_t(test)
tests_t test;
{
    int which = (int) test;
    delta[which][Elapsed] = time_so_far() - last_timestamp;
    delta[which][CPU] = cpu_so_far() - last_cpustamp;
}

/*********************************************************************/
/*                     FUNCTION cpu_so_far                           */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Identifies the System to check some variables     */
/*			if SysV          	 		     */
/*			 i) Creates a structure tms.		     */
/*			 ii) returns the CPU time after adding over  */
/*			 certain fields in the tms structure.        */
/*		   	Else                                         */
/*			 i) Creates a structure rusage.		     */
/*			 ii) Calls getrusage().(NOTE: Use man pages  */
/*                       to view the specific parameters write uses. */
/*                       Make sure you understand the return values, */
/*                       and see the errors the function returns).   */
/*			 iii) returns the CPU time after adding over */
/*			 certain fields in the rusage structure.     */
/*********************************************************************/
/* OUTPUT: return the CPU usage until the time it has been called    */
/*********************************************************************/

static double cpu_so_far()
{
#if defined(SysV)
    struct tms tms;
    if (times(&tms) == -1)
	io_error("times");
    return ((double) tms.tms_utime) / ((double) sysconf(_SC_CLK_TCK)) +
	((double) tms.tms_stime) / ((double) sysconf(_SC_CLK_TCK));
#else
    struct rusage rusage;
    getrusage(RUSAGE_SELF, &rusage);
    return ((double) rusage.ru_utime.tv_sec) +
	(((double) rusage.ru_utime.tv_usec) / 1000000.0) +
	((double) rusage.ru_stime.tv_sec) +
	(((double) rusage.ru_stime.tv_usec) / 1000000.0);
#endif
}

/*********************************************************************/
/*                     FUNCTION time_so_far                          */
/*********************************************************************/
/* INPUT: does not receive any parameter                             */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Identifies the System to check some variables     */
/*			if SysV          			     */
/*			 i) Creates a structure tms.	             */
/*                       ii) Call the times function and assign the  */
/*                       value it returns to the variable val        */
/*			 iii) returns the CPU time after dividing val*/
/*			 by the returning value from sysconf()       */
/*		   	Else                                         */
/*			 i) Creates a structure timeval tp.          */
/*			 ii) Calls gettimeofday().(NOTE:Use man pages*/
/*                       to view the specific parameters             */
/*                       gettimeofday uses. Make sure you understand */
/*                       the return values, and see the errors the   */
/*                       function returns).                          */
/*			 iii) returns the CPU time after substractin */
/*			 certain fields in the tp structure.         */
/*********************************************************************/
/* OUTPUT: return the time it has been used                          */
/*********************************************************************/

static double time_so_far()
{
#if defined(SysV)
    int val;
    struct tms tms;
    if ((val = times(&tms)) == -1)
	io_error("times");

    return ((double) val) / ((double) sysconf(_SC_CLK_TCK));

#else
    struct timeval tp;
    if (gettimeofday(&tp, (struct timezone *) NULL) == -1)
	io_error("gettimeofday");
    return ((double) (tp.tv_sec - basetime)) +
	(((double) tp.tv_usec) / 1000000.0);
#endif
}

/*********************************************************************/
/*                     FUNCTION io_error                             */
/*********************************************************************/
/* INPUT: Receives the error string                                  */
/*********************************************************************/
/* PROCESS:                                                          */
/*		1. Call perror().(NOTE:Use man pages to view the     */
/*          specific parameters gettimeofday uses. Make sure you     */
/*          understand the return values, and see the errors the     */
/*          function returns).					     */
/*		2. Call exit().(NOTE:Use man pages to view the       */
/*          specific parameters gettimeofday uses. Make sure you     */
/*          understand the return values, and see the errors the     */
/*          function returns).        				     */
/*********************************************************************/
/* OUTPUT: return the time it has been used                          */
/*********************************************************************/

static void io_error(char *message)
{
    char buf[Chunk];
    sprintf(buf, "Program: drastic I/O error (%s)", message);
    perror(buf);
    exit(1);
}

/*
 * Do a typical-of-something random I/O.  Any serious application that
 *  has a random I/O bottleneck is going to be smart enough to operate
 *  in a page mode.
 * The 'where' argument is used as a chunk number
 * To keep the cache from getting too clever, some pages must be updated.
 *  However an application that updated each of many random pages that
 *  it looked at is hard to imagine.
 * However, it would be wrong to put the update percentage in as a
 *  parameter - the effect is too nonlinear.
 */
static void doseek(off_t where, int fd, int update)
{
    int buf[Chunk / IntSize];
    off_t probe;
    off_t size;

    probe = where * Chunk;
    if (lseek(fd, probe, 0) != probe)
	io_error("lseek in doseek");
    if ((size = read(fd, (char *) buf, Chunk)) == -1)
	io_error("read in doseek");

    /* every so often, update a block */
    if (update) {		/* update this block */
	/* touch a word */
	buf[((int) random() % (size / IntSize - 2)) + 1]--;
	if (lseek(fd, (long) probe, 0) != probe)
	    io_error("lseek in doseek update");
	if (write(fd, (char *) buf, size) == -1)
	    io_error("write in doseek");
    }				/* update this block */
}

#if defined(SysV)

static char randseed[32];

static void srandom(int seed)
{
    sprintf(randseed, "%06d", seed);
}

static long random()
{
    return nrand48(randseed);
}

#endif
