#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define ROW_SIZE 1024 // Each row is of 1 KB

#define NUM_ROWS_1GB 1024*1024
#define NUM_ROWS_2GB 2*NUM_ROWS_1GB

char* USAGE_STRING = "Usage: ./large_file <file_size in GB>";

int main(int argc, char** argv){
    if(argc != 2){
        printf("%s\n", USAGE_STRING);
        return -1;
    }
    int num_rows, i, j, fd;
    char c;

    // Get the number of rows
    switch(atoi(argv[1])){
        case 1:
            num_rows = NUM_ROWS_1GB;
            break;
        case 2:
            num_rows = NUM_ROWS_2GB;
            break;
        default:
            num_rows = NUM_ROWS_1GB;
    }

    fd = open("test_file.txt", O_WRONLY | O_CREAT);
    for(i = 0; i < num_rows; i++){
        for(j = 0; j < ROW_SIZE; j++){
            c = rand();
            write(fd, &c, sizeof(char));
        }
    }
    close(fd);

    return 0;
}