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
#include <json-c/json.h>

//BUFFER SIZE
#define SA struct sockaddr

#define BUFFER_SIZE 2048  
volatile sig_atomic_t flag = 0;

// Status
char status[32];
char *name;
int sockfd = 0;
#define STATUS_ACTIVE "0"
#define STATUS_BUSY "2"
#define STATUS_INACTIVE "1"

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
    char message_copy[BUFFER_SIZE] = {};//copy of the message for
    char buffer[BUFFER_SIZE + 32] = {};
    char *key, *value;
    //char *instruccion;

    char *token;
    //const char *delim = " ";

    while (1){
         

        str_overwrite_stdout();
        fgets(message, BUFFER_SIZE, stdin);
        
        str_trim_lf(message, BUFFER_SIZE);
        
        strcpy(message_copy, message);
        
        // char *key, *value;
        token = strtok(message, " ");
        char *instruccion;
         
        

        //this is need to be done depends on what the token is

        //key = actual msg or usernme depends on what the user wants
        //key = strtok(NULL, " ");

        //value = null or msg depends on what the user wants
        //value = strtok(NULL, " ");

        //create token for the json
        if (strcmp(token, "exit") == 0){
            // Exit chat
            //end_conex 
            //create json and send to server

            //create json and send to server
            struct json_object *init_connection = json_object_new_object();
            //add request to json
            json_object_object_add(init_connection,"request",json_object_new_string("END_CONEX"));

            //convert json to string
            instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            // clear the init connection

            send(sockfd, instruccion, BUFFER_SIZE, 0);


            flag = 1;
            break;

        }
        else if (strcmp(token, "change_status")==0){
            //put_status
            key = strtok(NULL, " ");
            //key = status 
            //create json for change status and send to server
            struct json_object *init_connection = json_object_new_object();
            //add request to json
            json_object_object_add(init_connection,"request",json_object_new_string("PUT_STATUS"));
            json_object_object_add(init_connection,"body",json_object_new_string(key));

            //convert json to string
            instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            //printf("%s\n",key);
            //send(sockfd, instruccion, BUFFER_SIZE, 0);

        }
        else if (strcmp(token, "send_to")==0){

            //post chat
            // post chat to given user
            key = strtok(NULL, " ");
            //key = username 
            value = strtok(NULL, " ");
            //value = msg

            // printf("yo-> %s\n",message_copy);
            // printf("token: %s\n",token);
            // printf("key: %s\n",key);
            // printf("val: %s\n",value);

            //create json for change status and send to server
            struct json_object *init_connection = json_object_new_object();
            //add request to json
            json_object_object_add(init_connection,"request",json_object_new_string("POST_CHAT"));
            //create json for send msg to certain user and send to server

            //create body
            struct json_object *body = json_object_new_array();

            //add to the array
            json_object_array_add(body,json_object_new_string(value));
            json_object_array_add(body,json_object_new_string(name));
            json_object_array_add(body,json_object_new_string(name));
            json_object_array_add(body,json_object_new_string(key));
            // add body to init conection
            json_object_object_add(init_connection,"body",body);

            //convert json to string
            instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            // clear the init connection

            //wait response
            //send(sockfd, instruccion, BUFFER_SIZE, 0);

        }
        else if(strcmp(token,"--c")==0){
            key = strtok(NULL, " ");
            //create json for change status and send to server
            struct json_object *init_connection = json_object_new_object();
            //add request to json
            json_object_object_add(init_connection,"request",json_object_new_string("POST_CHAT"));
            //create json for send msg to certain user and send to server

            //create body
            struct json_object *body = json_object_new_array();

            //add to the array
            json_object_array_add(body,json_object_new_string(key));
            json_object_array_add(body,json_object_new_string(name));
            json_object_array_add(body,json_object_new_string(name));
            json_object_array_add(body,json_object_new_string("all"));
            // add body to init conection
            json_object_object_add(init_connection,"body",body);

            //convert json to string
            instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            // clear the init connection

            //wait response
            //send(sockfd, instruccion, BUFFER_SIZE, 0);

        }
        else if (strcmp(token, "get_users")==0){
            //get_user
            //ask info of all connected users

            struct json_object *init_connection = json_object_new_object();
            //add request to json
            json_object_object_add(init_connection,"request",json_object_new_string("GET_USER"));
            json_object_object_add(init_connection,"body",json_object_new_string("all"));

            //convert json to string
            instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            
            //send(sockfd, instruccion, BUFFER_SIZE, 0);
        }
        else if (strcmp(token, "get_user")==0){
            //get_user
            //ask info of a user
            key = strtok(NULL, " ");
            //key = username 
            //create json for info of a user and send to server
            struct json_object *init_connection = json_object_new_object();
            //add request to json
            json_object_object_add(init_connection,"request",json_object_new_string("PUT_STATUS"));
            json_object_object_add(init_connection,"body",json_object_new_string(key));

            //convert json to string
            instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
            
            //send(sockfd, instruccion, BUFFER_SIZE, 0);

        }else if(strcmp(token, "--help")==0){
            printf("COMANDOS: \n --------------------------------------------------------------- \n SALIR: exit\n CAMBIAR ESTADO: change_status <num_state>\n MENSAJE PRIVADO: send_to <user> <message>\n CHAT GENERAL: --c <message>\n USUARIOS CONECTADOS: get_users\n USUARIO CONECTADO EN ESPECIFICO: get_user <user>\n --------------------------------------------------------------- \n");
        }
        else{
            //send msg to all users
            //post_chat
            //create json for send msg to all users and send to server
            //printf("yo-> %s\n",message_copy);
            // printf("token: %s\n",token);
            // printf("key: %s\n",key);
            // printf("val: %s\n",value);
            // sprintf(buffer, "%s: %s\n", name, message_copy);
            // send(sockfd, buffer, strlen(buffer), 0);
            //prueba
            continue;
        }

        send(sockfd, instruccion, BUFFER_SIZE, 0);

        
       
        bzero(message, BUFFER_SIZE);
        

        bzero(buffer, BUFFER_SIZE + 32);
        
        
    }
    catch_commands(2);
    
}


