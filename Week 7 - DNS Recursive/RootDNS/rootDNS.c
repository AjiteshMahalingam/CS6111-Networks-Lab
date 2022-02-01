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
#define TLDDNS 35032

int main(){
    int socketfd;
    struct sockaddr_in rootAddr;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd < 0){
        fprintf(stderr, "[-] Unable to create socket.\n");
        return -1;
    }
    printf("[+] Socket created.\n");

    bzero(&rootAddr, sizeof(rootAddr));
    rootAddr.sin_family = AF_INET;
    rootAddr.sin_port = htons(ROOTDNS);
    rootAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(socketfd, (struct sockaddr*)&rootAddr, sizeof(rootAddr)) < 0){
        fprintf(stderr, "[-] Unable to bind the socket.\n");
        return -1;
    }

    int sentBytes, recvBytes;
    struct sockaddr_in localAddr;
    char buffer[1024], reqIP[1024];
    int len = sizeof(localAddr);
    while(1){
        recvBytes = recvfrom(socketfd, buffer, 1024, 0, (struct sockaddr*)&localAddr, &len);
        printf(">> Request from client (local DNS) : %s\n", buffer);

        char domain[6];
        int i = 0, j = 0;
        while(buffer[i++] != '.');
        while(buffer[i++] != '.');
        while(buffer[i] != '\0')
            domain[j++] = buffer[i++];
        domain[j] = '\0';

        char lineBuffer[1024], *reqDomain, *IP;
        int isPresent = 0;
        FILE *fd = fopen("root.txt", "r");
        if(!fd){
            fprintf(stderr, "[-] Unable to access DNS Records.\n");
            sendto(socketfd, "ERROR", strlen("ERROR") + 1, 0, (struct sockaddr*)&localAddr, len);
            continue;
        } 
        while(fgets(lineBuffer, 1024, fd)){
            reqDomain = strtok(lineBuffer, " ");
            if(strncmp(reqDomain, domain, strlen(reqDomain)) == 0){
                isPresent = 1;
                IP = strtok(NULL, "\n");
                break;
            }
        }
        fclose(fd);
        if(!isPresent){
            sendto(socketfd, "404", strlen("404") + 1, 0, (struct sockaddr*)&localAddr, len);
            printf("[+] Returning to Local DNS..\n");
            continue;
        }
        printf(">> IP for TLD DNS of %s - %s\n", domain, IP);
        printf("[+] Requesting TLD DNS...\n");

        // int tldfd = socket(AF_INET, SOCK_DGRAM, 0);
        // if(tldfd < 0){
        //     fprintf(stderr, "[-] Unable to create TLD socket.\n");
        //     return -1;
        // }
        struct sockaddr_in tldAddr;
        bzero(&tldAddr, sizeof(tldAddr));
        tldAddr.sin_family = AF_INET;
        tldAddr.sin_port = htons(TLDDNS);
        tldAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        sentBytes = sendto(socketfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&tldAddr, len);
        recvBytes = recvfrom(socketfd, reqIP, 1024, 0, NULL, NULL);
        printf(">> Requested IP (From TLD DNS) - %s\n", reqIP);
        //close(tldfd);
        printf("[+] Returning to Local DNS..\n");
        sendto(socketfd, reqIP, strlen(reqIP)+1, 0, (struct sockaddr*)&localAddr, len);
    }
    close(socketfd);
    return 0;
}