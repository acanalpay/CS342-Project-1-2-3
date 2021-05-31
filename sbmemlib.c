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
// Define a name for your shared memory; you can give any name that start with a slash character; it will be like a filename.
#define SHARED_NAME "/shared_memory_name"
#define SEM_OPEN "/semaphore_open"
#define SEM_MUTEX "/semaphore_mutex"
#define SEM_CNT "/semaphore_count"
// Define semaphore(s)
//sem_t* mutex1;
sem_t* mutex2; // For accessing shared memory
//sem_t* mutex3;

// Define your stuctures and variables. 
//struct stat sbuf;

struct inner_list{
	pid_t pid;// If not -1, the memory is allocated
	int length;
	int id;
	void* base;
	void* pointed;
	//int index;
};


int sbmem_init(int segmentsize){
	
	shm_unlink(SHARED_NAME); // Delete the previously allocated memory -- if any
	//shm_unlink(SEM_OPEN);
	shm_unlink(SEM_MUTEX);
	sem_destroy(mutex2);
    	int fd = shm_open(SHARED_NAME, O_RDWR | O_CREAT, 0666);//0660
	if(fd < 0)
		return -1;

	//Maintenance Size:
	int maintenance_size = 4096 * sizeof(struct inner_list);
	maintenance_size += 4 * sizeof(int); //for size and offset storage
       		
	ftruncate(fd, segmentsize + maintenance_size);
	
	//struct stat sbuf;	
	//fstat(fd, &sbuf);
	//printf("=====> %d\n", sbuf.st_size);	
	
	
	void* ptr = mmap(NULL, segmentsize + maintenance_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	int* x = (int *)ptr;

	x[0] = segmentsize; //We store the size of the shared memory
	x[1] = maintenance_size; //We store the maintenance memory / offset
	x[2] = 1; // Current number of struct
	x[3] = 10; // process number


	struct inner_list* il = (struct inner_list *) (ptr + 16);
	//insert the first size -> memory itself
	il->pid = -1;
	il->length = segmentsize;
	il->id = 1;
	il->pointed = ptr + x[1];
	il->base = ptr + x[1];
	

	//mutex1 = sem_open(SEM_OPEN, O_RDWR | O_CREAT, 0660, 5);
	
	//printf("%d %d ilk deger\n",il->id, il->length);
	/*if(mutex1 < 0){
		perror("connot create semaphore1\n");
		return -1;		
	}*/

	mutex2 = sem_open(SEM_MUTEX, O_RDWR | O_CREAT, 0660, 1);
	if(mutex2 < 0){
		perror("cannot create semaphore2\n");
		return -1;
	}

	/*mutex3 = sem_open(SEM_CNT, O_RDWR | O_CREAT, 0660, 1);
        if(mutex3 < 0){
                perror("cannot create semaphore3\n");
                return -1;
        }

	*/
	close(fd);
	return (0); 
}

int sbmem_remove(){
	shm_unlink(SHARED_NAME);	
	sem_destroy(mutex2);
	sem_unlink(SEM_MUTEX);
	//sem_destroy(mutex1);
	//sem_destroy(mutex3);
	return (0); 
}

int sbmem_open(){
	
	mutex2 = sem_open(SEM_MUTEX, O_RDWR);
	
	//printf("XXX\n");
	
	sem_wait(mutex2);
	//printf("BBB\n");
	int fd = shm_open(SHARED_NAME, O_RDWR, 0666);
	void* ptr = mmap(NULL, 20, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	int* x = (int *)ptr;
	printf("ACCESS REQUEST ==> %d\n", getpid());
	if(x[3] <= 0){// ten process already use the shared memory
		munmap(ptr, 20);
		close(fd);
		sem_post(mutex2);
		return -1;
	}
	printf("ACCESS GRANTED\n");
	x[3] = x[3] - 1;

	munmap(ptr, 20);

	sem_post(mutex2);
	
	close(fd);
	return (0); 
}

void *sbmem_alloc (int size){
    	
	// Find the min power of 2 that is >= size
	int pow2 = 1;

	while(pow2 < size)
		pow2 *= 2;
	
	sem_wait(mutex2);
	
	
	int fd = shm_open(SHARED_NAME, O_RDWR, 0666);
	void* ptr = mmap(NULL, 20, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		
	int *x = (int*)ptr;
	//x[0] = size
	//x[1] = offset
	//x[2] = struct number
	//x[3] = process number
		
	int siz = x[0] + x[1];
	
	munmap(ptr, 20);
	
	ptr = mmap(NULL, siz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	x = (int*) ptr;
	struct inner_list *il = (struct inner_list*) (ptr + 16);
	struct inner_list *cur = il;
	struct inner_list *loc = il;
	//printf("While basi %d\n", getpid());	
	
	int i = 0;
	while(i < x[2]){
		cur = il + i;
		
		if(cur->pid == -1 && cur->length > pow2){
			if((loc->pid != -1)||(loc->pid == -1 && loc->length > cur->length) || (loc->length < pow2)){
				loc = cur;	
			}
		}
		
		if(cur->pid == -1 && cur->length == pow2){
			cur->pid = getpid();
			
			void* ret = ptr + x[1];
			struct inner_list* lc = il;
			int sum = 0, j = 1;

			while(j <= i){
				sum += lc->length;
				lc = lc+1;
				j++;
			}
			ret += sum;
			close (fd);
		        printf("Allocation for %d, size = %d ==> %d\n",getpid(), size, cur->length);
			
			cur->pointed = ret;
			cur->base = ptr + x[1];
			sem_post(mutex2);
			return ret;
		}
		i++;		
	}
	//printf("BURAYA UGRADI %d\n", getpid());
	
	if(loc->pid != -1) { // NO MEMORY IS AVAILABLE
		printf("Could not allocate %d for %d\n", size, getpid());
		close(fd);
		sem_post(mutex2);
		return NULL;
	}

	// Divide the location
	// loc is the location that will be divided
	void* ret = ptr + x[1];
	struct inner_list* lc = il;
	int sum = 0;
	loc->base = ret;
	i = 0;
	while( i < x[2] && lc != loc){
		sum += lc->length;
		lc = lc + 1;
		i++;
	}
	ret += sum;
	loc->pointed = ret;
	//printf("DIVISIONA BASLAYACAK %d\n",getpid());
	
	while(loc->length > pow2){
		
		//lc = loc;	
		//i = 1;
		//while(i < x[2]){
		//	lc = lc + 1;
		//	i++;
		//}
		//void* p = ptr + 16 + sizeof(struct inner_list) * x[2];
		
		lc = il + x[2] - 1;
		struct inner_list* new = il + x[2];
		x[2] = x[2] + 1;
				
		loc->id *= 2;
		loc->length /= 2;
		
		while(lc != NULL && new != loc){
			new->pid = lc->pid;
			new->length = lc->length;
			new->id = lc->id;
			new->base = lc->base;
			new->pointed = lc->pointed;
			new = lc;
			lc = lc - 1;
		}
		
		// new is the loc
		(loc+1)->id = loc->id + 1;
		(loc+1)->pointed = loc->pointed + loc->length;			
                //printf("=== %d %d\n",loc->id, loc->length);

	}
	
	printf("Allocation for %d, size = %d ==> %d\n",getpid(), size, loc->length);
	
	loc->pid = getpid();
	close(fd);
	//printf("POSTING MUTEX %d\n",getpid());
	
	loc = il;
	i = 0;/*
	while(i < x[2]){
		printf("%p %d\n",loc, loc->length);
		loc= lc+1;
		i++;
	}*/
	sem_post(mutex2);
	
	return ret;
}


void sbmem_free (void *p){
	
	sem_wait(mutex2);

	int fd = shm_open(SHARED_NAME, O_RDWR, 0666);
	void* ptr = mmap(NULL, 16, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	//printf("AAA\n");
	int* x = (int *) ptr;
	//x[0] = size
	//x[1] = offset
	
	int siz = x[0] + x[1];

	munmap(ptr, 16);

	ptr = mmap(NULL, siz, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	x = (int *)ptr;
	
	struct inner_list* il = (struct inner_list*) (ptr + 16);
	struct inner_list* find = il;
	
	struct inner_list* print = il;
	int i = 0;

	printf("Before freeing the allocated memory, the memory is (-1 means empty):\n");
	while(i < x[2]){
		printf("%d pid, %d memory size\n", print->pid,print->length);
		print = print + 1;
		i++;	
	}
	//printf("_______________\n");
	i = 0;
	while(i < x[2]){
		if(find->pointed == p)
			break;
		i++;
		find = find+1;
	}

	if(i == x[2]){
		printf("Pointer not address to a memory\n");
		return;
	}
	
	find->pid = -1;
	int j = -1;
	
	int high = i+1, low = i;
	while((high < x[2] && (find+1)->pid == -1) || (low > 0  && (find-1)->pid == -1)){
		
		if(high<x[2] && (find+1)->id /2 == (find->id)/2 && (find+1)->pid == -1){
			find->length *= 2;
			find->id /= 2;
			
			struct inner_list* cur = find+1;
			int k = 1;
			
			while(k < x[2]){
				cur->id = (cur+1)->id;
				cur->base = (cur+1)->base;
				cur->pointed = (cur+1)->pointed;
				cur->length = (cur+1)->length;
				cur->pid = (cur+1)->pid;			
				k++;
				cur = cur + 1;
			}

			//cur = cur-1;
			//cur + 1 = NULL;
			x[2] = x[2] - 1;		
		}
		else if(low > 0 && (find-1)->id/2 == (find->id) /2  && (find-1)->pid == -1){
			//printf("=> ID: %d %d\n",(find-1)->id,(find->id));
			find = find-1;
			find->length *= 2;
			find->id /= 2;
			
			struct inner_list* cur = find+1;
			
			int k = 1;
                        while(k < x[2]){
                                cur->id = (cur+1)->id;
                                cur->base = (cur+1)->base;
                                cur->pointed = (cur+1)->pointed;
                                cur->length = (cur+1)->length;
                                cur->pid = (cur+1)->pid;
				k++;
                                cur = cur+1;
                        }
			
                        //cur = cur-1;
                        //cur+1 = NULL;
                        x[2] = x[2] - 1;
			low--;
			
		}
		else{
			break;
		}
	}
	if(x[2] == 2 && il->pid == -1 && (il+1)->pid == -1 && il->id/2 == (il+1)->id/2){
		x[2] = x[2]-1;
		il->id = il->id/2;
		il->length *=2;
	}
	//printf("%d %d %d\n",high,x[2],low);
	close(fd);
	
	print = il;

	printf("After freeing the memory, new memory:\n");
	i = 0;
	while(i < x[2]){
		printf("%d pid, %d memory\n",print->pid, print->length);
		print = print+1;
		
		i++;
	}

	sem_post(mutex2);
}

int sbmem_close(){
   	sem_wait(mutex2);
	//sem_post(mutex1);
        int fd = shm_open(SHARED_NAME, O_RDWR, 0666);
	
	void* ptr = mmap(NULL, 20, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	
	int* x = (int*)ptr;
	x[3] = x[3] + 1;

	munmap(ptr, 20);
	close(fd);	
	sem_post(mutex2);

    	return (0); 
}
