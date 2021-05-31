#include <sys/types.h>
#include <unistd.h>

#include <unistd.h>
#include <stdlib.h>
#include<stdio.h>
#include "sbmem.h"

int main(){
	int qwe = sbmem_open();
	if(qwe == -1){
		printf("Could not access the library\n");
		return 0;
	}        
	char* p = (char*) sbmem_alloc(31);
        int i = 0;
        for(i = 0; i < 31; i++)
                p[i] = 'a';

        //printf("%s\n",p);
        char* t = sbmem_alloc(120);
        for(i = 0; i < 120; i++)
                t[i] = 'b';

        char* y = sbmem_alloc(6);
        for(i = 0; i < 6; i++)
                y[i] = 'c';
	
        //printf("%s\n %s\n %s\n",t, y, p);

        sbmem_free(p);
        sbmem_free(t);
        sbmem_free(y);

        p = (char *)sbmem_alloc(65);
        if(p != NULL)
                sbmem_free(p);

        sbmem_close();
	

        for(i = 0; i < 5; i++){
                int n = fork();//Child processes also open new processes
                if(n == 0){
                        int r = sbmem_open();

                        if(r == -1){

                                printf("Process %d is not allowed to use the library\n",getpid());
                                exit(1);
                        }
			sbmem_close();//for experiment I commented this line
			//To get the outup in the appendix, you can comment sbmem_close(). However
			//when you comment this line it will cause problem since processes do not 
			//exit from the library and then the library do not allow other programs to use itself.
                }
                else
                        wait(n);
	}
	
	sbmem_close();

        return (0);
}

