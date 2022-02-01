#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define AUTHDNS 35036

int main(){
    int socketfd;
    struct sockaddr_in authAddr;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd < 0){
        fprintf(stderr, "[-] Unable to create socket.\n");
        return -1;
    }
    bzero(&authAddr, sizeof(authAddr));
    printf("[+] Socket created.\n");

    authAddr.sin_family = AF_INET;
    authAddr.sin_port = htons(AUTHDNS);
    authAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(socketfd, (struct sockaddr*)&authAddr, sizeof(authAddr)) < 0){
        fprintf(stderr, "[-] Unable to bind socket.\n");
        return -1;
    }

    int sentBytes, recvBytes;
    struct sockaddr_in tldAddr;
    int len = sizeof(tldAddr);
    char buffer[1024], reqIP[1024];
    while(1){
        recvBytes = recvfrom(socketfd, buffer, 1024, 0, (struct sockaddr*)&tldAddr, &len);
        printf(">> From TLD DNS : %s\n", buffer);

        FILE *fd = fopen("auth.txt", "r");
        if(!fd){
            fprintf(stderr, "[-] Unable to access DNS Records.\n");
            sendto(socketfd, "ERROR", strlen("ERROR")+1, 0, (struct sockaddr*)&tldAddr, len);
            continue;
        }
        char lineBuffer[1024], *reqDomain, *IP;
        int isPresent = 0;
        while(fgets(lineBuffer, 1024, fd)){
            reqDomain = strtok(lineBuffer, " ");
            if(strncmp(reqDomain, buffer, strlen(reqDomain)) == 0){
                isPresent = 1;
                IP = strtok(NULL, "\n");
                break;
            }
        }
        fclose(fd);

        if(!isPresent){
            printf(">> IP for %s - Not found\n", buffer);
            sendto(socketfd, "404", strlen("404") + 1, 0, (struct sockaddr*)&tldAddr, len);
            printf("[+] Returning to TLD DNS...\n");
            continue;
        }

        printf(">> IP for %s - %s\n", buffer, IP);
        sendto(socketfd, IP, strlen(IP)+1, 0, (struct sockaddr*)&tldAddr, len);
        printf("[+] Returning to TLD DNS...\n");
    }

    close(socketfd);
    return 0;
}