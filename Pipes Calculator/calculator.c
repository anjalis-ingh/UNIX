#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <sys/wait.h>

#define MAXLEN 50000

char operators[100];
int fds[100][2];
int operatorCount = 0;
int numPipes = 0;

void child(int index) {
	// fds[0] donates data to fd 0 and 3 
    // fds[1] donates data to stdout 
    dup2(fds[index * 3][0], 0);
    dup2(fds[index * 3 + 1][0], 3);
    dup2(fds[index * 3 + 2][1], 1);

	// stdin has a copy -> fds[0] no longer needed
	// fds[1] -> unused in child
	int j = 0;
	for(j = 0; j < operatorCount; j++) {
		close(fds[j * 3][0]); close(fds[j * 3][1]);
    	close(fds[j * 3 + 1][0]); close(fds[j * 3 + 1][1]);
    	close(fds[j * 3 + 2][0]); close(fds[j * 3 + 2][1]);
	 }
    
    // execl to appropriate operator 
	if (operators[index] == '+') 
    	execl("add", "add",  NULL);

    else if (operators[index] == '-')
		execl("subtract", "subtract",  NULL);
	
	else if (operators[index] == '*')
		execl("multiply", "multiply",  NULL);
	
	else if (operators[index] == '/')
		execl("divide", "divide",  NULL);
   
	// exit if somthing goes wrong 
	fprintf(stderr, "error!\n");
    exit(1);
}

int main(int argc, char *argv[]) {
	// variables
	char line[MAXLEN], *temp;
	int total, i, j;
	int count = 2;
	
	// wrong format
    if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(1);
	}
	
	// open the file and read the first line - it contains the configuration
	FILE *dataFile = fopen(argv[1], "r");
	fgets(line, MAXLEN, dataFile); 

	// put characters in a char[] 
	strtok(line, " \n"); 
	
	// keep looping until end of line reached 
	// find the number of operators in the line 
	while ( (temp = strtok(NULL, " \n")) ){
		operators[operatorCount] = temp[0];
		operatorCount++;
		// skip the symbol representing variable/parameter
		strtok(NULL, " \n"); 
	}
	
	// read the numbers from the file and store in array 
	char data[MAXLEN];
	int numbers[MAXLEN];
	int num = 0;
	int lines = 0; 

    while (fgets(data, sizeof data, dataFile)) {
        char *a = data;    
        while (1) {
            char *b;
            errno = 0;
            numbers[num] = strtol(a, &b, 10);
    
            if (b == a || errno) 
                break;
    
            a = b;
            num++;
        }   
        lines++;
    }
	
	// find the amount of numbers per line in the file 
	int numPerLine = num / lines;

	// create the necessary # of pipes -> 2n + 1 where n = operators 
	numPipes = operatorCount * 3;
	
	// populate the with fds
	for (i = 0; i < numPipes; i++) 
        pipe(fds[i]);
	
	// solve the expression - loop over lines and # of operators
	for (i = 0; i < lines; i++) {
		for (j = 0; j < operatorCount; j++) {
			// create child process
			int pid = fork();

			// child process
			if (pid == 0) 
       			child(j);	  
	 
	 		// parent process - cleanup
	 		close(fds[j*3][0]);
	 		close(fds[j*3 + 1][0]);
	 		close(fds[j*3 + 2][1]);
	 
	 		// write input to pipes and read output
			if (j == 0) {
	 			write(fds[j*3][1], &numbers[i*numPerLine], sizeof(int));
	 			write(fds[j*3 + 1][1], &numbers[i*numPerLine + 1], sizeof(int));
	 			read(fds[j*3 + 2][0], &total, sizeof(int));
			}
			else {
				write(fds[j*3][1], &total, sizeof(int));
				write(fds[j*3 + 1][1], &numbers[i*numPerLine + count], sizeof(int));
				read(fds[j*3 + 2][0], &total, sizeof(int));
				count++;
			}
		}
		// print total result after each line
		printf("%d\n", total);

		// reset counter
		count = 2;
	}

	// close the file
	fclose(dataFile);
}
