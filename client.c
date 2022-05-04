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
char *name[32];
int sockfd = 0;
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
    char message_copy[BUFFER_SIZE] = {};//copy of the message for
    char buffer[BUFFER_SIZE + 32] = {};

    //char *token;
    //const char *delim = " ";

    while (1){

        str_overwrite_stdout();
        fgets(message, BUFFER_SIZE, stdin);

        str_trim_lf(message, BUFFER_SIZE);

        strcpy(message_copy, message);

        char *key, *value;
        char* token = strtok(message, " ");
        

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
            break;

        }
        else if (strcmp(token, "change_status")==0){
            //put_status
            key = strtok(NULL, " ");
            //key = status 
            //create json for change status and send to server
        }
        else if (strcmp(token, "send_to")==0){

            //post chat
            // post chat to given user
            key = strtok(NULL, " ");
            //key = username 
            value = strtok(NULL, " ");
            //value = msg

            printf("yo-> %s\n",message_copy);
            printf("token: %s\n",token);
            printf("key: %s\n",key);
            printf("val: %s\n",value);
            //create json for send msg to certain user and send to server
        }
        else if (strcmp(token, "get_users")==0){
            //get_user
            //ask info of all connected users
            //create json for info of all connected users and send to server
        }
        else if (strcmp(token, "get_user")==0){
            //get_user
            //ask info of a user
            key = strtok(NULL, " ");
            //key = username 
            //create json for info of a user and send to server
        }
        else{
            //send msg to all users
            //post_chat
            //create json for send msg to all users and send to server
            printf("yo-> %s\n",message_copy);
            printf("token: %s\n",token);
            printf("key: %s\n",key);
            printf("val: %s\n",value);
            sprintf(buffer, "%s: %s\n", name, message_copy);
            send(sockfd, buffer, strlen(buffer), 0);
            //prueba
        }
        

        bzero(message, BUFFER_SIZE);
        bzero(buffer, BUFFER_SIZE + 32);
    }
    catch_commands(2);
    
}

void receive_message(){
    char message[BUFFER_SIZE] = {};
    char error_msg[BUFFER_SIZE] = "Username already exists.\n";
    char timeout_msg[BUFFER_SIZE] = "Sesion has timeout.\n";
    while (1)
    {
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0){
            printf("%s", message);
            str_overwrite_stdout();

            //change to response json
            if(strcmp(error_msg, message) == 0){
                flag = 1;
                break;
            }
            //change to response json
            else if(strcmp(timeout_msg, message) == 0){
                flag = 1;
	            break;
	        }
            str_overwrite_stdout();
        }else if (receive == 0){
            break;
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
        char *name = (argv[1]);
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