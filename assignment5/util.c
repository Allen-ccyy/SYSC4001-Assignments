/*************** YOU SHOULD NOT MODIFY ANYTHING IN THIS FILE ***************/
/* execl*/
#include <unistd.h>
/* errno */
#include <errno.h>
/* waitpid,WEXITSTATUS*/
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <wait.h>
/* fprintf*/
#include <stdio.h>
#include <string.h>

#include "util.h"

/* Print a detailed error message to the given file descriptor. The pointers f,mesg,src can be null. */
void print_error(FILE * f, int errn, char *mesg, char *src, int line)
{
    char errmesg[255];
    *errmesg = '\0';
    if (strerror_r(errn, errmesg, sizeof(errmesg)))
	strcpy(errmesg, "Unknown");
    fprintf(f ? f : stderr, "\nFAILED AT %s(%d); %s%s. errno=%d(%s)\n",
	    src ? src : "notset", line, mesg ? "because " : "",
	    mesg ? mesg : "", errn, errmesg);
}

/* Print a detailed error message and then exit with a status value of 1.*/
void quit_with_message(int errn, char *mesg, char *src, int line)
{
    print_error(stderr, errn, mesg, src, line);
    exit(1);
}

/* Calculate a very simple checksum for a character buffer.*/
unsigned short checksum(char *buffer, size_t sz, unsigned short val)
{
    if ((!buffer) || (!sz))
	return val;
    while ((sz--) > 0) {
	val += *(buffer++);
    }
    return val;
}
