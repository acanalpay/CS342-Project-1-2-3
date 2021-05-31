#include <sys/types.h>
#include <unistd.h>

#include <unistd.h>
#include <stdlib.h>
#include<stdio.h>
#include "sbmem.h"

int main(){
        int child = fork();
        void *p[5];

        if(child == 0){
                int access = sbmem_open();
                if(access == -1){
                        exit(1);
                }
                void* x[5];
                int i;

                for(i = 0; i < 5; i++){
                        x[i] = sbmem_alloc(250);

                }

                for(i = 0; i < 5; i++)
                        sbmem_free(x[i]);

                sbmem_close();

                exit(0);
        }
        else{
                int access = sbmem_open();
                if(access == -1){
                        exit(1);
                }

                int i;

                for(i = 0; i < 3; i++)
                        p[i] = sbmem_alloc(512);
        }

        wait(child);
        int i;
        for(i = 0; i < 3; i++)
                sbmem_free(p[i]);

        sbmem_close();



	return 0;
}
