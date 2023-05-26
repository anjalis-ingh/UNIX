#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// let us assume that there will not be more than 100 players
#define MAXPLAYERS 100
// let us assume that max length for any name is 100 characters
#define MAXLEN 100

// arrays to store the player names and balances
char names[MAXPLAYERS][MAXLEN];
double balances[MAXPLAYERS];
int numPlayers = 0; // set when actual player data is loaded

// search the player names array and return the index if specified name is found
// return -1 otherwise.
int findIndex(char *name) {
    for (int i = 0; i < numPlayers; i++)
        if (strcmp(name, names[i]) == 0)
            return i;
 
    return -1;
}

void removeSpaces(char* s) {
    char* d = s;
    do {
        while (*d == ' ') {
            ++d;
        }
    } while (*s++ = *d++);
}

void readFile () {
     int counter = 0; 
     char line[100];
      
     FILE *filep = fopen("gifts1.txt", "r");
     
     // loop over each line (colon = delimiter)
     // remove whitespaces and store inputs into names and balances arrays 
     while (fgets(line, 100, filep) != NULL)
     {
        char *p = strtok(line, ":");
        
        while(p)
        {
            removeSpaces(p);
            // even # = name, odd # = balance
            if (counter % 2 == 0) {
                strcpy(names[numPlayers], p);
            }
            else {
                balances[numPlayers] = atof(p);
                numPlayers++;
            }
            p = strtok(NULL, ":");
            counter++;
        }
     }
     
     fclose(filep);    
}

int main(int argc, char *argv[]) {
    // variables 
    int newAccess = 0, maxLength = 0;
    int length, index, splitCount;
    double balance, money; 
    
    // loop over command line input
    for (int i = 0; i < argc; i++) {
        // clear the names and balances array and reset numPlayers
        if ((strcmp(argv[1], "new") == 0) && (i < 2)) {
            numPlayers = 0;
            newAccess = 1;
        }
        // set new names and balances for each player 
        // even # = name, odd # = balance 
        else if ((newAccess == 1) && (i >= 2)) {
            if (i % 2 == 0) {
                strcpy(names[numPlayers], argv[i]);
            }
            else {
                balances[numPlayers] = atof(argv[i]);
		        numPlayers++;
            }
        }
        // perform transaction and split the bill
        else if (newAccess == 0) {
            // read the file and store data into names and balances arrays 
            if (i == 0) {
                readFile();
            }
            // find the index of the name  
            if (i == 1) {
                index = findIndex(argv[i]);
            }
            // note how much money will be given to players and subtract from player's balance 
            if (i == 2) {
                money = atof(argv[i]);
                balances[index] -= money;
            }
            // find the other players and split the bill evenly between them
            if (i >= 3) {
                // how many people the money should be split to 
                splitCount = argc - 3;
                
                index = findIndex(argv[i]);
                balances[index] += (money / splitCount);
            }
        }
        
    }
  
    FILE *filep = fopen("gifts1.txt", "w");
    
    // loop over each player name and balance 
    for (int i = 0; i < numPlayers; i++) {
        fprintf(filep, "%10s: %6.2lf\n", names[i], balances[i]);
        printf("%10s: %6.2lf\n", names[i], balances[i]);
    }   
 
    fclose(filep);
}


