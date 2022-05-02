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
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>

//BUFFER SIZE
#define SA struct sockaddr

#define BUFFER_SIZE 2048  
volatile sig_atomic_t flag = 0;

// Status
#define STATUS_ACTIVE 0
#define STATUS_BUSY 2
#define STATUS_INACTIVE 1

void str_overwrite_stdout(){
    printf("%s", "> ");
    fflush(stdout);
}

void str_trim_lf(char* arr, int length){
    int i;
    for(i=0; i<length; i++){
        if(arr[i] == '\n'){
            arr[i] = '\0';
            break;
        }
    }
}

void catch_commands(int sig){
    flag = 1;
}

void send_message(){
    char message[BUFFER_SIZE] = {};
    char buffer[BUFFER_SIZE + 32] = {};

    while (1){
        str_overwrite_stdout();
        fgets(message, BUFFER_SIZE, stdin);
        str_trim_lf(message, BUFFER_SIZE);
    

        if(strcmp(message, "exit")==0){
            break;
        }else{
            sprintf(buffer, "%s: %s\n", name, message);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(message, BUFFER_SIZE);
        bzero(buffer, BUFFER_SIZE + 32);
    }
    catch_commands(2);
    
}

int main(int argc, char **argv)
{
    if(argc == 1){
        printf("conexiones chingando");
    }
    if(argc>=4){

        int PORT = atoi(argv[3]);
        char *IP = (argv[2]);
        
        int sockfd, n;
        int sendbytes;
        struct sockaddr_in servaddr;
        

        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1){ 
            fprintf(stderr, "[SERVER-error]: socket creation failed. %d: %s \n", errno, strerror( errno ));
            return -1;
        }
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(PORT); //htons=host to network, short
        servaddr.sin_addr.s_addr = inet_addr(IP);

        if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
        {
            printf("CONNECTION FAILED! :(");
            return -1;
        }
        printf("----welcome to the bate papo----\n");
        send(sockfd, name, 32, 0);
        
        /* close the socket */
        close(sockfd); 
    }
}