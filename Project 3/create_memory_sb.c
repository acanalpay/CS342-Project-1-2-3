

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "sbmem.h"

int main()
{
   	int size;
  	printf("Please enter the size that is power of 2: ");
	scanf("%d",&size);

 	sbmem_init(size); 

    	printf ("memory segment is created and initialized \n");

    	return (0); 
}
