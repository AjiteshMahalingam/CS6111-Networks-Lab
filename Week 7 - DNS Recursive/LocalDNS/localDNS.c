#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define ROOTDNS 35034
#define LOCALDNS 35030

int main(){
    int socketfd;
    struct sockaddr_in hostAddr;
    
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd < 0){
        fprintf(stderr, "[-] Socket creation failed.\n");
        return -1;
    }
    printf("[+] Socket created.\n");

    bzero(&hostAddr, sizeof(hostAddr));
    hostAddr.sin_family = AF_INET;
    hostAddr.sin_port = htons(LOCALDNS);
    hostAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(socketfd, (struct sockaddr*)&hostAddr, sizeof(hostAddr)) < 0){
        fprintf(stderr, "[-] Socket bind failed.\n");
        return -1;
    }

    int recvBytes, sentBytes;
    char buffer[1024], reqip[1024];
    int rootfd;
    struct sockaddr_in clientAddr, rootAddr;
    int len = sizeof(clientAddr);

    while(1){
        recvBytes = recvfrom(socketfd, buffer, 1024, 0, (struct sockaddr*)&clientAddr, &len);
        if(strncmp(buffer, "Exit", 4) == 0)
            continue;
        printf(">> From client : %s\n", buffer);
        printf("[+] Requesting Root DNS..\n");

        //rootfd = socket(AF_INET, SOCK_DGRAM, 0);
        // if(rootfd < 0){
        //     fprintf(stderr, "[-] Unable to create root socket.\n");
        //     return -1;
        // }
        bzero(&rootAddr, sizeof(rootAddr));
        rootAddr.sin_family = AF_INET;
        rootAddr.sin_port = htons(ROOTDNS);
        rootAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        sentBytes = sendto(socketfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&rootAddr, len);
        recvBytes = recvfrom(socketfd, reqip, 1024, 0, NULL, NULL);

        printf(">> Requested IP (from Root DNS) : %s\n", reqip);
        printf("[+] Returning to client..\n");

        // close(rootfd);
        sentBytes = sendto(socketfd, reqip, strlen(reqip) + 1, 0, (struct sockaddr*)&clientAddr, len);
    }

    printf("[+] Client disconnected and closing.\n");
    close(socketfd);
    return 0;
}