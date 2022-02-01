#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define LOCALDNS 35030

int main(){
    int clientSocket;
    struct sockaddr_in dnsAddr;

    clientSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if(clientSocket < 0){
        fprintf(stderr, "[-] Socket creation failed.\n");
        return -1;
    }
    printf("[+] Socket created.\n");
    bzero(&dnsAddr, sizeof(dnsAddr));
    dnsAddr.sin_family = AF_INET;
    dnsAddr.sin_port = htons(LOCALDNS);
    dnsAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    int recvBytes, sentBytes;
    char input[30], buffer[1024];

    while(1){
        printf(">> Enter the hostname : ");
        fgets(input, 30, stdin);
        int i = 0;
        while(input[i] !='\0'){
            if(input[i] == '\n'){
                input[i] = '\0';
                break;
            }
            i++;
        }
        sentBytes = sendto(clientSocket, input, strlen(input)+1, 0, (struct sockaddr*)&dnsAddr, sizeof(dnsAddr));
        if(strncmp(input, "Exit", 4) == 0)
            break;
        recvBytes = recvfrom(clientSocket, buffer, 1024, 0, NULL, NULL);
        printf(">> IP of %s - %s\n", input, buffer);
    }

    close(clientSocket);
    return 0;
}
