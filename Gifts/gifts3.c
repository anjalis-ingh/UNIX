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

// array of structs
typedef struct Player {
    char name[MAXLEN];
    double balance; 
} Player_t; 

Player_t players[MAXPLAYERS];
int numPlayers = 0; // set when actual player data is loaded

// search the player names array and return the index if specified name is found
// return -1 otherwise.
int findIndex(char *name) {
    for (int i = 0; i < numPlayers; i++)
        if (strcmp(name, players[i].name) == 0)
            return i;
 
    return -1;
}

// use binary data file gifts2.dat to read and store the results.
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
                int counter = 0;
                FILE *filep = fopen("gifts3.dat", "r"); 
                
                while (fread(&players[counter], sizeof(Player_t), 1, filep) == 1) {
                    counter++;
                }
                
                numPlayers = counter;
                
                for (int i = 0; i < counter; i++) {
                    strcpy(names[i], players[i].name);
                    balances[i] = players[i].balance;
                }
                
                fclose(filep);
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
      
    FILE *filep = fopen("gifts3.dat", "wb"); 
    
    // loop over each player name and balance 
    for (int i = 0; i < numPlayers; i++) {
        strcpy(players[i].name, names[i]);
        players[i].balance = balances[i];
        printf("%10s: %6.2lf\n", names[i], balances[i]);
    }
    
    fwrite(players, sizeof(struct Player), numPlayers, filep);
 
    fclose(filep);
}

