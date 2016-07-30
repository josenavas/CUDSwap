#include <stdio.h>
#include <stdlib.h>

#define N 1
#define ROW_SIZE 1024
//#define NUM_ROWS 1024 // Allocating 1MB
//#define NUM_ROWS 1024*256 // Allocating 256 MB
//#define NUM_ROWS 1024*512 // Allocating 512MB
#define NUM_ROWS 1024*2014 // Allocating 1GB
//#define NUM_ROWS 2*1024*1024 // Allocating 2GB
//#define NUM_ROWS 4*1024*1024 // Allocating 4GB

int main(int argc, char** argv){
	int counter[10];
	char **m1;
	unsigned int i, j, k;

	// Iterate 20 times to see the module impact
	// Our module has impact over the allocation
	for(k = 0; k < N; k++){
		// Allocate the memory
		m1 = (char**) calloc(1, NUM_ROWS * sizeof(char*));
		for(i = 0; i < NUM_ROWS; i++){
			m1[i] = (char*) calloc(1, ROW_SIZE);
		}

		// Randomize each value of the char matrix
//		for(i = 0; i < NUM_ROWS; i++){
//			for(j = 0; j < 1024; j++) {
//				m1[i][j] = '0' + (rand() % 10);
//			}
//		}

//		for(i = 0; i < 10; i++){
//			counter[i] = 0;
//		}

//		for(i = 0; i < NUM_ROWS; i++){
//			for(j = 0; j < 1024; j++){
//				++counter[m1[i][j] - '0'];
//			}
//		}

		// Free de memory
		for(i = 0; i < NUM_ROWS; i++){
			free(m1[i]);
		}
		free(m1);
	}


	for(i = 0; i < 10; i++){
		printf("%d = %d\n", i, counter[i]);
	}
	
	return 0;
}
