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

int main(){
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
    struct Student student;
    
    printf("Enter roll number : ");
    scanf("%ld", &rollNum);
    rollNum = htonl(rollNum);
    write(clientSocket, &rollNum, sizeof(rollNum));
    
    read(clientSocket, &student, sizeof(student));
    if(student.regNum == -1)
        printf("Enter a valid reg.no.\n");
    else{
        printf("------------------------\n");
        printf("Student details : \n");
        printf("------------------------\n");
        printf("Name : %s\n", student.name);
        printf("Reg. No : %ld\n", student.regNum);
        printf("Dept : %s - %c batch\n", student.dept, student.batch);
        printf("------------------------\n");
        printf("Semester : %d\n", student.sem);
        for(int i = 0; i < 5; i++){
            printf("%s - %d/100\n", student.subjects[i].name, student.subjects[i].marks);
        }
    }
    
    close(clientSocket);
    return 0;
}
