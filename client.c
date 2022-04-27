#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>


//BUFFER SIZE
#define SA struct sockaddr

#define BUFFER_SIZE 2048  


// Status
#define STATUS_ACTIVE 0
#define STATUS_BUSY 2
#define STATUS_INACTIVE 1


int main(int argc, char **argv)
{
    if(argc == 1){
        printf("conexiones chingando");
    }
    if(argc>=4){

        int PORT = atoi(argv[3]);
        char *IP = argv[2];
        char *name = argv[1];
        int sockfd, n;
        int sendbytes;
        struct sockaddr_in servaddr;
        char sendline[BUFFER_SIZE];
        char recvline[BUFFER_SIZE];

        sockfd = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT); //htons=host to network, short
        servaddr.sin_addr.s_addr = inet_addr(IP);

        if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        {
            printf("CONNECTION FAILED! :(");
        }
        
        //SUCCESS CONNECTION, MESSAGE PREPARE
        sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
        sendbytes = strlen(sendline);

        if(write(sockfd, sendline, sendbytes) != sendbytes){
            printf("write error");
        }

        memset(recvline, 0, BUFFER_SIZE);
        exit(0);

        while ((n = read(sockfd, recvline, BUFFER_SIZE-1)) > 0)
        {
            printf("%s", recvline);
        }
        if(n<0){
            printf("read error");
        }

        exit(1);
    }
}