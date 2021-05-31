#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]){
	
	int m = atoi(argv[1]);
	int i;
	char a[m];
	for(i = 0; i < m; i++){ 
		scanf("%c",&a[i]);
	}
	//printf("Consumer output: %s\n",a);
	return 0;
}
