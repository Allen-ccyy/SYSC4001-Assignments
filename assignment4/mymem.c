#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "mymem.h"
#include <time.h>


/* The main structure for implementing memory allocation.
 * You may change this to fit your implementation.
 */

struct memoryList {
  // doubly-linked list
  struct memoryList *last;
  struct memoryList *next;

  int size;            // How many bytes in this block?
  char alloc;          // 1 if this block is allocated,
                       // 0 if this block is free.
  void *ptr;           // location of block in memory pool.
};

strategies myStrategy = NotSet;    // Current strategy


size_t mySize;
void *myMemory = NULL;
int memoryAllocated;
int holes;

static struct memoryList *head;
static struct memoryList *next;
static struct memoryList *lastAssigned;


/* initmem must be called prior to mymalloc and myfree.

   initmem may be called more than once in a given exeuction;
   when this occurs, all memory you previously malloc'ed  *must* be freed,
   including any existing bookkeeping data.

   strategy must be one of the following:
		- "best" (best-fit)
		- "worst" (worst-fit)
		- "first" (first-fit)
		- "next" (next-fit)
   sz specifies the number of bytes that will be available, in total, for all mymalloc requests.
*/

void initmem(strategies strategy, size_t sz) {
	myStrategy = strategy;

	/* all implementations will need an actual block of memory to use */
	mySize = sz;

	if (myMemory != NULL) free(myMemory); /* in case this is not the first time initmem2 is called */

	//fprintf(stderr, "\nEntering the while loop...\n");
	// TODO: release any other memory you were using for bookkeeping when doing a re-initialization! 
	if(head != NULL ) head->last->next = NULL;;
	while(head != NULL){
		if(head->next != NULL){
			head = head->next;
			free(head->last);
			head->last = NULL;
		} else {
			free(head);	
			head = NULL;
		}
	} 

	//fprintf(stderr, "Got by the while loop...\n");
	myMemory = malloc(sz);
	
	/* TODO: Initialize memory management structure. */
	memoryAllocated = 0;
	holes = 1;
	head = malloc(sizeof(struct memoryList));
	next = head;
	head->next = head;
	head->last = head;
	head->size = mySize;
	head->alloc = 0;
	head->ptr = myMemory;
	lastAssigned=head;
}

//This function finds the correct size of the best fitting free block
int mem_best_fit(int size){
	if (!mem_free()) return 0;
	int c = -1;
	next = head;
	do{
		if ((next->size < c || c == -1) && next->size >= size && !next->alloc) c = next->size; 
		next = next->next;
	} while(next != head);
	return c;
}

/* Allocate a block of memory with the requested size.
 *  If the requested block is not available, mymalloc returns NULL.
 *  Otherwise, it returns a pointer to the newly allocated block.
 *  Restriction: requested >= 1 
 */

void *assignBlock(struct memoryList *node, int requested) {
//temp will hold our newly assigned chunk of memory
	//it will start at the begining of the empty block
	struct memoryList *temp;
	temp = malloc(sizeof(struct memoryList));
	temp->next = node;
	temp->last = node->last;
	temp->size = requested;
	temp->alloc = 1;
	temp->ptr = node->ptr;
	node->last->next=temp;
	node->last=temp;
	//add the offest in address and size
	node->ptr += requested;
	node->size -= requested;
	//if we just assigned the first block it becomes head
	if(node == head) head = temp;
	memoryAllocated +=requested;
	lastAssigned = temp;
	return temp->ptr;
}

void *mymalloc(size_t requested) {
	assert((int)myStrategy > 0);
	int fit;
	switch (myStrategy){
		case NotSet: 
	        return NULL;
	    case First:
	    	next = head;
	    	do {
	    		//special case if we found a block that is perfectly sized
	    		if((!next->alloc) && (next->size == requested)){
	    			next->alloc = 1;
	    			memoryAllocated +=requested;
	    			holes -= 1;
	    			return next->ptr;
	    		}
	    		//if we found a block big enough
	    		if((!next->alloc) && (next->size > requested)){
					return assignBlock(next, requested);
	    		}
	    		next = next->next;
	    	} while(next != head); //loop until we get to the end
	        
	        return NULL; //if no apropriate blocks were found then return null
	    case Best:
	    	fit = mem_best_fit(requested); //get the best size
	    	next = head;
	    	//special case for a perfect match
	    	if(requested == fit){
	    		//loop until the slot is found then alloc it
		    	while ((next->size != fit) || (next->alloc)) next = next->next;
		    	next->alloc = 1;
		    	holes -= 1;
	    		memoryAllocated +=requested;
		    	return next->ptr;
		    }
		    //I don't know why but requested < -1 evaluates to true sometimes... probably types. 
		    if (fit == -1) return NULL;
	    	if(requested < fit){
	    		//find the best slot then pass it to the genaric assign.
		    	while ((next->size != fit) || (next->alloc)) next = next->next;
		    	return assignBlock(next, requested);
		    }

	        return NULL;
	    case Worst:
	    	fit = 0;
	    	fit = mem_largest_free();
	    	//largest free will get us the 'worst fit'
	    	next = head;
	    	if(requested == fit){
	    		//perfect fit, find block and alloc
		    	while ((next->size != fit) || (next->alloc)) next = next->next;
		    	next->alloc = 1;
		    	holes -= 1;
	    		memoryAllocated +=requested;
		    	return next->ptr;
		    }
	    	if(requested < fit){
	    		//find block and then do normal genaric assign
		    	while ((next->size != fit) || (next->alloc)) next = next->next;
		    	return assignBlock(next, requested);
		    }
		    
	        return NULL;
	    case Next:
	    	next = lastAssigned;
	    	do {
	    		//special case if we found a block that is perfectly sized
	    		if((!next->alloc) && (next->size == requested)){
	    			next->alloc = 1;
	    			memoryAllocated +=requested;
	    			holes -= 1;
	    			lastAssigned = next;
	    			return next->ptr;
	    		}
	    		//if we found a block big enough
	    		if((!next->alloc) && (next->size > requested)){
	    			void* x = assignBlock(next, requested);
					return x;
	    		}
	    		next = next->next;
	    	} while(next != lastAssigned); //loop until we get to the end
	        
	        return NULL;
	  }
	return NULL; 
}


