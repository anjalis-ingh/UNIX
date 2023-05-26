#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>

#define PORT "33242" 
#define MAXLEN 100  
#define SERVERIP "127.0.0.3"

int myrecv(int sockfd, void *buff, int size, char *err_msg);
int mysend(int sockfd, void *msg, int len, char *err_msg);

int main(int argc, char *argv[])
{
        int Csockfd;
        char receive = '0';
        char name[MAXLEN];
        struct addrinfo addr, *serv_info;

        memset(&addr, 0, sizeof addr);
        addr.ai_family = AF_INET;
        addr.ai_socktype = SOCK_STREAM;

        int rv = getaddrinfo(SERVERIP, PORT, &addr, &serv_info);

        if ((Csockfd = socket(serv_info->ai_family, serv_info->ai_socktype, serv_info->ai_protocol)) == -1) {
				perror("client: socket");
                exit(1);
        }

        if (connect(Csockfd, serv_info->ai_addr, serv_info->ai_addrlen) == -1) {
                perror("client: connect");
                close(Csockfd);
                exit(1);
        }

        printf("The Number Guessing Game\n");
        printf("Player name: ");
        scanf("%s", name);

        mysend(Csockfd, name, (int) strlen(name), "player name");

        myrecv(Csockfd, &receive, 1, "get start"); 
        if (receive != 'S') {
                printf("Error occur at Start message");
                exit(1);
        }

        printf("\nStart Game\n");
        int guessNum, convertNum;
        while (1) {
                myrecv(Csockfd, &receive, 1, "ROUND");

                if (receive == '-') {
                        printf("The game is over\n");
                        close(Csockfd);
                        exit(0);
                } 
				else if (receive != 'R')
                        continue;

                printf("\nIt's your turn\n");
                printf("Guess a number: ");
                scanf("%d", &guessNum);

                convertNum = htonl(guessNum); 
                mysend(Csockfd, &convertNum, sizeof(uint32_t), "guessed number");

                myrecv(Csockfd, &receive, 1, "return value");
                switch (receive) {
                        case '=':
                                printf("Your answer is correct\n");
                                printf("The game is over\n");
                                sleep(2);
                                close(Csockfd);
                                exit(0);
                        case '<':
                                printf("Go higher!\n");
                                break;
                        case '>':
                                printf("Go lower!\n");
                                break;

                        case '-':
                                printf("The game is over\n");
                                close(Csockfd);
                                exit(0);

                        default:
                                printf("error in switch case: %c\n", receive);
                                close(Csockfd);
                                exit(1);
                }

        }

        return 0;
}

int mysend(int sockfd, void *msg, int len, char *err_msg) {
        int bytes_send;
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
