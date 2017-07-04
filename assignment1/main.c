/*
About this program:
- This program counts words.
- The specific words that will be counted are passed in as command-line
  arguments.
- The program reads words (one word per line) from standard input until EOF or
  an input line starting with a dot '.'
- The program prints out a summary of the number of times each word has
  appeared.
- Various command-line options alter the behavior of the program.

E.g., count the number of times 'cat', 'nap' or 'dog' appears.
> ./main cat nap dog
Given input:
 cat
 .
Expected output:
 Looking for 3 words
 Result:
 cat:1
 nap:0
 dog:0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assignment1_test.h"

#define LENGTH(s) (sizeof(s) / sizeof(*s))

/* Structures */
typedef struct {
  char *word;
  int counter;
} WordCountEntry;

//out will specify the output location, default is STDOUT, -f can change this to a file.
void* out; 

int process_stream(WordCountEntry entries[], int entry_count)
{
  //Variables
  short line_count = 0;
  char buffer[30];
  char* words;
  //line_count will count the lines input, buffer reads stdin, words is used to tokenize buffer into words.
  //fgets reads either 30 chars or until \n then runs code
  while (fgets(buffer, 30, stdin)) {
    if (*buffer == '.' || (*buffer == 'E' && *(buffer+1) == 'O' && *(buffer+2) == 'F'))
      break;
    /* Compare against each entry, break loop if '.' or 'EOF' is passed.*/
    //Removes the \n from the end of buffer.
    strtok(buffer, "\n"); 
    //Sets 'words' to the first word in the buffer
    words = strtok(buffer, " \t");
    //loops until all words have been analized
    //compares the words to the words stored in entries
    //if the word is a hit, add one to the counter
    while(words != NULL){ 
      for(int i = 0; i< entry_count; i++){ 
        if (!strcmp(entries[i].word, words)){  
          entries[i].counter++;
        }
      }
      words = strtok(NULL, " \t"); //advances words to the next word in the buffer
      }
    line_count++; 
  }
  return line_count;
}


void print_result(WordCountEntry entries[], int entry_count)
{
  fprintf(out, "Result:\n");
  int temp = 0;
  //print all the entries and their count.
  while (temp++ < entry_count) {
    fprintf(out, "%s:%d\n", (entries+temp-1)->word, (entries+temp-1)->counter);
  }
}


void printHelp(const char *name)
{
  fprintf(out, "usage: %s [-h] <word1> ... <wordN>\n", name);
}


int main(int argc, char **argv)
{
  const char *prog_name = *argv;
  //WordCountEntry entries[argc-1];
  //create a dynamic array using the number of paramaters -1 as the size.
  //This will create at minimum enough spots for all the words
  //wasting some space when there is a '-fFILENAME'
  WordCountEntry* entries = malloc(sizeof(WordCountEntry) * (argc-1));
  int entryCount = 0;
  out = stdout;
  /* Entry point for the testrunner program */
  if (argc > 1 && !strcmp(argv[1], "-test")) {
    run_assignment1_test(argc - 1, argv + 1);
    return EXIT_SUCCESS;
  }
  argv++;
  //increment argv to point to the param after the program name.
  //loop through all the arguments if it starts with a - check for 'h' or 'f'
  //if not then create a word entry
  while (*argv != NULL) {
    if (**argv == '-') {
      switch ((*argv)[1]) {
        case 'h':
          printHelp(prog_name);
          break;
        case 'f':
      out = fopen(*argv+2, "w");
      break;
        default:
          fprintf(stderr, "%s: Invalid option %s. Use -h for help.\n",
                 prog_name, *argv);
      }
    } else {
    //argc-1 is the size of the stucture, can't use Length since it is a pointer not an array.
    //Lenght will be doing opperation sizeof(pointer)/sizeof(pointer) which always returns 1.
      if (entryCount < argc-1) {
        entries[entryCount].word = *argv;
        entries[entryCount++].counter = 0;
      }
    }
    argv++;
  }
  //ensure that at least one word is provided.
  if (entryCount == 0) {
    fprintf(stderr, "%s: Please supply at least one word. Use -h for help.\n",
           prog_name);
    return EXIT_FAILURE;
  }
  if (entryCount == 1) {
    fprintf(out, "Looking for a single word\n");
  } else {
    fprintf(out, "Looking for %d words\n", entryCount);
  }

  process_stream(entries, entryCount);
  print_result(entries, entryCount);

  return EXIT_SUCCESS;
}
