#include <stdio.h>
#include <stdlib.h>

#define ROW_SIZE 1024 // Each row is of 1 KB

#define NUM_ROWS_0_5GB  1024*512
#define NUM_ROWS_1GB 1024*1024
#define NUM_ROWS_2GB 2*NUM_ROWS_1GB

char* USAGE_STRING = "Usage: ./matrix_transpose <mem in GB>";

int main(int argc, char** argv){
    if(argc != 2){
        printf("%s\n", USAGE_STRING);
        return -1;
    }
    int num_rows, i, j;
    char **matrix;
    char **transposed;

    // Get the number of rows
    // Get the number of rows
    switch(atoi(argv[1])){
        case 0:
            num_rows = NUM_ROWS_0_5GB;
            break;
        case 1:
            num_rows = NUM_ROWS_1GB;
            break;
        case 2:
            num_rows = NUM_ROWS_2GB;
            break;
        default:
            num_rows = NUM_ROWS_1GB;
    }
    // Allocate the memory
    matrix = (char**) calloc(1, num_rows * sizeof(char*));
    for(i = 0; i < num_rows; i++){
        matrix[i] = (char*) calloc(1, ROW_SIZE);
    }
    // Perform sequential write
    for(i = 0; i < num_rows; i++){
        for(j = 0; j < ROW_SIZE; j++){
            matrix[i][j] = rand();
        }
    }
    // Allocate memory for the transposed matrix
    transposed = (char**) calloc(1, ROW_SIZE * sizeof(char*));
    for(i = 0; i < ROW_SIZE; i++){
        transposed[i] = (char*) calloc(1, num_rows);
    }
    // Transpose matrix
    for(i = 0; i < num_rows; i++){
        for(j = 0; j < ROW_SIZE; j++){
            transposed[j][i] = matrix[i][j];
        }
    }
    // Free the memory
    for(i = 0; i < num_rows; i++){
        free(matrix[i]);
    }
    for(i = 0; i < ROW_SIZE; i++){
        free(transposed[i]);
    }
    free(matrix);
    free(transposed);
    return 0;
}