void receive_message(){
    char message[BUFFER_SIZE] = {};
    char *json_instruccion1;
    char *response;
    char *opcion1;
    char *code_recv;
    char *msg;
    char *from_user;
    char *time_send; 
    char *ip_user;
    char *status_user;
    char *CODE_READ;
    
    while (1){
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0){
            //copy buffer out to json_instruccion
           //printf("%s\n", message);
            json_instruccion1 = message;
            //create json objet
            struct json_object *new_instruccion1 = json_object_new_object();
            //parser the json
            new_instruccion1 = json_tokener_parse(json_instruccion1);

            //get request
            struct json_object *request;
            json_object_object_get_ex(new_instruccion1,"request",&request);
            opcion1 = json_object_get_string(request);
            struct json_object *body;
            json_object_object_get_ex(new_instruccion1, "body",&body); 
            struct json_object *code;
            json_object_object_get_ex(new_instruccion1,"code",&code);

            if(strcmp(opcion1,"NEW_MESSAGE") == 0){
                //get msg from server
                json_object_object_get_ex(new_instruccion1,"body",&body);
                msg = json_object_get_string(json_object_array_get_idx(body,0));
                from_user = json_object_get_string(json_object_array_get_idx(body,1));
                time_send = json_object_get_string(json_object_array_get_idx(body,2));
                name = json_object_get_string(json_object_array_get_idx(body,3));

                if (strcmp(name, "all") == 0)
                {
                    printf("%s -> %s \n", from_user, msg);
                }else{
                    printf("%s to %s -> %s\n", from_user, name, msg);
                }
                

            }
            else if(strcmp(opcion1,"GET_USER") == 0){
                //get info of a user or all of them
                json_object_object_get_ex(new_instruccion1,"body",&body);
                continue;
            }
            else if(strcmp(opcion1,"GET_CHAT") == 0){
                //useless response made by useless people
                json_object_object_get_ex(new_instruccion1,"body",&body);
                continue;
            }
            else if(strcmp(opcion1,"POST_CHAT") == 0){
                //get info of a user or all of them
                CODE_READ = json_object_get_string(code);
                if (strcmp(CODE_READ,"200") == 0){
                    continue;
                }
                //if opcion == 101 no tudo bem el usuario ya existe
                else if(strcmp(CODE_READ,"102") == 0){
                    printf("----EL USUARIO NO  ESTA CONECTADO----\n");
                }
            }
            else if(strcmp(opcion1,"END_CONEX") == 0){
                //get info of a user or all of them
                CODE_READ = json_object_get_string(code);
                if (strcmp(CODE_READ,"200") == 0){
                    continue;
                }
                //if opcion == 101 no tudo bem el usuario ya existe
                else if(strcmp(CODE_READ,"105") == 0){
                    printf("----ERROR INESPERADO ----\n");
                    flag=1;
                }
            }
            else if(strcmp(opcion1,"PUT_STATUS") == 0){
                //get info of a user or all of them
                CODE_READ = json_object_get_string(code);
                if (strcmp(CODE_READ,"200") == 0){
                    continue;
                }
                //if opcion == 101 no tudo bem el usuario ya existe
                else if(strcmp(CODE_READ,"104") == 0){
                    printf("----NO SE PUDO MODIFICAR :(----\n");
                }  
            }

            //printf("%s\n", opcion1);

        }else if (receive == 0){
            break;
            flag =1;
        }
        memset(message, 0, sizeof(message));
    }
}


