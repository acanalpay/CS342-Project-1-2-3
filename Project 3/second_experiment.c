#include <sys/types.h>
#include <unistd.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "sbmem.h"

int main(){
	
        int child = fork();

        if(child == 0){
                int access = sbmem_open();
                if(access == -1){
                        exit(1);
                }
                void* p[5];
                p[1] = sbmem_alloc(120);
                p[2] = sbmem_alloc(430);
                p[3] = sbmem_alloc(230);

                int i;
                for(i = 1; i < 4; i ++)
                        sbmem_free(p[i]);

                sbmem_close();

                exit(0);
        }
        else{
                int access = sbmem_open();
                if(access == -1){
                        exit(1);
                }

                void* p1 = sbmem_alloc(3800);
                void* p2 = sbmem_alloc(124);


                sbmem_free(p1);
                sbmem_free(p2);

                sbmem_close();
        }

        wait(child);

	return 0;
}
