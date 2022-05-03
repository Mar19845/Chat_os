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

    while (1){
        str_overwrite_stdout();
        fgets(message, BUFFER_SIZE, stdin);

        str_trim_lf(message, BUFFER_SIZE);

        char* token = strtok(message, " ");

        strcpy(message_copy, message);
        if (strcmp(token, "exit") == 0){// Exit chat
            break;
        }else{
            sprintf(buffer, "%s\n",message_copy);
            send(sockfd, buffer, strlen(buffer), 0);
            //prueba
            printf("yo -> %s\n",buffer);
        }
        

        bzero(message, BUFFER_SIZE);
        bzero(buffer, BUFFER_SIZE + 32);
    }
    catch_commands(2);
    
}

void receive_message(){
    char message[BUFFER_SIZE] = {};
    while (1)
    {
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0){
            printf("%s", message);
            str_overwrite_stdout();
        }else if (receive == 0){
            break;
        }
        memset(message, 0, sizeof(message));
    }
}

void menu_chat(){
    printf("\n");
    printf("\n1. CHAT GENERAL\n 2. CHAT PRIVADO\n 3. CAMBIAR STATUS\n 4. USUARIOS ACTIVOS\n 5. INFORMACION DE USUARIOS\n 6. AYUDA\n 7. SALIR\n");
    printf("\n");
}

void privado(){
    char message[BUFFER_SIZE] = {};
    char user[BUFFER_SIZE] = {};
    char buffer[BUFFER_SIZE] = {};
    char temp;

    printf("Ingresa el usuario al que quieres enviarle el mensaje: ");
    scanf("%[^\n]s", &user);

    printf("Ingresa el mensaje: ");
    scanf("%c", &temp);

    str_overwrite_stdout();
    scanf("%[^\n]s", &message);

    if(strcmp(message, "exit") == 0){
        return;
    }
    else{
        send(sockfd,  buffer, strlen(buffer), 0);
    }

    bzero(message, BUFFER_SIZE);
    bzero(buffer, BUFFER_SIZE + 32);
}

void change_status(){
    int status_choice;
    char buffer[BUFFER_SIZE + 32] = {};
    char temp;
    char *status;
    scanf("%c", &temp);
    printf("Enter Status: \n 1.Offline, \n 2.Online \n 3.Busy\n");
    scanf("%d",&status_choice);

    switch(status_choice){
        case 1:
            status = STATUS_INACTIVE;
            break;
        case 2:
            status = STATUS_ACTIVE;
            break;
        case 3:
            status = STATUS_BUSY;
            break;
        default:
            printf("OPCION INVALIDA, INTENTE DE NUEVO\n");
            break;
    }
    sprintf(buffer, "%s: %s\n", name, status);
    send(sockfd, buffer, strlen(buffer), 0);
    bzero(buffer, BUFFER_SIZE + 32);
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
        send(sockfd, name, 32, 0);
        printf("----welcome to the bate papo----\n");


        pthread_t send_msg_thread;
        if(pthread_create(&send_msg_thread, NULL, (void*)send_message, NULL) != 0){
            printf("ERROR: pthread.\n");
            return -1;
        }
        
        while (1){
            if(flag){
            printf("\nBye\n");
            break;
        }
        }
        

        /* close the socket */
        close(sockfd); 
    }
}