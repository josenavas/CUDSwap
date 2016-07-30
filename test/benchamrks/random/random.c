#include <stdio.h>
#include <stdlib.h>

#define ROW_SIZE 1024 // Each row is of 1 KB

#define NUM_ROWS_1GB 1024*1024
#define NUM_ROWS_2GB 2*NUM_ROWS_1GB
#define NUM_ROWS_3GB 3*NUM_ROWS_1GB
#define NUM_ROWS_4GB 4*NUM_ROWS_1GB

char* USAGE_STRING = "Usage: ./random <mem in GB> <num reps>";

int main(int argc, char** argv){
    if(argc != 3){
        printf("%s\n", USAGE_STRING);
        return -1;
    }
    int num_rows, i, j, k, counter, idx1, idx2;
    char **matrix;
    int num_reps = atoi(argv[2]);

    // Get the number of rows
    switch(atoi(argv[1])){
        case 1:
            num_rows = NUM_ROWS_1GB;
            break;
        case 2:
            num_rows = NUM_ROWS_2GB;
            break;
        case 3:
            num_rows = NUM_ROWS_3GB;
            break;
        case 4:
            num_rows = NUM_ROWS_4GB;
            break;
        default:
            num_rows = NUM_ROWS_1GB;
    }
    for(k=0; k < num_reps; k++){
        // Allocate the memory
        matrix = (char**) calloc(1, num_rows * sizeof(char*));
        for(i = 0; i < num_rows; i++){
            matrix[i] = (char*) calloc(1, ROW_SIZE);
        }
        // Perform random write
        for(i = 0; i < num_rows; i++){
            for(j = 0; j < ROW_SIZE; j++){
                idx1 = rand() % num_rows;
                idx2 = rand() % ROW_SIZE;
                matrix[idx1][idx2] = rand();
            }
        }
        // Perform random read
        counter = 0;
        for(i = 0; i < num_rows; i++){
            for(j = 0; j < ROW_SIZE; j++){
                idx1 = rand() % num_rows;
                idx2 = rand() % ROW_SIZE;
                counter += matrix[idx1][idx2];
            }
        }
        // Free the memory
        for(i = 0; i < num_rows; i++){
            free(matrix[i]);
        }
        free(matrix);
        printf("Result: %d\n", counter);
    }
    return 0;
}