#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/signal.h>
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/time.h>
#include <math.h>


struct node{
	pthread_t index;
	int bindex;
	long int length;
	double wall_clock;
	
	struct node* next;
};

struct bound_list{
	struct node* head;
	struct node* tail;
	pthread_mutex_t th_mutex;
	
	pthread_cond_t th_cond_scheduler;
};

struct bound_list *rq;
char file[50];
int flag = 0;
int n,bsize,minB,avgB,minA,avgA, type;
int thread_jobs[15]; // Max thread number is 10

unsigned long long int thread_waits[15];

void* create_random(void* arg){
	int i;
	srand(pthread_self());

	for (i = 0; i < bsize; i++){
		double x;
		//Start with sleeping
		do{
                        x = (-avgA) * log(1 - (rand()/(RAND_MAX + 1.)));
                }while(x < minA);

                usleep((unsigned int) x * 1000);
		
		struct node* l = (struct node*)malloc (sizeof(struct node));
		l->index = (long int)arg;
		l->bindex = i;
		l->next = NULL;
		
		//BURST TIME
		
		do{
			x = (-avgB) * log(1 - (rand()/(RAND_MAX + 1.)));		
		}while(x < minB);
		
		l->length = (long int) x;

		//GET THE TIME
		struct timeval current_time;
		gettimeofday(&current_time,NULL);
		l->wall_clock = current_time.tv_usec * 0.001 + (current_time.tv_sec * 1000);
		
		pthread_mutex_lock(&(rq->th_mutex));
		if(rq->head == NULL){
			rq->head = l;
			rq->tail = l;
			rq->head->next = NULL;
			//signal that the list is not empty anymore
			pthread_cond_signal(&(rq->th_cond_scheduler));
		}

		else{
			rq->tail->next = l;
			rq->tail = rq->tail->next;
		}

		printf("Thread %ld  JobID %d , create => %ld ms\n",l->index,l->bindex, l->length);

		pthread_mutex_unlock(&rq->th_mutex);
		
		//printf("Thread %ld  JobID %d , create => %ld ms\n",l->index,l->bindex, l->length);
	}

	pthread_exit(NULL);
}

