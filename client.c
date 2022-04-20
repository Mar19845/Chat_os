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

//PUERTO HTTP
#define SERVER_PORT 4565

//BUFFER SIZE
#define MAXLINE 4096
#define SA struct sockaddr

void handle_error(const char *fmt, ...);


int main(int argc, char **argv)
{
    int sockfd, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    if (argc != 2){ //USAGE CHECK
        handle_error("usage: %s <server addres>", argv[0]);
    }

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
        handle_error("Error while creating the socket! :(");
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT); //htons=host to network, short

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
        handle_error("inet_pton error for %s", argv[1]);
    }

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
    {
        handle_error("CONNECTION FAILED! :(");
    }
    
    //SUCCESS CONNECTION, MESSAGE PREPARE
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    sendbytes = strlen(sendline);

    if(write(sockfd, sendline, sendbytes) != sendbytes){
        handle_error("write error");
    }

    memset(recvline, 0, MAXLINE);
    exit(0);

    while ((n = read(sockfd, recvline, MAXLINE-1)) > 0)
    {
        printf("%s", recvline);
    }
    if(n<0){
        handle_error("read error");
    }
    
    
}

void handle_error(const char *fmt, ...)
{
    int errno_save;
    va_list ap;

    errno_save = errno;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    if (errno_save != 0) {
        fprintf(stdout, "(errno = %d) : %s\n",errno_save,
        strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    va_end(ap);

    exit(1);
}