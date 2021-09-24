#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>

#define PORT 3503
#define MAX 1024
#define N 5
#define M 5

struct Subject{
    char name[20];
    int marks;
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

void initDS();

int findStudent(long rollNum){
    int index = N;
    for(int i = 0; i < N; i++){
        if(students[i].regNum == rollNum){
            index = i;
            break;
        }
    }
    return index;
}

int clientSockets[M];

void *manageClients(void *sockfd){
    int clientSocket = *((int *)sockfd);
    long rollNum = 0;
    struct Student reqStudent;
    int index = 0;
    for(int i = 0; i < M; i++){
        if(clientSockets[i] == clientSocket){
            index = i;
            break;
        }
    }
    printf("[+] Client %d connected.\n", index);

    if(read(clientSocket, &rollNum, sizeof(rollNum)) > 0){
        rollNum = ntohl(rollNum);
        printf("[+] Client %d requested data for reg.no %ld.\n", index, rollNum);
        int id = findStudent(rollNum);
        if(id == N)
            reqStudent.regNum = -1;
        else
            reqStudent = students[id];
        write(clientSocket, &reqStudent, sizeof(reqStudent));
        printf("[+] Client %d request completed.\n", index);
    }
    printf("[+] Client %d disconnected.\n", index);
    clientSockets[index] = 0;
    close(clientSocket);
    
    return NULL;
}

int main(){
    initDS();

    // Server socket set-up
    int serverSocket;
    struct sockaddr_in serverAddr;
    pthread_t clientThreads[M];

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
    for(int i = 0; i < M; i++)
        clientSockets[M] = 0;
    while(1){
        int index = M;
        for(int i = 0; i < M; i++){
            if(clientSockets[i] == 0){
                index = i;
                break;
            }
        }
        if(index == M)
            continue;
        clientSockets[index] = accept(serverSocket, NULL, NULL);
        if(clientSockets[index] < 0){
            printf("[-] Client request not accepted.\n");
            exit(1);
        }
        pthread_create(&clientThreads[index], NULL, manageClients, (void *)&clientSockets[index]);
    }

    for(int i = 0; i < M; i++)
        pthread_join(clientThreads[i], NULL);

    close(serverSocket);
    return 0;
}

void initDS(){
    // Data structure to store students and their marks
    // Student 1
    sprintf(students[0].name,"Ajitesh M");
    students[0].regNum = 2019103503;
    sprintf(students[0].dept, "CSE");
    students[0].batch = 'Q';
    students[0].sem = 5;
    sprintf(students[0].subjects[0].name,"Computer Networks");
    students[0].subjects[0].marks = 93;
    sprintf(students[0].subjects[1].name, "Java Programming");
    students[0].subjects[1].marks = 95;
    sprintf(students[0].subjects[2].name,"Compiler Design");
    students[0].subjects[2].marks = 88;
    sprintf(students[0].subjects[3].name, "OOAD");
    students[0].subjects[3].marks = 82;
    sprintf(students[0].subjects[4].name,"Linear Algebra");
    students[0].subjects[4].marks = 93;
    
    // Student 2
    sprintf(students[1].name,"Barathraj T");
    students[1].regNum = 2019103511;
    sprintf(students[1].dept, "CSE");
    students[1].batch = 'P';
    students[1].sem = 5;
    sprintf(students[1].subjects[0].name,"Computer Networks");
    students[1].subjects[0].marks = 93;
    sprintf(students[1].subjects[1].name, "Java Programming");
    students[1].subjects[1].marks = 95;
    sprintf(students[1].subjects[2].name,"Compiler Design");
    students[1].subjects[2].marks = 80;
    sprintf(students[1].subjects[3].name, "OOAD");
    students[1].subjects[3].marks = 95;
    sprintf(students[1].subjects[4].name,"Linear Algebra");
    students[1].subjects[4].marks = 93;
    
    // Student 3
    sprintf(students[2].name,"Ishwarya Rani M");
    students[2].regNum = 2019103527;
    sprintf(students[2].dept, "CSE");
    students[2].batch = 'Q';
    students[2].sem = 5;
    sprintf(students[2].subjects[0].name,"Computer Networks");
    students[2].subjects[0].marks = 90;
    sprintf(students[2].subjects[1].name, "Java Programming");
    students[2].subjects[1].marks = 92;
    sprintf(students[2].subjects[2].name,"Compiler Design");
    students[2].subjects[2].marks = 82;
    sprintf(students[2].subjects[3].name, "OOAD");
    students[2].subjects[3].marks = 85;
    sprintf(students[2].subjects[4].name,"Linear Algebra");
    students[2].subjects[4].marks = 97;
    
    // Student 4
    sprintf(students[3].name,"Navvya L");
    students[3].regNum = 2019103548;
    sprintf(students[3].dept, "CSE");
    students[3].batch = 'Q';
    students[3].sem = 5;
    sprintf(students[3].subjects[0].name,"Computer Networks");
    students[3].subjects[0].marks = 84;
    sprintf(students[3].subjects[1].name, "Java Programming");
    students[3].subjects[1].marks = 92;
    sprintf(students[3].subjects[2].name,"Compiler Design");
    students[3].subjects[2].marks = 81;
    sprintf(students[3].subjects[3].name, "OOAD");
    students[3].subjects[3].marks = 87;
    sprintf(students[3].subjects[4].name,"Linear Algebra");
    students[3].subjects[4].marks = 92;
    
    // Student 5
    sprintf(students[4].name,"Shafeequr Rahman P A");
    students[4].regNum = 2019103511;
    sprintf(students[4].dept, "CSE");
    students[4].batch = 'P';
    students[4].sem = 5;
    sprintf(students[4].subjects[0].name,"Computer Networks");
    students[4].subjects[0].marks = 87;
    sprintf(students[4].subjects[1].name, "Java Programming");
    students[4].subjects[1].marks = 90;
    sprintf(students[4].subjects[2].name,"Compiler Design");
    students[4].subjects[2].marks = 80;
    sprintf(students[4].subjects[3].name, "OOAD");
    students[4].subjects[3].marks = 95;
    sprintf(students[4].subjects[4].name,"Linear Algebra");
    students[4].subjects[4].marks = 89;
    
}