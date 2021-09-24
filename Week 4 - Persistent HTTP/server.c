#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT 3503
#define MAX 1024
#define N 4

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

struct Student students[N];

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

void initDS();

int main(){
    initDS();

    // Server socket set-up
    int serverSocket;
    struct sockaddr_in serverAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocket < 0){
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    
    int opt = 1;
    if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        printf("[-] Socket set options failed.\n");
        exit(1);
    }

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        printf("[-] Socket bind failed.\n");
        exit(1);
    }

    if(listen(serverSocket, N) < 0){
        printf("[-] Socket listen failed.\n");
        exit(1);
    }
    printf("[+] Server running at port %d.\n", PORT);
    
    // Accept client request and send corresp response
    int clientSocket;
    while(1){
        clientSocket = accept(serverSocket, NULL, NULL);
        if(clientSocket < 0){
            printf("[-] Client request not accepted.\n");
            exit(1);
        }

        printf("[+] New client connected.\n");
        while(1){
            long rollNum = 0;
            int sem = 0;
            struct Student reqStudent;
            struct Request request;

            if(read(clientSocket, &request, sizeof(request)) > 0){
                printf("\n%s\n%s\n%s\n", request.requestLine, request.requestHeaders, request.requestBody);
                
                if(strncmp(request.requestLine, "EXIT", 4) == 0){
                    printf("[+] Client connection closed.\n");
                    close(clientSocket);
                    break;
                }
                
                char *reqMsg = strtok(request.requestBody, ",");
                rollNum = strtol(reqMsg, NULL, 10);
                reqMsg = strtok(NULL, ",");
                sem= strtol(reqMsg, NULL, 10);
                
                printf("[+] Client requested data for reg.no %ld - Semester %d.\n",rollNum, sem);

                time_t currTime;
                time(&currTime);
                
                reqStudent = students[sem-1];

                struct Response response;
                sprintf(response.statusLine, "HTTP/1.1 200 OK\n");
                sprintf(response.responseTime, "Date : %s", ctime(&currTime));
                response.responseBody = reqStudent;
                sprintf(response.responseHeaders, "Connection: keep-alive\nContent-Type: text/plain\nContent-Length: %ld\n", sizeof(response.responseBody));
                                
                write(clientSocket, &response, sizeof(response));
                printf("[+] Client request completed and connection kept alive.\n");
            }
        }
    }
    close(serverSocket);
    return 0;
}

void initDS(){
    // Data structure to store semester results
    // Semester 1
    sprintf(students[0].name,"Ajitesh M");
    students[0].regNum = 2019103503;
    sprintf(students[0].dept, "CSE");
    students[0].batch = 'Q';
    students[0].sem = 1;
    sprintf(students[0].subjects[0].name,"C Programming");
    students[0].subjects[0].grade = 9;
    sprintf(students[0].subjects[1].name, "Computational Thinking");
    students[0].subjects[1].grade = 9;
    sprintf(students[0].subjects[2].name,"Engineering Mathematics");
    students[0].subjects[2].grade = 10;
    sprintf(students[0].subjects[3].name, "Engineering Physics");
    students[0].subjects[3].grade = 10;
    sprintf(students[0].subjects[4].name,"Technical English 1");
    students[0].subjects[4].grade = 9;
    
    // Semester 2
    sprintf(students[1].name,"Ajitesh M");
    students[1].regNum = 2019103503;
    sprintf(students[1].dept, "CSE");
    students[1].batch = 'Q';
    students[1].sem = 2;
    sprintf(students[1].subjects[0].name,"Application Development Practices");
    students[1].subjects[0].grade = 9;
    sprintf(students[1].subjects[1].name, "Engineering Chemistry");
    students[1].subjects[1].grade = 10;
    sprintf(students[1].subjects[2].name,"Engineering Graphics");
    students[1].subjects[2].grade = 9;
    sprintf(students[1].subjects[3].name, "Discrete Mathematics");
    students[1].subjects[3].grade = 10;
    sprintf(students[1].subjects[4].name,"Technical English 2");
    students[1].subjects[4].grade = 9;

    // Semester 3
    sprintf(students[2].name,"Ajitesh M");
    students[2].regNum = 2019103503;
    sprintf(students[2].dept, "CSE");
    students[2].batch = 'Q';
    students[2].sem = 3;
    sprintf(students[2].subjects[0].name,"Data Structures and Algorithms");
    students[2].subjects[0].grade = 10;
    sprintf(students[2].subjects[1].name, "Digital Fundamentals");
    students[2].subjects[1].grade = 9;
    sprintf(students[2].subjects[2].name,"Basic EEE");
    students[2].subjects[2].grade = 9;
    sprintf(students[2].subjects[3].name, "Probability and Statistics");
    students[2].subjects[3].grade = 9;
    sprintf(students[2].subjects[4].name,"Interview Skills");
    students[2].subjects[4].grade = 9;

    // Semester 4
    sprintf(students[3].name,"Ajitesh M");
    students[3].regNum = 2019103503;
    sprintf(students[3].dept, "CSE");
    students[3].batch = 'Q';
    students[3].sem = 4;
    sprintf(students[3].subjects[0].name,"Database management systems");
    students[3].subjects[0].grade = 10;
    sprintf(students[3].subjects[1].name, "Operating systems");
    students[3].subjects[1].grade = 10;
    sprintf(students[3].subjects[2].name,"Computer Architecture");
    students[3].subjects[2].grade = 9;
    sprintf(students[3].subjects[3].name, "Theory of Computation");
    students[3].subjects[3].grade = 9;
    sprintf(students[3].subjects[4].name,"Environmental Studies");
    students[3].subjects[4].grade = 9;
       
}