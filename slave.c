#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(int argc, char const* argv[]) {
	
	for (int i = 1; i < argc; i++)
    {
        printf("%s \n", argv[i]);
        
    }
    

	return 0;
}
