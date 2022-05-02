// standard
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

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
#define BUFFER_SIZE 2048
// create a counter to chek if there is room in the chat
static _Atomic unsigned int client_count = 0;
//ip
#define IP "127.0.0.1"
// Status
#define STATUS_ACTIVE 0
#define STATUS_BUSY 2
#define STATUS_INACTIVE 1
//define a struct for the client info
typedef struct {
    struct sockaddr_in address;
    int sock_fd;
    char name[32];
    int status;
    time_t connect_time;
}Client;

//create pointer array for the clients
Client *CLIENT_ar[MAX_CLIENTS];

//print ip for users
void print_ip(struct sockaddr_in addr){
    printf("%d.%d.%d.%d\n",
    addr.sin_addr.s_addr & 0xff,
    (addr.sin_addr.s_addr & 0xff00) >> 8,
    (addr.sin_addr.s_addr & 0xff0000) >> 16,
    (addr.sin_addr.s_addr & 0xff000000) >> 24);
}
//function that add client to the server clients array
void add_to_queue(Client *cliente){
    
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is empty
        if(!CLIENT_ar[i]){
            CLIENT_ar[i]=cliente;
            break;
        }

    }
}
//function that delete client of the server clients array
void remove_of_queue(Client *cliente){
    
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            if(CLIENT_ar[i]->name == cliente->name){
                //replace the client to null
                CLIENT_ar[i]=NULL;
                break;
            }
        }

    }
}

//send msg to all clients in queue except the sender
void send_msg(char *msg,Client *cliente){
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            //compare if the name is not equal to the sender
            if(CLIENT_ar[i]->name != cliente->name){
                //send msg to the other client
                if(write(CLIENT_ar[i]->sock_fd,msg,strlen(msg))<0){
                    printf("[SERVER]: send msg to client failed..\n"); 
                    break;
                }
            }
        }

    }
}
//send msg to a specific client
void send_msg_client(char *msg,Client *cliente){
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            if(CLIENT_ar[i]->name != cliente->name){
                if(write(CLIENT_ar[i]->sock_fd,msg,strlen(msg))<0){
                    printf("[SERVER]: send msg to client failed..\n"); 
                    break;
                }
            }

        }
    }
}

int main(int argc,char* argv[]){
    if(argc==1)
        printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
    if(argc>=2){
        //format port 
        int PORT = atoi(argv[1]);

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
        //serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        serv_addr.sin_port = htons(PORT);

        //bind the socket
        if ((bind(listen_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) != 0){ 
            fprintf(stderr, "[SERVER-error]: socket bind failed. %d: %s \n", errno, strerror( errno ));
            return -1;
        }
        else{
            printf("[SERVER]: Socket successfully binded \n");
        }
        //listen
        if ((listen(listen_fd, MAX_CLIENTS)) != 0){ 
            fprintf(stderr, "[SERVER-error]: socket listen failed. %d: %s \n", errno, strerror( errno ));
            return -1;
        }
        else{
            printf("[SERVER]: Listening on SERV_PORT %d \n\n", ntohs(serv_addr.sin_port) ); 
        }
        // get len of socket for client
        int len_client = sizeof(client_addr);
        //infinite loop for the accept
        while(1){
            connfd = accept(listen_fd,(struct sockaddr *)&client_addr, &len_client );
            if (connfd < 0) {
                fprintf(stderr, "[SERVER-error]: connection not accepted. %d: %s \n", errno, strerror( errno ));
                return -1;
            }
            else{
                char name[32];
                while(1){
                    //recv(connfd, name, 32, 0);
                    if(recv(connfd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
		                printf("Didn't enter the name.\n");
                    }else{
                        //check if reach max clients in queque
                        if((client_count + 1) == MAX_CLIENTS){
                            printf("Number of Clients reached: \n");
                            close(connfd);
                            continue;

                        }
                        //create a new client and configure it
                        Client *cliente = (Client *)malloc(sizeof(Client));
                        cliente->address = client_addr;
                        cliente->sock_fd = connfd;
                        cliente->status = STATUS_ACTIVE;
                        cliente->connect_time = time(NULL);
                        
                        //add client to the queue
                        add_to_queue(cliente);

                        //reduce cpu usage and imporove perfom 
                        sleep(1);
                    }
                    break;
                }

            }
        }
        
    }
    return 0;
}