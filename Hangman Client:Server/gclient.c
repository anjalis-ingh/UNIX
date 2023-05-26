#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLEN 1000

int main(int argc, char *argv[]) {	
	// error statment for incorrect input 
	if (argc != 2) {
		puts("Usage: gclient <server-fifo-name>");
		exit(1);
	}

	// argv[1] is the server fifo name
    char clientfifo[MAXLEN];
	sprintf(clientfifo, "/tmp/%s-%d", getenv("USER"), getpid());
	mkfifo(clientfifo, 0600);
	chmod(clientfifo, 0622);
	
	// open argv[1] for writing, send clientfifo
	FILE *fp = fopen(argv[1], "w");
	fprintf(fp, "%s\n", clientfifo);
	fclose(fp);

	// open clientfifo for reading and read the quote
	FILE *clientfp = fopen(clientfifo, "r");

	// read the new server-fifo, then open it for writing! as serverfp
	char serverfifo[MAXLEN];
	fscanf(clientfp, "%s", serverfifo);
	char line[MAXLEN];
	fgets(line, MAXLEN, clientfp); 

	FILE *serverfp = fopen(serverfifo, "w");
	// variables 
	char wordString[MAXLEN], astString[MAXLEN];
	char letter = ' ', wordFound = 'n', printKey = '0';
	int misses = 0;
	
	// first time starting game, get wordString (test) and asterik string
	fscanf(clientfp, "%s", &wordString);
	fscanf(clientfp, "%s", &astString);
	
	// keep looping until word is found 
	while (wordFound == 'n') {
		// if word is not found, display prompt and get user input
		printf("(Guess) Enter a letter in word %s > ", astString);
		scanf(" %c", &letter);
		
		// send user letter to server
		fprintf(serverfp, "%c\n", letter);
		fflush(serverfp);

		// get wordFound status from server 
		fscanf(clientfp, " %c", &wordFound);

		// word found -> get final word and number of misses 
		if (wordFound == 'y') {
			fscanf(clientfp, "%s", &wordString);
			fscanf(clientfp, "%d", &misses);
		}
		// word not found -> get updated string after user guess 
		if (wordFound == 'n') {
			fscanf(clientfp, "%s", &astString);
		}
		
		// receive printKey from server and print appropriate response
		fscanf(clientfp, " %c", &printKey);
		if (printKey == '1') 
			printf("     %c  is already in the word\n", letter);
		if (printKey == '2')
			printf("     %c  is not in the word\n", letter);
		if (printKey == '3') {
			printf("The word is %s. You missed %d times\n", wordString, misses);
		}
	}	
	// close file and client connection 
	fclose(clientfp);
	unlink(clientfifo);
}
