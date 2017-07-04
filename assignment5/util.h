/*************** YOU SHOULD NOT MODIFY ANYTHING IN THIS FILE ***************/
#include <stdlib.h>
/* stdlib required for size_t */

#define quit_if_ne(p1,p2) {if((p1) != (p2)) quit_with_message((int)errno,"" #p1 "!=" #p2 ,__FILE__,__LINE__);}

#define quit_if(mutley) {if(mutley) quit_with_message((int)errno, #mutley,__FILE__,__LINE__);}
#define quit(mesg) {quit_with_message((int)errno,mesg,__FILE__,__LINE__);}

void quit_with_message(int errn, char *mesg, char *src, int line);

unsigned short checksum(char *buffer, size_t sz, unsigned short val);
