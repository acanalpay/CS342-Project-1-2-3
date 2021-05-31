#include <sys/types.h>
#include <unistd.h>

#include <unistd.h>
#include <stdlib.h>
#include<stdio.h>
#include "sbmem.h"

int main()
{
	
    	int i, ret; 
    	char *p;    
        printf("APP ITSELF\n");
    	ret = sbmem_open(); 
    	if (ret == -1)
        	exit (1); 
    
    	p = sbmem_alloc (256); // allocate space to hold 1024 characters
    	for (i = 0; i < 256; ++i)
        	p[i] = 'a'; // init all chars to ‘a’
    
    	//printf("%s\n",p);
    
    	sbmem_free (p);
        
	/*
    	p = sbmem_alloc(128);
    	for(i = 0; i < 128; i++)
		p[i] = 'b';

  	*/
	sbmem_close(); 
  
	return 0;
}
