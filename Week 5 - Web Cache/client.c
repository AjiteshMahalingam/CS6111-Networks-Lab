#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT 3503
#define MAX 1024

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

struct Request{
    char requestLine[MAX];
    char requestHeaders[MAX];
    char requestBody[MAX];
};

struct Response{
    char statusLine[MAX];
    char responseHeaders[MAX];
    char responseTime[MAX];
    struct Student responseBody;
};

int main(){
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientSocket < 0){
        printf("[-] Socket creation failed.\n");
        exit(1);
    }

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("[-] Unable to connect to the server.\n");
        exit(1);
    }
    printf("[+] Connected to the server.\n");

    long rollNum;
    int sem;
    int semesters[4] = {1, 2, 3, 4};
    struct Student reqStudent;
    
    printf("Enter roll number : ");
    scanf("%ld", &rollNum);

    sem = semesters[0];
    struct Request request;
    sprintf(request.requestLine, "GET /%ld/%d HTTP/1.1\n", rollNum, sem);
    sprintf(request.requestHeaders, "Host: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: keep-alive\n\n");
    sprintf(request.requestBody, "%ld,%d", rollNum, sem);
    
    write(clientSocket, &request, sizeof(request));
    
    struct Response response;
    read(clientSocket, &response, sizeof(response));
    printf("\n%s%s%s\n", response.statusLine, response.responseHeaders, response.responseTime );
    reqStudent = response.responseBody;
    printf("------------------------\nStudent details : \n------------------------\nName : %s\nReg. No : %ld\nDept : %s - %c batch\nSemester : %d\n------------------------\n%s - %d/10\n%s - %d/10\n%s - %d/10\n%s - %d/10\n%s - %d/10\n------------------------\nDate - time: %s------------------------\n", reqStudent.name, reqStudent.regNum, reqStudent.dept, reqStudent.batch, reqStudent.sem, reqStudent.subjects[0].name, reqStudent.subjects[0].grade, reqStudent.subjects[1].name, reqStudent.subjects[1].grade, reqStudent.subjects[2].name, reqStudent.subjects[2].grade, reqStudent.subjects[3].name, reqStudent.subjects[3].grade, reqStudent.subjects[4].name, reqStudent.subjects[4].grade, response.responseTime);
    
    sem = semesters[1];
    
    sprintf(request.requestLine, "GET /%ld/%d HTTP/1.1\n", rollNum, sem);
    sprintf(request.requestHeaders, "Host: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: keep-alive\n\n");
    sprintf(request.requestBody, "%ld,%d", rollNum, sem);
    
    write(clientSocket, &request, sizeof(request));
    
    read(clientSocket, &response, sizeof(response));
    printf("\n%s%s%s\n", response.statusLine, response.responseHeaders, response.responseTime );
    reqStudent = response.responseBody;
    printf("------------------------\nStudent details : \n------------------------\nName : %s\nReg. No : %ld\nDept : %s - %c batch\nSemester : %d\n------------------------\n%s - %d/10\n%s - %d/10\n%s - %d/10\n%s - %d/10\n%s - %d/10\n------------------------\nDate - time: %s------------------------\n", reqStudent.name, reqStudent.regNum, reqStudent.dept, reqStudent.batch, reqStudent.sem, reqStudent.subjects[0].name, reqStudent.subjects[0].grade, reqStudent.subjects[1].name, reqStudent.subjects[1].grade, reqStudent.subjects[2].name, reqStudent.subjects[2].grade, reqStudent.subjects[3].name, reqStudent.subjects[3].grade, reqStudent.subjects[4].name, reqStudent.subjects[4].grade, response.responseTime);
        
    sem = semesters[0];
    
    sprintf(request.requestLine, "GET /%ld/%d HTTP/1.1\n", rollNum, sem);
    sprintf(request.requestHeaders, "Host: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: keep-alive\n\n");
    sprintf(request.requestBody, "%ld,%d", rollNum, sem);
    
    write(clientSocket, &request, sizeof(request));
    
    read(clientSocket, &response, sizeof(response));
    printf("\n%s%s%s\n", response.statusLine, response.responseHeaders, response.responseTime );
    reqStudent = response.responseBody;
    printf("------------------------\nStudent details : \n------------------------\nName : %s\nReg. No : %ld\nDept : %s - %c batch\nSemester : %d\n------------------------\n%s - %d/10\n%s - %d/10\n%s - %d/10\n%s - %d/10\n%s - %d/10\n------------------------\nDate - time: %s------------------------\n", reqStudent.name, reqStudent.regNum, reqStudent.dept, reqStudent.batch, reqStudent.sem, reqStudent.subjects[0].name, reqStudent.subjects[0].grade, reqStudent.subjects[1].name, reqStudent.subjects[1].grade, reqStudent.subjects[2].name, reqStudent.subjects[2].grade, reqStudent.subjects[3].name, reqStudent.subjects[3].grade, reqStudent.subjects[4].name, reqStudent.subjects[4].grade, response.responseTime);
    
    sprintf(request.requestLine, "EXIT");    
    sprintf(request.requestHeaders, "Host: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: close\n\n");
    sprintf(request.requestBody, " ");
    write(clientSocket, &request, sizeof(request));

    close(clientSocket);
    return 0;
}