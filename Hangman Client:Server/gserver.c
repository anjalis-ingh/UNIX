#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXWORDS 100000
#define MAXLEN 1000

// stores all words from dictionary.txt
// wordString = hello, astString = *****
char *words[MAXWORDS];
char wordString[MAXLEN], astString[MAXLEN];

// global variables 
int numWords = 0, randomNum = 0, misses = 0;
int unexposed = 0, letterFound = 0;
char wordFound = 'n', printKey = '0';

// function to read random generated word and store in wordString and astString 
void generateWordString() {
	int i;
	for (i = 0; i < strlen(words[randomNum]) - 1; i++) {
		wordString[i] = words[randomNum][i];	
		astString[i] = '*';
	}
	// how many asteriks are there at the start 
	unexposed = strlen(astString);
}

// function to play the hangman game
// use user input letter and compare with string, update string and printKey
void playHangman(char letter) {
	// if word is not found, there are still asteriks  
	if (unexposed > 0) {
		int i;
		for (i = 0; i < strlen(astString); i++) {
			// letter is in the word -> letterFound = true
			if (letter == wordString[i]) {
				letterFound = 1;
				// letter is already in the word 
				if (letter == astString[i]) {
					printKey = '1';
					break;
				}
				// found a new letter, update string, decrement unexposed counter  
				else {
					printKey = '0';
					astString[i] = letter;
					unexposed--;
				}
			}
		}
		// did the whole for loop, could not find the letter in the word
		// wrong guess -> increment misses counter 
		if (!letterFound) {
			printKey = '2';
			misses++;
		}
	}
	
	// no more asteriks, word is found -> wordFound set to true
	if (unexposed == 0) {
		printKey = '3';
		wordFound = 'y';
	}

	// reset letterFound
	letterFound = 0; 
}

int main() {
	char line[MAXLEN];
	// open the dictionary file for reading
	FILE *fp = fopen("dictionary.txt", "r");
	if (!fp) {
		puts("dictionary.txt cannot be opened for reading.");
		exit(1);
	}

	int i = 0;
	// read each word from file and store in words[] array
	while (fgets(line, MAXLEN, fp)) {
		words[i] = (char *) malloc (strlen(line)+1);
		strcpy(words[i], line);
		i++;
	}

	numWords = i;
	srand(getpid() + time(NULL) + getuid());
	
	// create a named pipes to read client's requests
	// filename: client knows where to send requests to server 
	char filename[MAXLEN];
	sprintf(filename, "/tmp/%s-%d", getenv("USER"), getpid());
	mkfifo(filename, 0600);
	chmod(filename, 0622);
	printf("Send your requests to %s\n", filename);

	while (1) {
		// open filename for which has server fifo name 
		FILE *fp = fopen(filename, "r");
		if (!fp) {
			printf("FIFO %s cannot be opened for reading.\n", filename);
			exit(2);
		}
		printf("Opened %s to read...\n", filename);

		// wait for clients' requests
		// reads fp (filename) contents 
		while (fgets(line, MAXLEN, fp)) {
			char *cptr = strchr(line, '\n');
			if (cptr) 
				*cptr = '\0';
	
			// create a child to work with this client
			int pid = fork();
			if (pid == 0) {
				FILE *clientfp = fopen(line, "w");
				// create and send new server fifo to the client for private comm
				char serverfifo[MAXLEN];
				sprintf(serverfifo, "/tmp/%s-%d", getenv("USER"), getpid());
				mkfifo(serverfifo, 0600);
				chmod(serverfifo, 0622);
				
				fprintf(clientfp, "%s\n", serverfifo);
				fflush(clientfp);

				FILE *serverfp = fopen(serverfifo, "r");
				char letter = ' ';

				// randomize word 
				srand(time(NULL));
				randomNum = rand() % numWords; 

				// using random word, generate wordString + astString
				generateWordString();
				fprintf(clientfp, "%s\n", wordString);
				fflush(clientfp);
				fprintf(clientfp, "%s\n", astString);
				fflush(clientfp);

				// keep looping until user input stops and game is complete 
				// stop when word is found 
				while (fscanf(serverfp, " %c", &letter) && wordFound == 'n') {
					// check if letter matches or not with the word
					playHangman(letter);

					// send back status of wordFound 
					fprintf(clientfp, "%c\n", wordFound);
					fflush(clientfp);
					
					// word is found, return wordString and missses counter
					// exit the loop, game done
					if (wordFound == 'y') {
						fprintf(clientfp, "%s\n", wordString);
						fflush(clientfp);
						fprintf(clientfp, "%d\n", misses);
						fflush(clientfp);
					}
					// word not found, print updated string, loop again 
					if (wordFound == 'n') {
						fprintf(clientfp, "%s\n", astString);
						fflush(clientfp);
					}

					// send printKey back to client to know what to print
					fprintf(clientfp, "%c\n", printKey);
					fflush(clientfp);
				}
				exit(0);
			}
			// for parent
			else {
				continue; 
			}
		}
		// close the file 
		fclose(fp);
	}
}
