#include <sys/types.h>
#include <unistd.h>

#include <unistd.h>
#include <stdlib.h>
#include<stdio.h>
#include "sbmem.h"

int main(){
	int ch1;

        int i, ret;

        void* p[10];

        ch1 = fork();

        if(ch1 == 0){
                int ch2 = fork();

                if(ch2 == 0){
                        ret = sbmem_open();

                        if(ret == -1){
                                printf("Error on library accessing\n");

                                exit(1);
                        }

                        p[0] = sbmem_alloc(340);
                        p[1] = sbmem_alloc(340);
                        p[2] = sbmem_alloc(540);
                        p[3] = sbmem_alloc(140);
                        p[4] = sbmem_alloc(1040);


                        for(i = 0; i < 5; i++)
                                sbmem_free(p[i]);

                        sbmem_close();

                        exit(0);
                }

                ret = sbmem_open();

                if(ret == -1){
                        printf("Error on library accessing\n");

                        exit(1);
                }

                p[0] = sbmem_alloc(610);
                p[1] = sbmem_alloc(710);
                p[2] = sbmem_alloc(4096);
                p[3] = sbmem_alloc(1024);
                p[4] = sbmem_alloc(400);

                wait(ch2);

                for(i = 0; i < 5; i++)
                        sbmem_free(p[i]);

                sbmem_close();

                exit(0);

        }

        ret = sbmem_open();

        if(ret == -1){
                printf("Error on library accessing\n");

                exit(1);
        }

        p[0] = sbmem_alloc(200);
        p[1] = sbmem_alloc(300);
        p[2] = sbmem_alloc(400);
        p[3] = sbmem_alloc(500);
        p[4] = sbmem_alloc(600);

        wait(ch1);
        for(i = 0; i < 5; i++)
               sbmem_free(p[i]);

        sbmem_close();


        return 0;
}
