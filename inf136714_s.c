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

struct user{
    char login[MAX_LOGIN_LENGTH];
    char password[MAX_PASSWORD_LENGTH];
    int attempts;
};

struct topic{
    char title[M];
    long subs[20];
    char message[M];
    int currentSubs;
};

struct topic topics[200];
int currentTopics;

struct user users[200];
int currentUsers;
int id;

void reg(char login[], char password[]){
    strcpy(users[currentUsers].login,login);
    strcpy(users[currentUsers].password,password);
    users[currentUsers].attempts = 0;
    currentUsers++;
}

int login(char login[], char password[]){
    for(int x=0;x<currentUsers;x++){
        if(strcmp(users[x].login,login)==0){
            if(users[x].attempts >= MAX_LOGIN_TRIES){
                return -3;
            }
            if(strcmp(users[x].password,password)==0){
                users[x].attempts=0;
                return 1;
            } else {
                users[x].attempts++;
                return -2;
            }
        }
    }
    return -1;
}

void rlogin(struct srvquery q){
    char *l = strtok(q.message, "|");
    char *p = strtok(NULL, "|");
    int result = login(l,p);
    printf("Login attempt L:%s P:%s %d\n",l,p,result);
    switch(result){
        case -3:
            strcpy(response.message,"Too many login attempts!");
            break;
        case -2:
            strcpy(response.message,"Wrong password!");
            break;
        case -1:
            strcpy(response.message,"No such login!");
            break;
        case 1:
            strcpy(response.message,"Login successful!");
            break;
    }
    printf("%s\n",response.message);
    
    response.type = q.ip;
    msgsnd(id,&response,sizeof(response)-sizeof(response.type),0);
}

void rregister(struct srvquery q){
    char *l = strtok(q.message, "|");
    char *p = strtok(NULL, "|");
    printf("Registering L:%s P:%s\n",l,p);
    reg(l,p);
    strcpy(response.message,"Registration successful!");
    
    response.type = q.ip;
    msgsnd(id,&response,sizeof(response)-sizeof(response.type),0);
}

void rregtopic(struct srvquery q){
    if(strcmp(q.message,"-")==0){
        strcpy(q.message,"aaa");
        q.message[0]+=q.ip%('z'-'a');
        q.message[1]+=(q.ip/('z'-'a'))%('z'-'a');
        q.message[2]+=(q.ip/10)%('z'-'a');
    }
    printf("registering new topic: %s\n",q.message);
    
    strcpy(topics[currentTopics].title,q.message);
    topics[currentTopics].currentSubs = 0;
    currentTopics++;
    
    strcpy(response.message,"Succesfully registered topic: ");
    strcat(response.message,q.message);
    printf("%s\n",q.message);
    
    response.type = q.ip;
    msgsnd(id,&response,sizeof(response)-sizeof(response.type),0);
}

void rsend(struct srvquery q){
    char *t = strtok(q.message, "|");
    char *m = strtok(NULL, "|");
    
    int i;
    for(i=0;i<currentTopics;i++){
        if(strcmp(topics[i].title,t)==0){
            break;
        }
    }
    if(i>=currentTopics){
        strcpy(response.message,"Nonexistent topic!");
    }
    else{
        strcpy(topics[i].message,m);
        printf("Topic: %s\n",t);
        printf("Saved message: %s\n",m);
        for(int x=0;x<topics[i].currentSubs;x++){
            printf("Sending message to: %ld\n",topics[i].subs[x]);
            kill(-(int)topics[i].subs[x],16);
        }
        strcpy(response.message,"Succesfully sent message!");
    }
    
    response.type = q.ip;
    msgsnd(id,&response,sizeof(response)-sizeof(response.type),0);
}

void rread(struct srvquery q){
    int i;
    for(i=0;i<currentTopics;i++){
        if(strcmp(topics[i].title,q.message)==0){
            break;
        }
    }
    if(i>=currentTopics || topics[i].message[0]=='-'){
        strcpy(response.message,"Nonexistent topic!");
    }
    else{
        strcpy(response.message,"Message:\n");
        strcat(response.message,topics[i].message);
        //topics[i].message[0]='-';
    }
    
    response.type = q.ip;
    msgsnd(id,&response,sizeof(response)-sizeof(response.type),0);
}

void rsub(struct srvquery q){
    int i;
    for(i=0;i<currentTopics;i++){
        if(strcmp(topics[i].title,q.message)==0){
            break;
        }
    }
    if(i>=currentTopics){
        strcpy(response.message,"Nonexistent topic!");
    }
    else{
        strcpy(response.message,"Successfully subscribed!");
        printf("Added sub to %s with ip=%ld\n",topics[i].title,q.ip);
        topics[i].subs[topics[i].currentSubs]=q.ip;
        topics[i].currentSubs=topics[i].currentSubs+1;
    }
    
    response.type = q.ip;
    msgsnd(id,&response,sizeof(response)-sizeof(response.type),0);
}

void defaultresponse(struct srvquery q){
    printf("Invalid message of type %lu received from %lu.\n",q.type,q.ip);
    strcpy(response.message,"Unexpected server error!");
    
    response.type = q.ip;
    msgsnd(id,&response,sizeof(response)-sizeof(response.type),0);
}

void msgloop(){
    while(1){
        msgrcv(id,&query,sizeof(query)-sizeof(query.type),-20,0);
        printf("Received message from %ld\n%s\n",query.ip,query.message);
        switch(query.type){
            case mreg:
                rregister(query);
                break;
            case mlog:
                rlogin(query);
                break;
            case mtop:
                rregtopic(query);
                break;
            case msnd:
                rsend(query);
                break;
            case mread:
                rread(query);
                break;
            case msub:
                rsub(query);
                break;
            default:
                defaultresponse(query);
                break;
        }
    }
}

int main(){
    id = msgget(0x127,0777|IPC_CREAT);
    currentTopics = 0;
    currentUsers = 0;
    printf("Started\n");
    msgloop();
    return 0;
}














































