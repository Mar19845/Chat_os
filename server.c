// standard
#include <unistd.h>
#include<stdio.h>

//sockets lib
#include <netdb.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <arpa/inet.h>

// strings  lib
#include <errno.h>
#include <stdio.h> 
#include <string.h>


// global variables
// number of clients the server can handle 
#define MAX_CLIENTS 100
//buffer max size
#define BUFFER_SIZE 100  
//ip
#define IP "192.168.0.21"
int main(int argc,char* argv[]){
    if(argc==1)
        printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
    if(argc>=2){
        printf("\nPort_Serv: %s",argv[1]);
        // define int for comunication
        int listen_fd,connfd; // listen to socket and socket connection descriptors
        // import socket struct for server and client
        struct sockaddr_in serv_addr,client_addr;
        
        //
        int  len_rx, len_tx = 0;                     /* received and sent length, in bytes */
        char buff_tx[BUFFER_SIZE] = "Hello client, I am the server";
        char buff_rx[BUFFER_SIZE];   /* buffers for reception  */

        //create the socket
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1){ 
            fprintf(stderr, "[SERVER-error]: socket creation failed. %d: %s \n", errno, strerror( errno ));
            return -1;
        } 
        else{
            printf("[SERVER]: Socket successfully created..\n"); 
        }

        // clean the pointer of the socket
        memset(&serv_addr, 0, sizeof(serv_addr));
        // configure server socket
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = inet_addr(IP);
        serv_addr.sin_port = htons(argv[1]);

        //bind the socket
        if ((bind(listen_fd, (struct serv_addr *)&serv_addr, sizeof(serv_addr))) != 0){ 
            fprintf(stderr, "[SERVER-error]: socket bind failed. %d: %s \n", errno, strerror( errno ));
            return -1;
        }
        //listen
        if ((listen(listen_fd, MAX_CLIENTS)) != 0){ 
            fprintf(stderr, "[SERVER-error]: socket listen failed. %d: %s \n", errno, strerror( errno ));
            return -1;
        }
        // get len of socket for client
        int len_client = sizeof(client_addr);
        //infinite loop for the accept
        while(1){
            connfd = accept(listen_fd,(struct serv_addr *)&client_addr, &len_client );
            if (connfd < 0) {
                fprintf(stderr, "[SERVER-error]: connection not accepted. %d: %s \n", errno, strerror( errno ));
                return -1;
            }
            else{
                while(1){
                    //read client message
                    len_rx = read(connfd, buff_rx, sizeof(buff_rx));
                    if(len_rx == -1){
                        fprintf(stderr, "[SERVER-error]: connfd cannot be read. %d: %s \n", errno, strerror( errno ));
                    }
                    else if(len_rx == 0) /* if length is 0 client socket closed, then exit */{
                        printf("[SERVER]: client socket closed \n\n");
                        close(connfd);
                        break; 
                    }
                    else{
                        write(connfd, buff_tx, strlen(buff_tx));
                        printf("[SERVER]: %s \n", buff_rx);
                    }
                }

            }
        }
        
    }
    return 0;
}