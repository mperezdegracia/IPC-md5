#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


#define BUFFSIZE 256 
int
main(int argc, char const* argv[]) {
	char buffer[BUFFSIZE]; 
	char * r; 

    FILE *fptr;

    fptr = fopen("out.txt","w"); 

	do {
		r = fgets(buffer, BUFFSIZE, stdin);
		int n = strlen(buffer); 
		buffer[n] = '\0';  


        fprintf(fptr,"%s \n",buffer);
	

	} while( r != NULL );
    fclose(fptr);
	return 0; 
}
