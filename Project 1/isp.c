#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include<sys/time.h>


#define READ 0
#define WRITE 1

int main(int argc, char *argv[]){
	int mode, buffer;
	struct timeval begin, end;

	mode = atoi(argv[2]);
	buffer = atoi(argv[1]);
	
	//printf("mode: %d %s\n",mode, argv[2]);
		
	while(1){
		printf("can.alpay:isp.c$  ");
		int arg_size = 1, size = -1;
		
		char command[20000];
		int fd1[2];
		int fd2[2];
			
		do{
			size++;
			
			scanf("%c",&command[size]);
			
			if(command[size] == ' ')
				arg_size++;

		}while(command[size] != '\n');


		char arguments[arg_size][size];
		char *arg[arg_size+1];

		command[size] = '\0';	
		int i, j = 0, ar = 0, pipe_loc, k;
		bool pipe_bol = false;
		//printf("COMMAND: %s\n",command); 
		for(i = 0; i < size; i++){
			k = j;
			if(command[i] == ' '){
				arguments[ar][j] = '\0';
				//printf("%s\n",arguments[ar]);
				j = 0;
				ar++;
			
				continue;
			
			}
			
			arguments[ar][j++] = command[i];
			
			if(command[i] == '|'){
				pipe_loc = ar;
				pipe_bol = true;
				//printf("PIPE EXIST\n");
			}
		}
		arguments[ar][k+1] = '\0';
		//printf("%s %d %d %c \n",arguments[ar], arg_size,pipe_loc, arguments[ar][k]);
		
		
		//char cmd[100] = "/bin/";
		//for(i = 0; i < strlen(arguments[0]); i++){
		//	cmd[i+5] = arguments[0][i];
		//}
		arg[arg_size] = NULL;
		for(i = 0; i < arg_size; i++){
			//printf("%s\n",(arguments[i]));
			arg[i] = arguments[i];
			
			//printf("%s ",arg[i]);
		}

		//printf("\nARG1\n");
		
		int child_pid1, child_pid2;

		if(mode == 1){ // normal mode
				

			if(pipe_bol){

				char* arg2[pipe_loc + 1];
				char* arg3[arg_size - pipe_loc];
				
				arg2[pipe_loc] = NULL;
				for(i = 0; i < pipe_loc; i++){
					arg2[i] = arg[i];
					
				//	printf("%s ",arg2[i]);
				}
				//printf("\nARG2\n");


				for(i = 0; i < arg_size - pipe_loc; i++){
					arg3[i] = arg[pipe_loc + i + 1];
					
					//printf("%s ",arg3[i]);
				}
				//printf("\nARG3\n");
				
				gettimeofday(&begin, NULL);
				
				if(pipe(fd1)== -1){
					printf("ERROR PIPE CREATION IN MODE 1\n");
					return 1;
				}
				
				child_pid1 = fork();

				if(child_pid1 == 0){//child1
					//writes to the pipe

					close(fd1[READ]);
					dup2(fd1[WRITE], 1);//stdout
					close(fd1[WRITE]);
					
					execvp(arg2[0], arg2);		
						
				}

				else{
					
					child_pid2 = fork();

					if(child_pid2 == 0){//child2

						
						close(fd1[WRITE]);
						dup2(fd1[READ], 0);
						close(fd1[READ]);

						execvp(arg3[0], arg3);
						
						for(i = 0 ; i < arg_size - pipe_loc; i++)
							printf("%s------%s ", arg3[i],arg[pipe_loc + i + 1]);
						printf("\n");
					}

					else{
						close(fd1[READ]);
						close(fd1[WRITE]);
					}
				}
				
				waitpid(child_pid1, NULL, 0);
				waitpid(child_pid2, NULL, 0);

				gettimeofday(&end, NULL);
				long seconds = (end.tv_sec - begin.tv_sec);
                                long micros = ((seconds * 1000000) + end.tv_usec) - (begin.tv_usec);


				printf("Time for normal mode => %ld microseconds\n", micros); 
			}

			else{
				child_pid1 = fork();

				if(child_pid1 == 0){
					execvp(arg[0], arg);
				}

				waitpid(child_pid1, NULL, 0);
			}
		}
		
		
		else if(mode == 2){
			if(pipe_bol){

                                char* arg2[pipe_loc + 1];
                                char* arg3[arg_size - pipe_loc];
				int character_count = 0, count, read_count = 0;
                                arg2[pipe_loc] = NULL;
                                for(i = 0; i < pipe_loc; i++){
                                        arg2[i] = arg[i];

                                //      printf("%s ",arg2[i]);
                                }
                                //printf("\nARG2\n");


                                for(i = 0; i < arg_size - pipe_loc; i++){
                                        arg3[i] = arg[pipe_loc + i + 1];

                                        //printf("%s ",arg3[i]);
                                }
                                //printf("\nARG3\n");

				gettimeofday(&begin, NULL);

                                if(pipe(fd1)== -1){
                                        printf("ERROR PIPE1 CREATION IN MODE2\n");
                                        return 1;
                                }

				if(pipe(fd2) == -1){
					printf("ERROR PIPE2 CREATION IN MODE2");
					return 1;
				}

				child_pid1 = fork();

				if(child_pid1 == 0){//child1
					close(fd2[READ]);
					close(fd2[WRITE]);
					close(fd1[READ]);
					
					dup2(fd1[WRITE], 1);
					close(fd1[WRITE]);

					execvp(arg2[0], arg2);
				}

				else{
					child_pid2 = fork();

					if(child_pid2 == 0){//child2
						close(fd1[READ]);
						close(fd1[WRITE]);
						close(fd2[WRITE]);

						dup2(fd2[READ], 0);
						close(fd2[READ]);

						execvp(arg3[0], arg3);
					}

					else{
						char read_msg[buffer];
				
						close(fd1[WRITE]);
						close(fd2[READ]);

						while((count = read(fd1[READ], read_msg, buffer))>0){
							write(fd2[WRITE], read_msg, count);
							
							character_count += count;
							read_count ++;
							//printf("%s ",read_msg);
						}
						read_count++; //At the end of the file read will return 0
						close(fd2[WRITE]);
						close(fd1[READ]);
						
						// When there is a read there is a write except for the last time when the read
						// call returned 0. That's why write-call is always equal to read-call - 1
					}
				
				}

				waitpid(child_pid1, NULL, 0);
				waitpid(child_pid2, NULL, 0);
				
				gettimeofday(&end, NULL);
				//long int total_time = end.tv_usec - begin.tv_usec;
				

    				long seconds = (end.tv_sec - begin.tv_sec);
    				long micros = ((seconds * 1000000) + end.tv_usec) - (begin.tv_usec);

			        printf("character_count: %d\n" ,character_count);
                                printf("read-call-count: %d\n" ,read_count);
                                printf("write-call-count: %d\n" ,read_count - 1);
                                // When there is a read there is a write except for the last time when the read
                                // call returned 0. That's why write-call is always equal to read-call - 1
                                      
				printf("Time for tapped mode => %ld microseconds\n", micros);
				
			}

			else{
				child_pid1 = fork();

                                if(child_pid1 == 0){
                                        execvp(arg[0], arg);
                                }

                                waitpid(child_pid1, NULL, 0);
			}
		
		
		
		}
	}
	
	return 0;
}
