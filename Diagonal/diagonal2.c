#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define LINESIZE 16

// use one command line argument
int main(int argc, char *argv[]) {
	// incorrect user input format
	if (argc < 2) {
		printf("Usage: diagonal <textstring>\n");
		return -1;
	}
	
	// create a file so that 16 rows of empty will appear with od -c command
	int fd = open("diagonal2.out", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	
	// variables
	int line, column, i, word;
	int start = 0, end = (LINESIZE * LINESIZE) - 1;
	char space = ' ';

	// create a block of 16 x 16 characters for each word 
	for (word = 0; word < argc - 1; word++)
		for (line = 0; line < LINESIZE; line++)
			for (column = 0; column < LINESIZE; column++)
				write(fd, &space, 1);

	// each line of od outputs 16 characters 
	// make the output diagonal, we will use 0, 17, 34, ....
	// reverse for loop for the other direction and do for each word 

	// loop each word
	for (word = 0; word < argc - 1; word++) {
		// find length of the word 
		int length = strlen(argv[word+1]);
		
		// when its the first, third, fifth word...
		// print left to right
		if (word % 2 == 0) {
			// loop over every character of word
			for(i = 0; i < length; i++) {
				lseek(fd, start + (17 * i), SEEK_SET);
				write(fd, &argv[word + 1][i], 1);
			}	
		}
		// second, fourth, sixth word...
		// print right to left now (reverse order)
		else if (word % 2 != 0) {
			for(i = 0; i < length; i++) { 
				lseek(fd, (start + 15) + (15 * i), SEEK_SET);
				write(fd, &argv[word + 1][i], 1);
			}
		}

		// set a new 16 x 16 block for next word
		start = end + 1;
		end = (LINESIZE * LINESIZE) * (word + 2) - 1;
}

	// close file and print statement 
	close(fd);
	puts("diagonal.out has been created. Use od -c diagonal.out to see the contents.");
}
