#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>

#define PORT "33242"  
#define PLAYERCOUNT 2
#define MAXLEN 100   

// fucntions 
void handleSignal(int);
int handleConnection(int sockfd, char *buff);
int myrecv(int sockfd, void *buff, int size, char *err_msg);
int mysend(int sockfd, char *msg, int len, char *err_msg);
void playGame(int *players_fd, char players_names[PLAYERCOUNT][MAXLEN]);

// global variable
int server_sockfd;
int i, j;

int main(void)
{
        signal(SIGINT, handleSignal);
		
		// variables 
        int player_sockfd[PLAYERCOUNT];
        char player_name[PLAYERCOUNT][MAXLEN];
        struct addrinfo addr, *serv_info;

        memset(&addr, 0, sizeof(addr));
        addr.ai_family = AF_INET;
        addr.ai_socktype = SOCK_STREAM;
        addr.ai_flags = AI_PASSIVE;

        int rv;
        if ((rv = getaddrinfo(NULL, PORT, &addr, &serv_info)) != 0) {
                fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
                return 1;
        }

        while (1) { 
                if ((server_sockfd = socket(serv_info->ai_family, serv_info->ai_socktype, serv_info->ai_protocol)) == -1) {
                        close(server_sockfd);
                        perror("server: socket");
                        exit(1);
                }

                if (bind(server_sockfd, serv_info->ai_addr, serv_info->ai_addrlen) == -1) {
                        close(server_sockfd);
                        perror("server: bind");
                        exit(1);
                }

                if (listen(server_sockfd, PLAYERCOUNT) == -1) {
                        close(server_sockfd);
                        perror("server: listen");
                        exit(1);
                }

                printf("\n\nStart Game \n", PLAYERCOUNT);
                for (i = 0; i < PLAYERCOUNT; i++) {
                        player_sockfd[i] = handleConnection(server_sockfd, player_name[i]);
                }
                close(server_sockfd);
                playGame(&player_sockfd[0], player_name);

                for (i = 0; i < PLAYERCOUNT; i++) 
                        close(player_sockfd[0]);
        }

        return 0;
}

int handleConnection(int sockfd, char *buff){
        struct sockaddr_storage p_addr; 
        socklen_t sin_size = sizeof(p_addr);
        int new_fd;

        new_fd = accept(sockfd, (struct sockaddr *) &p_addr, &sin_size);
        if (new_fd == -1) {
                perror("accept");
        }

        int numbytes;
        printf("Connection accepted\n");

        numbytes = myrecv(new_fd, buff, MAXLEN - 1, "player name");

        buff[numbytes] = '\0';
        printf("%s logged in\n", buff);
        return new_fd;
}

void playGame(int *players_fd, char players_names[PLAYERCOUNT][MAXLEN]){
        printf("All players have joined, the game will start now\n");
        srand(time(NULL));

        int randomNum = (rand() % 99) + 1;
		int guessNum = 0, convertNum, i;
        for (i = 0; i < PLAYERCOUNT; i++)
                mysend(players_fd[i], "S", 1, "S send"); 

        sleep(1);
        int round = 1;

        while (1) {
                printf("\n");
                for (i = 0; i < PLAYERCOUNT; i++) {
                        printf("Round %d, %s's turn\n", round, players_names[i]);
                        mysend(players_fd[i], "R", 1, "R send"); 
                        myrecv(players_fd[i], &guessNum, sizeof(int), "player guessed number");
                        convertNum = ntohl(guessNum);
                        printf("%s guessed %d\n", players_names[i], convertNum);

                        if (convertNum == randomNum) {
                                printf("%s won the game\n", players_names[i]);
								for (j = 0; j < PLAYERCOUNT; j++) {
                                        if (i == j)
                                                mysend(players_fd[j], "=", 1, "= send");
                                        else
                                                mysend(players_fd[j], "-", 1, "- send");
                                }
                                return;
                        } 
						else if (convertNum > randomNum) {
                                mysend(players_fd[i], ">", 1, " > send");
                        } 
						else if (convertNum < randomNum) {
                                mysend(players_fd[i], "<", 1, " < send");
                        } 
						else {
                                printf("error at %d\n", __LINE__);
                                exit(1);
                        }
                }
                round++;
        }
}

int mysend(int sockfd, char *msg, int len, char *err_msg) {
        int bytes_send = 0;
        if ((bytes_send = (int) send(sockfd, msg, (size_t) len, 0)) == -1) {
                perror("send");
                printf("%s at %d\n", err_msg, __LINE__);
                exit(0);
        }
        return bytes_send;
}

int myrecv(int sockfd, void *buff, int len, char *err_msg) {
        int numbytes;
        if ((numbytes = (int) recv(sockfd, buff, (size_t) len, 0)) == -1) {
                perror("recv");
                printf("%s at %d\n", err_msg, __LINE__);
                exit(1);
        }
        return numbytes;
}

void handleSignal(int sig) {
        printf("    closing server socket: %d\n", server_sockfd);
        close(server_sockfd);
        exit(0);
}
