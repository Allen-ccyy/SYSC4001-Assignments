#include <stdlib.h>

#define IOERR_INVALID_ARGS (-1)
#define IOERR_INVALID_PATH (-2)
#define IOERR_POSIX (-3)
#define IOERR_BUFFER_TOO_SMALL (-4)
#define IOERR_NOT_YET_IMPLEMENTED (-5)

/* You will implement all of the following functions in fileio.c. */

/*
ERROR HANDLING
These functions will support basic error handling:

    If the arguments are bad (e.g., a pointer is NULL), then the
    function should immediately return IOERR_INVALID_ARGS.

    If a POSIX error occurs, the function should return IOERR_POSIX,
    unless the error is due to a bad path, in which case the return
    value is IOERR_INVALID_PATH.

    These functions do not return if a POSIX function is interrupted by
    a signal; only dirlist returns IOERR_BUFFER_TOO_SMALL (see below).
    You may find some of the library functions in restart.c useful.

COMMON PARAMETERS
    path - file to be read
    offest - where to start reading/writing from
    buffer - buffer to write bytes into
    bufbytes - maximum number of bytes to read

FUNCTION DESCRIPTIONS

int file_read(char *path, int offset, void *buffer, size_t bufbytes);
    Reads bytes from a file into the buffer.
    Return value: >=0 number of bytes read, <0 ERROR (see above)

int file_info(char *path, void *buffer, size_t bufbytes)
    Writes into buffer a string that describes meta information about
    the file.  The string is in format:
    "Size:NNN Accessed:NNN Modified:NNN Type X"
    X can be 'f' or 'd' (file or directory, respectively)
    Size is in bytes
    Accessed and Modified - accessed and modified times (seconds since epoch)

int file_write(char *path, int offset, void *buffer, size_t bufbytes);
    Writes bytes from 'buffer' into file 'path' at position 'offset'.
    Return value: >0 number of bytes written, <0 ERROR (see above)

int file_create(char *path, char *pattern, int repeatcount);
    Creates a new file with the string 'pattern' repeated 'repeatcount'
    times.
    Return value:0 Success , <0 ERROR (see above)

int file_remove(char *path);
    Removes an existing file if it exists.
    Return value: 0 file removed, <0 ERROR (see above)

int dir_create(char *path);
    Creates a new directory.
    Return value: 0 directory created, <0 ERROR (see above)

int dir_list(char *path, void *buffer, size_t bufbytes);
    Writes a list file and directory names inside path (including "."
    and ".." entries).  Each entry is line terminated with the newline
    character '\n'.
    Return value: 0 success, <0 ERROR (see above)
    Returns IOERR_BUFFER_TOO_SMALL if the buffer is not large enough to
    write all entries.
    Hint: man opendir, readdir, closedir

int file_checksum(char *path)
    Calculates a checksum value calculated by summing all the bytes of a
    file using an unsigned short.
    Return value: >=0 checksum value, <0 ERROR (see above)
    Hint: use the checksum function in util.c

int dir_checksum(char *path);
    Recursively calculated a checksum: checksums of regular files are
    calculated using file_checksum; directory entries are traversed,
    also the subdirectory names are included in the checksum calculation
    See assignment5_tests.c for details of what we are looking for.
    Return value: >=0 checksum value, <0 ERROR (see above)

    Hint: during development add debug statements, e.g.:
    fprintf(stderr,"xxx=%d\n",numbytes);
    write(2,buffer,numbytes);
    write(2,"RETURNING x",11);
    to your code in fileio.c or assignment5_tests.c

    Hint2: don't forget the POSIX man pages of
    open/close/write/read/lseek/opendir/readdir/closedir/mkdir
*/

int file_read(char *path, int offset, void *buffer, size_t bufbytes);
int file_info(char *path, void *buffer, size_t bufbytes);
int file_write(char *path, int offset, void *buffer, size_t bufbytes);

int dir_create(char *path);
int dir_list(char *path, void *buffer, size_t bufbytes);

int file_create(char *path, char *pattern, int repeatcount);
int file_remove(char *path);

int file_checksum(char *path);
int dir_checksum(char *path);
