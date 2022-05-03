// standard
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

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

//create thread iniciator
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void str_trim_lf(char* arr, int length){
    int i;
    for(i=0; i<length; i++){
        if(arr[i] == '\n'){
            arr[i] = '\0';
            break;
        }
    }
}

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
    pthread_mutex_lock(&clients_mutex);
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is empty
        if(!CLIENT_ar[i]){
            CLIENT_ar[i]=cliente;
            break;
        }

    }
    pthread_mutex_unlock(&clients_mutex);
}
//function that delete client of the server clients array
void remove_of_queue(Client *cliente){
    pthread_mutex_lock(&clients_mutex);
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,cliente->name)==0){
                //replace the client to null
                CLIENT_ar[i]=NULL;
                break;
            }
        }

    }
    pthread_mutex_unlock(&clients_mutex);
}

//send msg to all clients in queue except the sender
void send_msg(char *msg,Client *cliente){
    pthread_mutex_lock(&clients_mutex);
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            //compare if the name is not equal to the sender
            if(strcmp(CLIENT_ar[i]->name,cliente->name)!=0){
                //send msg to the other client
                if(write(CLIENT_ar[i]->sock_fd,msg,strlen(msg))<0){
                    printf("[SERVER]: send msg to client failed..\n"); 
                    break;
                }
            }
        }

    }
    pthread_mutex_unlock(&clients_mutex);
}
//send msg to a specific client
void send_msg_client(char *msg,Client *cliente){
    pthread_mutex_lock(&clients_mutex);
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,cliente->name)!=0){
                if(write(CLIENT_ar[i]->sock_fd,msg,strlen(msg))<0){
                    printf("[SERVER]: send msg to client failed..\n"); 
                    break;
                }
            }

        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
//validate if the name exists return true if there is a cliente with this name or false if there is not
bool val_username(Client *cliente){

    for (int i=0; i < MAX_CLIENTS; ++i){
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,cliente->name)==0){
                return true;
            }
        }
    }
    return false;
}
//validate if the user exits in the array
bool is_in_users(Client *cliente){
    for (int i=0; i < MAX_CLIENTS; ++i){
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,cliente->name)==0){
                return true;
            }
        }
    }
    return false;
}
//function to kick out a user
void kick_user(char*msg,Client *cliente){
    pthread_mutex_lock(&clients_mutex);
    for (int i=0; i < MAX_CLIENTS; ++i){
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,cliente->name)==0){
                if(write(CLIENT_ar[i]-> sock_fd, msg, strlen(msg)) < 0){
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
void *handle_chat(void *arg){
    char buffer_out[BUFFER_SIZE];
    char buffer_out_copy[BUFFER_SIZE];
    char name[32];

    client_count++;

    Client *cliente = (Client*)arg;

    int leave_flag = 0;

    //get name from the client
    if(recv(cliente->sock_fd, name, 32, 0) <= 0 || strlen(name) <  2 || strlen(name) >= 32-1){
        printf("Didn't enter the name.\n");
        leave_flag = 1;
    }else{
        strcpy(cliente->name, name);
        bool user_name_exists = val_username(cliente);

        sprintf(buffer_out, "%s has joined\n", cliente->name);
        printf("%s", buffer_out);
        printf("%s\n",user_name_exists);
        if(!user_name_exists){//the username not exist in the array
            sprintf(buffer_out, "%s has joined\n", cliente->name);
            printf("%s", buffer_out);
        }
        else{//user exits
            sprintf(buffer_out, "Username (%s) already exists.\n", cliente->name);
            kick_user(buffer_out, cliente);
            leave_flag = 1;

        }
        //printf("%s\n",cliente->name);
        
    }
    //clear the buffer
    bzero(buffer_out, BUFFER_SIZE);

    //principal loop
    while(1){
        //chek if the user gone
        //if(leave_flag){ break;}
        printf("leave_flag: %d\n",leave_flag);
        //get info
        int receive = recv(cliente->sock_fd, buffer_out, BUFFER_SIZE, 0);
        printf("recive: %d\n",receive);
        if(receive > 0){
             //copu buffer
            strcpy(buffer_out_copy, buffer_out);
            printf("--------\n");
            if (strlen(buffer_out) > 0){
                str_trim_lf(buffer_out, strlen(buffer_out));
                printf("%s -> %s\n", buffer_out, cliente->name, cliente->status);

            }
            else{
                printf("no msg\n");
            }
           

            //send msg to other users
            //send_msg(buffer_out,cliente);


        }else if(receive==0){
            sprintf(buffer_out, "%s has left\n", cliente->name);
            printf("%s\n", buffer_out);
            send_msg(buffer_out, cliente);

        }else{
            printf("ERROR: -1\n");
        }
        bzero(buffer_out, BUFFER_SIZE);
    }

}
int main(int argc,char* argv[]){
    if(argc==1)
        printf("\nNo Extra Command Line Argument Passed Other Than Program Name");
    if(argc>=2){
        //format port 
        int PORT = atoi(argv[1]);
        //create pid 
        pthread_t tid;
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

                    pthread_create(&tid, NULL, &handle_chat, (void*)cliente);

                    //reduce cpu usage and imporove perfom 
                    sleep(1);
                    
                    break;
                }

            }
        }
        
    }
    return 0;
}