void* take_jobs(void* arg){
	/*
	 * type = 1 => FCFS
	 * type = 2 => SJF
	 * type = 3 => PRIO
	 * type = 4 => VRUNTIME
	 */ 	
	int i, t_size = n;
	double vruntime[n];

	for(i = 0; i < n; i++)
		vruntime[i] = 0;
	
	int x = 0;
	int size = bsize;

	if(!flag)
		size *=n;

	while(x < size){
		pthread_mutex_lock(&rq->th_mutex);
	
		//Wait if the list is empty
		while(rq->head == NULL){
			printf("Waiting for a job %d - %d\n",x, size);
			pthread_cond_wait(&rq->th_cond_scheduler, &rq->th_mutex);
		}
		struct node* n;
	       
		if(type == 1){//FCFS
			n= rq->head;
	
			rq->head = rq->head->next;
	
			
			if(rq->head == NULL)
				rq->tail = NULL;

		}

		else if(type == 2){//SJF
			struct node* cur = rq->head;
			struct node* prev = NULL;
			struct node* cur_prev = rq->head;

			int x = cur->length;
			int j = 0,in = 0;
			

			while(cur != NULL){
				
				if(x > cur->length){
					prev = cur_prev;
					x = cur->length;
				}
				
				cur_prev = cur;
				cur = cur->next;
			}

			if(rq->head->next == NULL){
                                n = rq->head;
                                rq->head = NULL;
                                //printf("A\n");
                                rq->tail = NULL;
                        }

			else if(prev == NULL){//head is selected
				n = rq->head;
				if(rq->head->next == NULL){
					rq->head = NULL;
					rq->tail = NULL;
				}
				else
					rq->head = rq->head->next;
			}

			else if(prev->next->next == NULL){
				rq->tail = prev;
				n = prev->next;
				//printf("B\n");
				rq->tail->next = NULL;
			}
			else{
				n = prev->next;
				//printf("C\n");
				prev->next = prev->next->next;
			}

		}

		else if(type == 3){//PRIO
			struct node* cur = rq->head;
			struct node* prev = NULL;
                        struct node* cur_prev = rq->head;


			int in1 = cur->index, in2 = cur->bindex;

			while(cur!= NULL){
				if(in1 > cur->index || (in1 == cur->index && in2 > cur->bindex)){
                                        prev = cur_prev;
                                        in1 = cur->index;
					in2 = cur->bindex;
                                }

                                cur_prev = cur;
                                cur = cur->next;
                        }

                        if(prev == NULL){//head is selected
                                n = rq->head;
                                if(rq->head->next == NULL){
                                        rq->head = NULL;
                                        rq->tail = NULL;
                                }
                                else
                                        rq->head = rq->head->next;
                        }

                        else if(rq->head->next == NULL){
                                n = rq->head;
                                rq->head = NULL;
                                rq->tail = NULL;
                        }
                        else if(prev->next->next == NULL){
                                rq->tail = prev;
                                n = prev->next;
                                rq->tail->next = NULL;
                        }
                        else{
                                n = prev->next;
                                prev->next = prev->next->next;
                        }
		}
		
		else{
			struct node* cur = rq->head;
                        struct node* prev = NULL;
                        struct node* cur_prev = rq->head;

			double x = vruntime[cur->index - 1];
			
                        
                        while(cur!= NULL){
                                if(x > vruntime[cur->index - 1]){
                                        prev = cur_prev;
                                        x = vruntime[cur->index - 1];
                                }

                                cur_prev = cur;
                                cur = cur->next;
                        }
			if(rq->head->next == NULL){
				n = rq->head;
				rq->head = NULL;
				rq->tail = NULL;
			}
			else if(prev == NULL){//head is selected
                                n = rq->head;
                                if(rq->head->next == NULL){
                                        rq->head = NULL;
                                        rq->tail = NULL;
                                }
                                else
                                        rq->head = rq->head->next;
                        }
			
                        else if(prev->next->next == NULL){
                                rq->tail = prev;
                                n = prev->next;
                                rq->tail->next = NULL;
                        }
                        else{
                                n = prev->next;
                                prev->next = prev->next->next;
                        }
			
			vruntime[n->index - 1] += (n->length) * (0.7 + 0.3*(n->index));
			printf("New vruntime's are: ");
			for(i = 0; i < t_size; i++)
				printf("%d = %lf ",(i + 1), vruntime[i]);
			printf("\n");			
		}
		
		struct timeval current_time;
                gettimeofday(&current_time,NULL);
               
		long int waiting_time = current_time.tv_usec * 0.001 + (current_time.tv_sec * 1000)  - n->wall_clock;
		thread_waits[n->index] +=waiting_time;		
		printf("\n----- Thread %ld JobID %d run for %ldms time\nWaiting time is %ldms\n\n",n->index, n->bindex, n->length,waiting_time);
	    	pthread_mutex_unlock(&rq->th_mutex);

		usleep(n->length * 1000);
		
		free(n);
		x++;
	}
	
	for(i = 0; i < n; i++)
		printf("VRUNTIME For Thread%d is %lf\n",(i+1), vruntime[i]);

	pthread_exit(NULL);	
}

void* read_file(void* arg){
	char filename[50];
        strcpy(filename,file);
   	strcat(filename, "-");
        char number[2];
    	sprintf(number,"%ld", (long int) arg);
      	strcat(filename, number);
       	strcat(filename, ".txt");

	//printf("%s\n",filename);
	
	flag = 1;
	FILE *fp;

	fp = fopen(filename, "r");
	if(fp == NULL){
		perror("Cannot find the file");
		exit(1);
	}
	
	int i = 0,numberA, numberB;
	
	while(fscanf(fp, "%d %d",&numberA, &numberB) == 2){
	
		printf("%ld is slept for %dms\n",(long int) arg, numberA);

		usleep(numberA * 1000);
		
		struct node* n = (struct node*)malloc (sizeof(struct node));
		
		n->index = (long int) arg;
		n->bindex = i;
		n->length = numberB;
		n->next = NULL;
		
		struct timeval current_time;
                gettimeofday(&current_time,NULL);
                n->wall_clock = current_time.tv_usec * 0.001 + (current_time.tv_sec * 1000);

		printf("Thread %ld, JobID %d, burst_time = %d is created!\n", (long int) arg, i, numberB);
		
		pthread_mutex_lock(&(rq->th_mutex));
                if(rq->head == NULL){
                        rq->head = n;
                        rq->tail = n;
                        rq->head->next = NULL;
                        //signal that the list is not empty anymore
                        pthread_cond_signal(&(rq->th_cond_scheduler));
                }

                else{
                        rq->tail->next = n;
                        rq->tail = rq->tail->next;
                }

                pthread_mutex_unlock(&rq->th_mutex);

		i++;
	}
	
	fclose (fp); 
	//printf("FILE CLOSED!\n");
	
	pthread_exit(NULL);
	
}

