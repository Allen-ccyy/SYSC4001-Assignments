ASSIGNMENT5: I/O and Filesystems
==========-------===============

This assignment is the first stage (of potentially many) of a project aimed
at creating a simple yet functional networked filesystem.  In this assignment,
you will learn about and use POSIX file system calls. Specifically, you 
will implement a custom filesystem and test its performance using a 
filesystem benchmark.  A benchmark is an application used to test the
performance of some aspect of the system.  We will be using Bonnie, a real
filesystem benchmark, to test various performance aspects of the filesystem we
implement.

ASSIGNMENT5 consists of four steps:

1. Read the code; run the Bonnie benchmark and the ASSIGNMENT5 test suite.

2. Implement Test Suite 1 functionality, encompassing basic file I/O operations.

3. Implement Test Suite 2-4 functionality (directory operations, file
creation/deletion, and recursive checksumming).

4. Modify Bonnie to use your client-server file I/O methods.


Code structure
--------------

The code for this project is structured according to the client-server
model.  The client code (filesystem benchmark) will interact with the
server (filesystem) only through interface functions defined in
fileio.h:

int file_read(char *path, int offset, void *buffer, size_t bufbytes);
int file_info(char *path, void *buffer, size_t bufbytes);
int file_write(char *path, int offset, void *buffer, size_t bufbytes);

int file_create(char *path,char *pattern, int repeatcount);
int file_remove(char *path);

int dir_create(char *path);
int dir_list(char *path,void *buffer, size_t bufbytes);

int file_checksum(char *path);
int dir_checksum(char *path);

These functions represent a simple interface to our filesystem.  In Steps 2 and
3 of this assignment, you will write the code for functions implementing 
this interface, replacing the stub code in fileio.c.  In Step 4, you will 
modify a Bonnie method to use this interface, rather than calling the normal 
POSIX I/O functions directly.  The purpose of Step 4 is to help test our 
implementation.


Step 1: Understanding the code
------------------------------

1. Compile the project, execute Bonnie and the test framework.

Note: you may need to add execute permissions to the .sh files using
the command "chmod +x *.sh".

Try the following:
make
./assignment5
(this runs the Bonnie benchmark - it may take a little while)
./assignment5 -test suite1
(run Test Suite 1 - this has to work for stage1)
make test
(run all tests - this has to work for stage2)

2. Read through the provided .c and .h files and understand how this
project is organized:
bonnie.c - a version of the filesystem benchmark
fileio.c - file I/O functions to be implemented
fileio.h - declaration of file I/O functions
restart.c - restart library (available for use in fileio.c)
restart.h - declaration of restart library functions
util.c - useful utility functions
util.h - declaration of useful utility functions and macros

In particular, pay close attention to the comments in fileio.h and
bonnieb.c.  You should understand what each of the following functions
in bonnie.c does before undertaking the remainder of the assignment:

fill_file_char()
file_read_rewrite()
file_read_rewrite_block()
fill_file_block()
fill_read_getc()
file_read_chunk()
newfile()


Step 2: Basic I/O operations
----------------------------

Implement file_read, file_write and file_info operations in fileio.c.

If done correctly, your code should pass all suite1 tests:
./assignment5 -test suite1
Running tests...
 1.read                ::pass
 2.info                ::pass
 3.write               ::pass

Test Results:3 tests,3 passed,0 failed.

IMPORTANT: fileio.c is the only file you should modify for this step.


Step 3: More filesystem operations
----------------------------------

Implement file and directory operations for suite2 (dir_create and
dir_list), suite3 (file_create and file_remove), and suite4
(file_checksum and dir_checksum).

You can test each operation and test suite individually:
./assignment5 -test dirlist
./assignment5 -test suite2

All tests should now pass:
make test
Running tests...
 1.read                ::pass
 2.info                ::pass
 3.write               ::pass
 4.dirlist             ::pass
 5.dircreate           ::pass
 6.remove              ::pass
 7.create              ::pass
 8.filechecksum        ::pass
 9.dirchecksum         ::pass

Test Results:9 tests,9 passed,0 failed.


Step 4: Performance testing
---------------------------

In this step, we will change parts of Bonnie to use our filesystem
interface.

Make the function file_read_rewrite_block() in bonnie.c to call your
fileio.c functions instead of POSIX I/O operations.  When answering the
questions below, use this modified version of bonnie.c.

Before making this change, it's a good idea to write pseudocode comments
for what each part of file_read_rewrite_block() does, so that you
understand the code and can perform the change correctly.  There may
not be an exact one-to-one correspondence between our filesystem
interface and the POSIX commands.


Questions
---------

Q1.  Briefly explain what the following code from bonnie.c does:
if ((words = read(fd, (char *) buf, Chunk)) == -1) ...


Q2. Is the above an example of a block read or a character read?  What
is the value of the variable 'words' if the read succeeds?  Fails?


Q3.  Explain the meaning of the flag value (O_CREAT | O_WRONLY |
O_APPEND) for the POSIX function open().


Q4. Run Bonnie. What is being measured by each test function?


Q5. Look at the summary results from the Bonnie run in Q4. Does Bonnie
measure latency, throughput or something else?  Justify your answer.


Q6. Compare character reads with block reads using Bonnie.  Which is
faster?  Why do you think this is the case?


Q7. Copy and paste the performance measures output when running Bonnie
benchmarks in a local directory and again in an NFS-mounted directory.
Is one kind of disk access noticeably slower over the network, or are
all tests significantly slower?

Your home directory may be an NFS mount, whereas /tmp and /scratch are local
disks.  To test your code in /tmp, do the following:
mkdir /tmp/your_username
cp assignment5 /tmp/your_username
cd /tmp/your_username
./assignment5
(record the output)
cd
rm -fr /tmp/your_username


Q8. How does Bonnie handle incomplete reads, e.g., due to interruptions
from signals?  Justify why Bonnie's approach is good or bad for a
filesystem benchmark program.


Q9. By now you should be very familiar with the self-evaluation test
harness we provide for the assignments.  Examine the function test_file_read()
in assignment5_tests.c, which tests your file_read() function from Step 2.

What does this test check for, specifically?  You may want to copy and
paste the code for this function in your answer, and annotate each
quit_if or group of related quit_ifs with a comment.
