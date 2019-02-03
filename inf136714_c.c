#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define M 1000

#define MAX_LOGIN_TRIES 3
#define MAX_LOGIN_LENGTH 50
#define MAX_PASSWORD_LENGTH 20

#define mreg 10
#define mlog 11
#define mtop 12
#define msnd 13
#define mread 14
#define msub 15

struct srvquery{
    long type;
    long ip;
    char message[M];
} query;

struct srvresponse{
    long type;
    char message[M];
} response;

char login[MAX_LOGIN_LENGTH];
char password[MAX_PASSWORD_LENGTH];

int id;
char lastTopic[M];

void sreg(){
    printf("Login: ");
    scanf("%s",login);
    printf("Password: ");
    scanf("%s",password);
    query.type = mreg;
    strcpy(query.message, login);
    strcat(query.message, "|");
    strcat(query.message, password);
    msgsnd(id,&query,sizeof(query)-sizeof(query.type),0);
}

void slogin(){
    query.type = mlog;
    strcpy(query.message, login);
    strcat(query.message, "|");
    strcat(query.message, password);
    msgsnd(id,&query,sizeof(query)-sizeof(query.type),0);
}

void sloginother(){
    printf("Login: ");
    scanf("%s",login);
    printf("Password: ");
    scanf("%s",password);
    slogin();
}

void sregtopic(){
    printf("Enter topic name (\"-\" will generate an automatic topic)\n");
    scanf("%s",query.message);
    query.type = mtop;
    msgsnd(id,&query,sizeof(query)-sizeof(query.type),0);
}

void ssend(){
    query.type = msnd;
    printf("Topic: ");
    scanf("%s",query.message);
    char m[M];
    printf("Message: ");
    scanf("%c",&m[0]);
    scanf("%[^\n]s",m);
    strcat(query.message,"|");
    strcat(query.message,m);
    msgsnd(id,&query,sizeof(query)-sizeof(query.type),0);
}

void sread(){
    query.type = mread;
    printf("Topic: ");
    scanf("%s",query.message);
    msgsnd(id,&query,sizeof(query)-sizeof(query.type),0);
}

void sreadasynch(){
    printf("Received a message from a subcribed topic!\n");
    query.type = mread;
    strcpy(query.message,lastTopic);
    msgsnd(id,&query,sizeof(query)-sizeof(query.type),0);
    msgrcv(id,&response,sizeof(response)-sizeof(response.type),query.ip,0);
    printf("%s\n",response.message);
    exit(0);
}

void ssub(){
    query.type = msub;
    printf("Topic: ");
    scanf("%s",query.message);
    strcpy(lastTopic,query.message);
    msgsnd(id,&query,sizeof(query)-sizeof(query.type),0);
    if(fork()==0){
        signal(16,sreadasynch);
        pause();
    } else {
        signal(16,SIG_IGN);
    }
}

int main(){
    login[0]='-';
    id = msgget(0x127,0777|IPC_CREAT);
    query.ip = getpid();
    response.type = query.ip;
    while(1){
        if(login[0]!='-'){
            printf("\n\nHello, %s\n",login);
        }
        printf("Choose an option:\n1. Register\n");
        if(login[0]!='-'){
            printf("2. Login\n");
        }
        printf("3. Login with different user\n");
        if(login[0]!='-'){
            printf("4. Register a new topic\n");
            printf("5. Send message\n");
            printf("6. Read message\n");
            printf("7. Subscribe to a topic\n");
        }
        int action;
        scanf("%d",&action);
        printf("\x1b[H\x1b[J");
        switch(action){
            case 1:
                sreg();
                break;
            case 2:
                slogin();
                break;
            case 3:
                sloginother();
                break;
            case 4:
                sregtopic();
                break;
            case 5:
                ssend();
                break;
            case 6:
                sread();
                break;
            case 7:
                ssub();
                break;
        }
        msgrcv(id,&response,sizeof(response)-sizeof(response.type),query.ip,0);
        printf("%s\n",response.message);
    }
    return 0;
}














