void line_count(){
	int i = 1;
	int total = 0;
	
	while(i <= n){
		char filename[50];
	        strcpy(filename,file);
        	strcat(filename, "-");
        	char number[2];
        	sprintf(number,"%ld", (long int) i);
        	strcat(filename, number);
        	strcat(filename, ".txt");

        	//printf("%s\n",filename);


	        FILE *fp;

        	fp = fopen(filename, "r");
        	if(fp == NULL){
                	perror("Cannot find the file");
                	exit(1);
        	}
		
		int a,b;
		int count = 0;
		while(fscanf(fp,"%d %d",&a,&b) == 2)
			count++;
		
		thread_jobs[i] = count;	
		total += count;
		thread_waits[i] = 0;
		
		fclose(fp);
		i++;
	}

	bsize = total;
	printf("%d\n",bsize);	
}

int main(int argc, char **argv){
	
	pthread_t scheduler;
	
	pthread_t worker[10];
	
	rq = (struct bound_list*)malloc(sizeof(struct bound_list));

	rq->head = NULL;
	if(pthread_mutex_init(&(rq->th_mutex), NULL) != 0){
		printf("mutex init ERROR!");
		return 1;
	}
	
	pthread_cond_init(&rq->th_cond_scheduler, NULL);

	int i, x = 7;
	n = atoi(argv[1]);
	if(!strcmp(argv[3],"-f")){
		x = 2;
		strcat(file, argv[4]);
		line_count();
	}
	
	if(!strcmp(argv[x], "FCFS")){
		type = 1;
	}
	else if(!strcmp(argv[x], "SJF")){
		type = 2;	
	}
	else if(!strcmp(argv[x], "PRIO")){
		type = 3;
	}
	else{
		type = 4;
	}

	if(x == 7){
		bsize = atoi(argv[2]);

		for(i = 0; i < n; i++)
			thread_jobs[i + 1] = bsize;

		bsize = bsize;
		minB = atoi(argv[3]);
		avgB = atoi(argv[4]);
		minA = atoi(argv[5]);
		avgA = atoi(argv[6]);
	}

		
	for(i = 0; i < n; i++){
				
		int error;
		if(x == 7)
			error = pthread_create(&(worker[i]), NULL, &create_random, (void *)(long int) (i + 1));
		else{
				
			error = pthread_create(&(worker[i]), NULL, &read_file, (void *) (long int) (i+1));
			//printf("SUCCESS");
		}
		if(error < 0){
			perror("thread creation failed\n");
			exit(1);
		}
		
	}

	int error = pthread_create(&scheduler, NULL, &take_jobs, NULL);

	if(error<0){
		perror("thread creation failed-scheduler\n");
		exit(1);
	}

	for(i = 0; i < n; i++)
		pthread_join(worker[i], NULL);

	pthread_join(scheduler, NULL);
	pthread_mutex_destroy(&rq->th_mutex);
	pthread_cond_destroy(&rq->th_cond_scheduler);
		
	printf("FINISHED\n");
	
	free(rq);

	for(i = 1; i <= n; i++){
		double wait = (double)thread_waits[i]/(double)thread_jobs[i];
		printf("For thread %d: Average waiting time is: %.4lfms\n",i,wait);
	
		printf("%lld %d\n",thread_waits[i], thread_jobs[i]);
	}


	return 0;
}

