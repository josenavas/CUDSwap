#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <wait.h>
#include <sys/swap.h>

/*
	We create all the variables here in order to allocate
	the less amount of memory under OOM state, i.e. maintain
	the system between the 7% and the 3% thresholds
*/
int finish = 0;
char swapFileName[] = "/mnt/swap00";
int swapFileNum = 0;
unsigned long long swapFileSize = 1024 * 1024 * 2; // 2GB (we're writting 1KB at a time)
char* zero;
int fpSwapIds;
int fpSwapFile;
int fpWakeUp;
int pid;
int cum = 0;
int status;
int fpAux;
int len = 0;
char pidChar[10];
char c;
int creating_swap = 0;

void sigusr1_callback(int snum){
	// Check if we are creating a swap file
	if(creating_swap == 0){
		/*
	 	Update swapFileName
	 	Current kernels supports 32 swap files,
	 	so we don't need to check the 99 case.
		*/
		if(swapFileNum != 0)
			swapFileName[9] = '0' + (swapFileNum / 10);
		swapFileName[10] = '0' + (swapFileNum % 10);
		++swapFileNum;
		fprintf(stderr, "Signal for creating swap space received\n"); fflush(stderr);
	}
	else{
		fprintf(stderr, "We are already creating a swap file\n"); fflush(stderr);
	}
}

void sigusr2_callback(int snum){
	/* Finish the process */
	finish = 1;
	fprintf(stderr, "Signal for shutting down received\n"); fflush(stderr);
}

void create_swap(void){
	// Create file
	creating_swap = 1;
	fpSwapFile = open(swapFileName, O_WRONLY | O_CREAT, 00600);
	if(fpSwapFile == -1){
		/*
		 We cannot create the swap file,
		 return and let the OOM killer
		 kill the process, we can't do
		 anything...
		*/
		fprintf(stderr, "Open swap file failed\n"); fflush(stderr);
		return;
	}
	while (cum < swapFileSize) {
		if( write(fpSwapFile, zero, 1024) != 1024) {
			/*
			 We cannot write the entire file
			 Close it and return - let the OOM killer kill the process
			 we can't do anything...
			*/
			close(fpSwapFile);
			fprintf(stderr, "Write swap file failed\n"); fflush(stderr);
			return;
		}
		++cum; // Write the entire file in block of 1KB - usually efficient
	}
	cum = 0;
	close(fpSwapFile);
	// Format file - mkswap
	pid = fork();
	if(pid == 0) {
		// Child process - executes makeswap - redirects output to /dev/null
		fprintf(stderr, "Child process created %d\n", getppid()); fflush(stderr);
		fpAux = open("/dev/null", O_RDWR);
		dup2(fpAux, 0);
		dup2(fpAux, 1);
		close(fpAux);
		execl("/sbin/mkswap", "mkswap", swapFileName, NULL);
		exit(1);
	}
	// Parent process - check if mkswap finished successfully
	wait(&status);
	if(status != 0){
		/* mkswap failed - let the OOM killer kill the process */
		fprintf(stderr, "mkswap process failed\n"); fflush(stderr);
		return;
	}
	// Activate swap space - swapon
	// If the process fails or it succeeds - we finish anyway
	if(swapon(swapFileName, 0)){
		fprintf(stderr, "Swapon failed\n"); fflush(stderr);
	}
}

void wake_up_processes(void) {
	fpSwapIds = open("/proc/CUDSwap", O_WRONLY);
	if(fpSwapIds == -1){
		fprintf(stderr, "Opening /proc/CUDSwap failed\n"); fflush(stderr);
		return;
	}
	c = '1';
	write(fpSwapIds, &c, 1);
	close(fpSwapIds);
	creating_swap = 0;
	fprintf(stderr, "Write /proc/CUDSwap success\n"); fflush(stderr);
}

void cleanup_swap_spaces(void) {
	int i;
	fprintf(stderr, "Cleaning up swap spaces\n"); fflush(stderr);

	for(i = 0; i < swapFileNum; i++){
		if(i != 0)
			swapFileName[9] = '0' + (i / 10);
		swapFileName[10] = '0' + (i % 10);
		// Swapoff the swap space
		swapoff(swapFileName);
		// Remove the swap file
		unlink(swapFileName);
	}
}

int main(){
	// Using calloc because it effectively allocates the memory and zeroes it
	zero = calloc(1, 1024);

	// Register the signal we expect and the function to execute
	signal(SIGUSR1, sigusr1_callback);
	signal(SIGUSR2, sigusr2_callback);

	creating_swap = 0;

	while(1){
		// Wait until a swap space is needed
		pause();
		// Check if we have to finish withouth creating a swap space
		if(finish)
			break;
		// Swap space needed!!! Create one
		create_swap();
		// Swap space added!!! Wake up all the processes
		wake_up_processes();
	}
	// We've finished our work - cleanup the swap spaces
	cleanup_swap_spaces();

	return 0;
}
