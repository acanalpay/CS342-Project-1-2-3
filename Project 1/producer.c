#include <stdlib.h>
#include <stdio.h>
#include <time.h>    // time()
int main(int argc, char *argv[]){
	
	int m = atoi(argv[1]);
	int i;
	time_t t;
	srand((unsigned)time(&t));	
	for(i = 0; i < m; i++){
		char c = rand() % 26 + 97; // 'a' = 97 
		printf("%c",c);
	}	

	return 0;
}
