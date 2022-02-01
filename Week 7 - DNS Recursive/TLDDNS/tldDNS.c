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
#define TLDDNS 35032

int main(){
    int socketfd;
    struct sockaddr_in tldAddr;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(socketfd < 0){
        fprintf(stderr, "[-] Unable to create socket.\n");
        return -1;
    }
    printf("[+] Socket created.\n");

    bzero(&tldAddr, sizeof(tldAddr));
    tldAddr.sin_family = AF_INET;
    tldAddr.sin_port = htons(TLDDNS);
    tldAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(socketfd, (struct sockaddr*)&tldAddr, sizeof(tldAddr)) < 0){
        fprintf(stderr, "[-] Unable to bind socket.\n");
        return -1;
    }

    int recvBytes, sentBytes;
    struct sockaddr_in rootAdr;
    char buffer[1024], reqIP[1024];
    int len = sizeof(rootAdr);
    while(1){
        recvBytes = recvfrom(socketfd, buffer, 1024, 0, (struct sockaddr*)&rootAdr, &len);
        printf(">> From Root DNS : %s\n", buffer);

        char domain[30];
        int i = 0, j = 0;

        while(buffer[i++] != '.');
        while(buffer[i] != '\0')
            domain[j++] = buffer[i++];
        domain[j] = '\0';

        FILE *fd = fopen("tld.txt", "r");
        if(!fd){
            fprintf(stderr, "[-] Unable to access DNS Records.\n");
            sendto(socketfd, "ERROR", strlen("ERROR")+1, 0, (struct sockaddr*)&rootAdr, len);
            continue;
        }
        char lineBuffer[1024], *reqDomain, *IP;
        int isPresent;
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
            sendto(socketfd, "404", strlen("404") + 1, 0, (struct sockaddr*)&rootAdr, len);
            printf("[+] Returning to Root DNS..\n");
            continue;
        }
        printf(">> IP for Authoritative DNS of %s - %s\n", domain, IP);
        printf("[+] Requesting Auth. DNS...\n");

        // int authfd = socket(AF_INET, SOCK_DGRAM, 0);
        // if(authfd < 0){
        //     fprintf(stderr, "[-] Unable to create Auth socket.\n");
        //     return -1;
        // }
        struct sockaddr_in authAddr;
        bzero(&authAddr, sizeof(authAddr));
        authAddr.sin_family = AF_INET;
        authAddr.sin_port = htons(AUTHDNS);
        authAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

        sentBytes = sendto(socketfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&authAddr, len);
        recvBytes = recvfrom(socketfd, reqIP, 1024, 0, NULL, NULL);
        printf(">> Requested IP (From Auth DNS) - %s\n", reqIP);
        // close(authfd);
        printf("[+] Returning to Root DNS..\n");
        sendto(socketfd, reqIP, strlen(reqIP)+1, 0, (struct sockaddr*)&rootAdr, len);
    }

    close(socketfd);
    return 0;
}