int main(int argc, char **argv)
{
    if(argc == 1){
        printf("conexiones chingando");
    }
    if(argc>=4){

        int PORT = atoi(argv[3]);
        char *IP = (argv[2]);
        name = (argv[1]);
        char *instruccion;
        char *connection_date;
        //int sockfd, n;
        int sendbytes;
        struct sockaddr_in servaddr;
        char *json_instruccion;
        char *opcion;
        

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


        // init conex
        //create json and send to server
        struct json_object *init_connection = json_object_new_object();
        //add request to json
        json_object_object_add(init_connection,"request",json_object_new_string("INIT_CONEX"));
        //create body
        struct json_object *body = json_object_new_array();
        
        //get time of connection
        time_t current_time;
        time(&current_time);
        connection_date = ctime(&current_time);

        //add to the array
        json_object_array_add(body,json_object_new_string(connection_date));
        json_object_array_add(body,json_object_new_string(name));
        // add body to init conection
        json_object_object_add(init_connection,"body",body);

        //convert json to string
        instruccion = json_object_to_json_string_ext(init_connection, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY);
        // clear the init connection

        //wait response
        send(sockfd, instruccion, BUFFER_SIZE, 0);


        //to get the respond from chat
        char message[BUFFER_SIZE] = {};
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);

        //check the response from the server
        json_instruccion = message;
        //create json objet
        struct json_object *new_instruccion = json_object_new_object();
        //parser the json
        new_instruccion = json_tokener_parse(json_instruccion);

        //get request
        struct json_object *request;
        json_object_object_get_ex(new_instruccion,"request",&request);

        //get body
        struct json_object *code;
        json_object_object_get_ex(new_instruccion,"code",&code);

        opcion = json_object_get_string(code);


        //if opcion == 200 tudo bem
        if (strcmp(opcion,"200") == 0){
            printf("----welcome to the bate papo----\n");

        }
        //if opcion == 101 no tudo bem el usuario ya existe
        else if(strcmp(opcion,"101") == 0){
            printf("----el username ya existe----\n");
            flag=1;
        }

        
        bzero(message,BUFFER_SIZE);
        


        pthread_t send_msg_thread;
        if(pthread_create(&send_msg_thread, NULL, (void*)send_message, NULL) != 0){
            printf("ERROR: pthread.\n");
            return -1;
        }
        pthread_t recv_msg_thread;
        if(pthread_create(&recv_msg_thread, NULL, (void*)receive_message, NULL) != 0){
            printf("ERROR: pthread.\n");
            return EXIT_FAILURE;
        }
        while (1){
            if(flag==1){
            printf("\nBye\n");
            break;
        }
        }
        

        /* close the socket */
        close(sockfd); 
    }
}