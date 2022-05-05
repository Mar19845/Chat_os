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
#include <json-c/json.h>

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
#define STATUS_ACTIVE "0"
#define STATUS_BUSY "2"
#define STATUS_INACTIVE "1"

//id for user
static int id = 10;


//define a struct for the client info
typedef struct {
    struct sockaddr_in address;
    int sock_fd;
    char name[32];
    int id;
    char status[32];
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

    char *instruccion;

    //create json and send to server
    struct json_object *init_connection = json_object_new_object();
    //add request to json
    json_object_object_add(init_connection,"request",json_object_new_string("NEW_MESSAGE"));
    //create body
    struct json_object *body = json_object_new_array();
        
    
    json_object_array_add(body,json_object_new_string(msg));
    json_object_array_add(body,json_object_new_string(cliente->name));
    //deliver time
    json_object_array_add(body,json_object_new_string(msg));
    json_object_array_add(body,json_object_new_string("all"));

    // add body to init conection
    json_object_object_add(init_connection,"body",body);

    //convert json to string
    instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
    // clear the init connection

    
    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            //compare if the name is not equal to the sender
            
            if(CLIENT_ar[i]->id != cliente->id){
                if(send(CLIENT_ar[i]->sock_fd, instruccion, BUFFER_SIZE, 0)<0){
                    printf("[SERVER]: send msg to client failed..\n"); 
                    break;
                }
            }
            
            
        }

    }
    
    pthread_mutex_unlock(&clients_mutex);
}
//send msg to a specific client
void send_msg_client(char *msg,char *name,Client *cliente){

    pthread_mutex_lock(&clients_mutex);

    char *instruccion;

    //create json and send to server
    struct json_object *init_connection = json_object_new_object();
    //add request to json
    json_object_object_add(init_connection,"request",json_object_new_string("NEW_MESSAGE"));
    //create body
    struct json_object *body = json_object_new_array();
        
    
    json_object_array_add(body,json_object_new_string(msg));
    json_object_array_add(body,json_object_new_string(cliente->name));
    //deliver time
    json_object_array_add(body,json_object_new_string(msg));
    json_object_array_add(body,json_object_new_string(name));

    // add body to init conection
    json_object_object_add(init_connection,"body",body);

    //convert json to string
    instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
    // clear the init connection



    //for loop to iter the clients in the array
    for (int i=0; i < MAX_CLIENTS; ++i){
        //check if this position is not empty
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,name)==0){
                if(send(CLIENT_ar[i]->sock_fd, instruccion, BUFFER_SIZE, 0)<0){
                    printf("[SERVER]: send msg to client failed..\n"); 
                    break;

                }
                //if(write(CLIENT_ar[i]->sock_fd,msg,strlen(msg))<0){
                    //printf("[SERVER]: send msg to client failed..\n"); 
                    //break;
                //}
            }

        }
    }
    pthread_mutex_unlock(&clients_mutex);
}
//validate if the name exists return true if there is a cliente with this name or false if there is not
int val_username(Client *cliente){

    for (int i=0; i < MAX_CLIENTS; ++i){
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,cliente->name)==0){
                //check if the id is the same for the users
                if(CLIENT_ar[i]->id < cliente->id){
                    return 1;
                }
            }
        }
    }
    return 0;
}
//validate if the user exits in the array
int is_in_users(char *name){
    for (int i=0; i < MAX_CLIENTS; ++i){
        if(CLIENT_ar[i]){
            if(strcmp(CLIENT_ar[i]->name,name)==0){
                return 1;
                
            }
        }
    }
    return 0;
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
    char *connect_time;
    char *name;
    char *json_instruccion;
    char *response;
    char *opcion;
    char *code;
    //int code;

    client_count++;

    Client *cliente = (Client*)arg;

    int leave_flag = 0;
    //get name from the client

    //init conex json request
    
    int receive = recv(cliente->sock_fd, buffer_out, BUFFER_SIZE, 0);
    if(receive > 0){
        
        //convert the buffer out from json to strings
        //copy buffer out to json_instruccion
        json_instruccion = buffer_out;
        //create json objet
        struct json_object *instruccion = json_object_new_object();
        //parser the json
        instruccion = json_tokener_parse(json_instruccion);

        //get request
        struct json_object *request;
        json_object_object_get_ex(instruccion,"request",&request);

        //get body
        struct json_object *body;
        json_object_object_get_ex(instruccion,"body",&body);

        opcion = json_object_get_string(request);
        
        if(strcmp(opcion,"INIT_CONEX") == 0){
            //get index 0 of array
            //connect_time = json_object_get_string(json_object_array_get_idx(body,0));
            //strcpy(cliente->connect_time, connect_time);
          
            //get index 1 of array
            name = json_object_get_string(json_object_array_get_idx(body,1));
            strcpy(cliente->name, name);
            
            //check if the username is valid
            int user_name_exists = val_username(cliente);
            if(user_name_exists == 0){//the username not exist in the array
                sprintf(buffer_out, "%s has joined\n", cliente->name);
                printf("%s", buffer_out);
                code = "200";
            }
            else{//user exits
                sprintf(buffer_out, "Username (%s) already exists.\n", cliente->name);
                //kick_user(buffer_out, cliente);
                leave_flag = 1;
                printf("user exist: %d\n",leave_flag);
                code = "101";

            }
        
            //respond to the client
            struct json_object *response_form = json_object_new_object();
            json_object_object_add(response_form,"response",json_object_new_string("INIT_CONEX"));
            json_object_object_add(response_form,"code",json_object_new_string(code));

            //convert to json
            response = json_object_to_json_string_ext(response_form,JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            //send response to client
            send(cliente->sock_fd,response,strlen(response),0);
            

        }

    }else{
        leave_flag = 1;
    }


     
    //clear the buffer
    bzero(buffer_out, BUFFER_SIZE);
    //principal loop
    while(1){

        //chek if the user gone
        if(leave_flag==1){ 
            break;
        }
    
        //get info
        int receive = recv(cliente->sock_fd, buffer_out, BUFFER_SIZE, 0);
        //add json parser y demas
        if(receive > 0){
            //convert the buffer out from json to strings
            //copy buffer out to json_instruccion
            json_instruccion = buffer_out;
            //create json objet
            struct json_object *instruccion = json_object_new_object();
            //parser the json
            instruccion = json_tokener_parse(json_instruccion);

            //get request
            struct json_object *request;
            json_object_object_get_ex(instruccion,"request",&request);

            //get body
            struct json_object *body;
            json_object_object_get_ex(instruccion,"body",&body);

            opcion = json_object_get_string(request);
            //end connection
            if(strcmp(opcion,"END_CONEX") == 0){
                sprintf(buffer_out, "%s has left\n", cliente->name);
                printf("%s\n", buffer_out);
                //send_msg(buffer_out, cliente);
                leave_flag = 1;
                code = "200";

            }
            else if(strcmp(opcion,"PUT_STATUS") == 0){
                //get the new status
                char *new_status;
                new_status=json_object_get_string(body);
                if(strcmp(new_status,"0")==0){
                    //set status to active
                    strcpy(cliente->status, "0");
                    code = "200";
                    printf("%s->%s\n",cliente->name,cliente->status);
                }
                else if(strcmp(new_status,"1")==0){
                    //set status to inactive
                    strcpy(cliente->status, "1");
                    code = "200";
                    printf("%s->%s\n",cliente->name,cliente->status);

                }
                else if(strcmp(new_status,"2")==0){
                    //set status to busy
                    strcpy(cliente->status, "2");
                    code = "200";
                    printf("%s->%s\n",cliente->name,cliente->status);

                }
                else{
                    code = "104";

                }

            }
            else if(strcmp(opcion,"POST_CHAT") == 0){

                char *msg;
                char *user_sender;
                char *time_sent;
                char *to_who;
                //get values from the body array
                msg = json_object_get_string(json_object_array_get_idx(body,0));
                user_sender = json_object_get_string(json_object_array_get_idx(body,1));
                time_sent = json_object_get_string(json_object_array_get_idx(body,2));
                to_who = json_object_get_string(json_object_array_get_idx(body,3));
                
                
                if(strcmp(to_who,"all") == 0){
                    //send msg to all users
                    //printf("%s->%s\n",user_sender,msg);
                    send_msg(msg,cliente);
                    //add funciton to send msg to other users
                    code = "200";
                    
                }
                else{
                    int valid_user = is_in_users(to_who);
                    if(valid_user==1){
                        //printf("%s->sent msg to: %s ->%s\n",user_sender,to_who,msg);
                        send_msg_client(msg,to_who,cliente);
                        //add funciton to send msg to user
                        code = "200";
                    }
                    else{
                        code = "102";
                    }
                }
                //printf("%s->sent msg to: %s ->%s\n",user_sender,to_who,msg);
            }
            else if (strcmp(opcion,"GET_USER") == 0){
                printf("hola\n");
                char *info_of_user;
                info_of_user=json_object_get_string(body);

                printf("%s\n",info_of_user);

                if(strcmp(info_of_user,"all") == 0){
                    printf("%s\n",info_of_user);
                    printf("asds\n");

                }
                else{
                    int valid_user = is_in_users(info_of_user);
                    if(valid_user==1){
                        printf("%s->sent msg to: %s ->%s\n",info_of_user,info_of_user,info_of_user);
                        print_ip(cliente->address);

                    }

                }
            }
            else if (strcmp(opcion,"GET_CHAT") == 0){
                continue;
            }
            // send response to client
            //respond to the client
            struct json_object *response_form = json_object_new_object();
            json_object_object_add(response_form,"response",json_object_new_string(opcion));
            json_object_object_add(response_form,"code",json_object_new_string(code));

            //convert to json
            response = json_object_to_json_string_ext(response_form,JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            //send response to client
            send(cliente->sock_fd,response,strlen(response),0);

            bzero(buffer_out, BUFFER_SIZE);



        }
        //if receive == 0, user left
        else if(receive==0){
            sprintf(buffer_out, "%s has left\n", cliente->name);
            printf("%s\n", buffer_out);
            //send_msg(buffer_out, cliente);
            leave_flag = 1;

        }
        else{
            printf("ERROR: -1\n");
           //leave_flag = 1;
        }
        bzero(buffer_out, BUFFER_SIZE);
    }

    //close sockt
    close(cliente->sock_fd);
    remove_of_queue(cliente);
    free(cliente);

    // Decrease Client count
    client_count--;

    pthread_detach(pthread_self());

    return NULL;

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
        int opt = 1;
    

        //create the socket
        listen_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == -1){ 
            fprintf(stderr, "[SERVER-error]: socket creation failed. %d: %s \n", errno, strerror( errno ));
            return -1;
        } 
        else{
            printf("[SERVER]: Socket successfully created..\n"); 
        }

        //force the socket attaching to the port/ip addres
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt)) < 0){
            perror("setsockopt(SO_REUSEADDR) failed");
        }
        #ifdef SO_REUSEPORT
        if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEPORT, (const char*)&opt, sizeof(opt)) < 0){
            perror("setsockopt(SO_REUSEPORT) failed");
        }
        #endif
        

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
                    strcpy(cliente->status, "0");
                    cliente->id = id++;
  
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