//Joins the node passed with the one before it then frees the node
void join(struct memoryList *node){
	if(node->last->alloc) printf("ERROR - Could not join node with previous.");
	//consume the size in the node that will stay
	node->last->size += node->size;
	//take next out of the list 
	node->last->next = node->next;
	node->next->last = node->last;
	//special cases for removing the head or lastAssigned node in 'next'
	if(node == head) head = node->last;
	if(node == lastAssigned) lastAssigned = node->last;
	//next is gone, free it :)
	free(node);
}

/* Frees a block of memory previously allocated by mymalloc. */
void myfree(void* block) {
	next = head;
	//find the block
	int holeStatus = 0;
	while(next->ptr != block) next = next->next;
	if (next == lastAssigned) lastAssigned = next->next;
	next->alloc = 0;
	memoryAllocated -= next->size;
	//check if the surounding blocks are free
	//must ensure we don't join the start and end
	if((next!=head) && (!next->last->alloc)){
		holeStatus++;
		next = next->last;
		join(next->next);
	} else if((next->next != head) && (!next->next->alloc)){
		holeStatus++;
		join(next->next);
	}
	if(!holeStatus){
		holes++;
	} 
	if(holeStatus==2){
		holes--;
	}
	return;
}

/****** Memory status/property functions ******
 * Implement these functions.
 * Note that when we refer to "memory" here, we mean the 
 * memory pool this module manages via initmem/mymalloc/myfree. 
 */

/* Get the number of contiguous areas of free space in memory. */
int mem_holes(){
	return holes;
}

/* Get the number of bytes allocated */
int mem_allocated(){
	return memoryAllocated;
}

/* Number of non-allocated bytes */
int mem_free(){
	int c = mem_total() - memoryAllocated;
	return c;
}

/* Number of bytes in the largest contiguous area of unallocated memory */
int mem_largest_free() {
	if (!mem_free()) return 0;
	int c = 0;
	next = head;
	do{
		if (next->size > c && !next->alloc) c = next->size; 
		next = next->next;
	} while(next != head);
	return c;
}

/* Number of free blocks smaller than "size" bytes. */
int mem_small_free(int size)
{
	int c = 0;
	next = head;
	do{
		if((next->size <= size) && (!next->alloc)) c+=1; 
		next = next->next;
	} while(next != head);
	return c;
}       

char mem_is_alloc(void *ptr)
{
    while(next->ptr != ptr) next = next->next;
    return next->alloc;
}

//Returns a pointer to the memory pool.
void *mem_pool()
{
	return myMemory;
}

// Returns the total number of bytes in the memory pool. */
int mem_total()
{
	return mySize;
}


// Get string name for a strategy. 
char *strategy_name(strategies strategy)
{
	switch (strategy)
	{
		case Best:
			return "best";
		case Worst:
			return "worst";
		case First:
			return "first";
		case Next:
			return "next";
		default:
			return "unknown";
	}
}

// Get strategy from name.
strategies strategyFromString(char * strategy)
{
	if (!strcmp(strategy,"best"))
	{
		return Best;
	}
	else if (!strcmp(strategy,"worst"))
	{
		return Worst;
	}
	else if (!strcmp(strategy,"first"))
	{
		return First;
	}
	else if (!strcmp(strategy,"next"))
	{
		return Next;
	}
	else
	{
		return 0;
	}
}


/* 
 * These functions are for you to modify however you see fit.  These will not
 * be used in tests, but you may find them useful for debugging.
 */

/* Use this function to print out the current contents of memory. */
void print_memory()
{
	next = head;
	int c = 0;
	do{
		printf("%i, Size:%i Alloc=%i Start:\n", c++, next->size, next->alloc);
		next = next->next;
	} while(next != head);
	return;
}

/* Use this function to track memory allocation performance.  
 * This function does not depend on your implementation, 
 * but on the functions you wrote above.
 */ 
void print_memory_status()
{
	printf("%d out of %d bytes allocated.\n",mem_allocated(),mem_total());
	printf("%d bytes are free in %d holes; maximum allocatable block is %d bytes.\n",mem_free(),mem_holes(),mem_largest_free());
	printf("Average hole size is %f.\n\n",((float)mem_free())/mem_holes());
}

/* Use this function to see what happens when your malloc and free
 * implementations are called.  Run "mem -try <args>" to call this function.
 * We have given you a simple example to start.
 */
void try_mymem(int argc, char **argv) {
 	print_memory_status();
	
}
