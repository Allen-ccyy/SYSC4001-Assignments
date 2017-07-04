/* ASSIGNMENT2: Simple Shell */

/* LIBRARY SECTION */
#include <ctype.h>              /* Character types                       */
#include <stdio.h>              /* Standard buffered input/output        */
#include <stdlib.h>             /* Standard library functions            */
#include <string.h>             /* String operations                     */
#include <sys/types.h>          /* Data types                            */
#include <sys/wait.h>           /* Declarations for waiting              */
#include <unistd.h>             /* Standard symbolic constants and types */

#include "assignment2_tests.h"  /* Built-in test system                  */

/* MAIN PROCEDURE SECTION */
int main(int argc, char **argv)
{
	if (access("home/Ben/Documents/assignment2/Assignment2Start/assignment2/shell", X_OK) != -1 ) {
		printf("test\n");
	} 
}
