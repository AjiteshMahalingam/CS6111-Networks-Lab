#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT 3503

struct Subject{
    char name[40];
    int grade;
};

struct Student{
    char name[30];
    long regNum;
    char dept[10];
    char batch;
    int sem;
    struct Subject subjects[5];
};

int main(){
    // Connection #1
    int clientSocket;
    struct sockaddr_in serverAddr;
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("[-] Connection request failed.\n");
        exit(1);
    }
    printf("[+] Connected to the server.\n");

    long rollNum;
    int sem;
    int semesters[4] = {1, 2, 3, 4};
    struct Student student;
    
    printf("Enter roll number : ");
    scanf("%ld", &rollNum);

    sem = semesters[0];
    char request[3000] = {0};
    sprintf(request, "GET /%ld/%d HTTP/1.1\nHost: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: close\n\n", rollNum, sem);    
    write(clientSocket, request, 3000);
    
    char response[3000] = {0};
    read(clientSocket, response, 3000);
    printf("\n%s\n", response);
    
    close(clientSocket);
    printf("[+] Socket closed.\n");

    // Connection #2
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");

    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("[-] Connection request failed.\n");
        exit(1);
    }
    printf("[+] Connected to the server.\n");

    sem = semesters[1];
    request[0] = '\0';
    sprintf(request, "GET /%ld/%d HTTP/1.1\nHost: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: close\n\n", rollNum, sem);    
    write(clientSocket, request, 3000);
    
    response[0] = '\0';
    read(clientSocket, response, 3000);
    printf("\n%s\n", response);

    close(clientSocket);
    printf("[+] Socket closed.\n");

    // Connection #3
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");

    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("[-] Connection request failed.\n");
        exit(1);
    }
    printf("[+] Connected to the server.\n");

    sem = semesters[2];
    request[0] = '\0';
    sprintf(request, "GET /%ld/%d HTTP/1.1\nHost: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: close\n\n", rollNum, sem);    
    write(clientSocket, request, 3000);

    response[0] = '\0';
    read(clientSocket, response, 3000);
    printf("\n%s\n", response);
    
    close(clientSocket);
    printf("[+] Socket closed.\n");

    // Connection #4
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");

    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("[-] Connection request failed.\n");
        exit(1);
    }
    printf("[+] Connected to the server.\n");

    sem = semesters[3];
    request[0] = '\0';
    sprintf(request, "GET /%ld/%d HTTP/1.1\nHost: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: close\n\n", rollNum, sem);    
    write(clientSocket, request, 3000);

    response[0] = '\0';
    read(clientSocket, response, 3000);
    printf("\n%s\n", response);
    
    close(clientSocket);
    printf("[+] Socket closed.\n");
    
    return 0;
}
