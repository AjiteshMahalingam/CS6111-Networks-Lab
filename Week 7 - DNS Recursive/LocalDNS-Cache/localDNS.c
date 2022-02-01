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

        FILE *fd = fopen("cache.txt", "r");
        if(!fd){
            fprintf(stderr, "[-] Couldn't access cache.\n");
            continue;
        }
        char lineBuffer[1024], *hostname, *IP;
        char fileBuffer[10*1024];
        fileBuffer[0] = '\0';
        int isPresent = 0;
        while(fgets(lineBuffer, 1024, fd)){
            hostname = strtok(lineBuffer, " ");
            if(strncmp(hostname, buffer, strlen(hostname)) == 0){
                isPresent = 1;
                IP = strtok(NULL, "\n");
                break;
            }
            lineBuffer[0] ='\0';
        }
        fclose(fd);
        if(isPresent){
            printf("[+] Requested IP present in cache.\n");
            printf(">> Requested IP (from Local cache) : %s\n", IP);
            printf("[+] Returning to client..\n");
            sentBytes = sendto(socketfd, IP, strlen(IP) + 1, 0, (struct sockaddr*)&clientAddr, len);
        } else {
            printf("[+] Requested IP not present in cache.\n");
            printf("[+] Requesting Root DNS..\n");

            rootfd = socket(AF_INET, SOCK_DGRAM, 0);
            if(rootfd < 0){
                fprintf(stderr, "[-] Unable to create root socket.\n");
                return -1;
            }
            bzero(&rootAddr, sizeof(rootAddr));
            rootAddr.sin_family = AF_INET;
            rootAddr.sin_port = htons(ROOTDNS);
            rootAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

            sentBytes = sendto(rootfd, buffer, strlen(buffer)+1, 0, (struct sockaddr*)&rootAddr, len);
            recvBytes = recvfrom(rootfd, reqip, 1024, 0, NULL, NULL);

            printf(">> Requested IP (from Root DNS) : %s\n", reqip);
            printf("[+] Returning to client..\n");

            close(rootfd);
            sentBytes = sendto(socketfd, reqip, strlen(reqip) + 1, 0, (struct sockaddr*)&clientAddr, len);
            if(!(strncmp(reqip, "404", strlen("404")) == 0)){
                FILE *fd = fopen("cache.txt", "a");
                if(!fd){
                    fprintf(stderr, "[-] Couldn't access cache.\n");
                    continue;
                }
                char dnsRecord[3*1024];
                sprintf(dnsRecord, "%s %s\n", buffer, reqip);
                fputs(dnsRecord, fd);
                fclose(fd);
                printf("[+] DNS Record stored in cache.\n");
            }

        }
    }

    printf("[+] Client disconnected and closing.\n");
    close(socketfd);
    return 0;
}