#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 3503
#define MAX 1024

int main()
{
    int clientSocket;
    struct sockaddr_in servAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("[-] Unable to connect to server.\n");
        exit(1);
    }
    printf("[+] Connected to server.\n");

    while (1)
    {
        char buffer[MAX];
        int n = 0;
        bzero(buffer, MAX);
        printf("Enter number 1 : ");
        while ((buffer[n++] = getchar()) != '\n')
            ;
        write(clientSocket, buffer, MAX);
        if (strncmp(buffer, "X", 1) == 0)
        {
            printf("[+] Disconnected from server.\n");
            break;
        }
        n = 0;
        bzero(buffer, MAX);
        printf("Enter number 2 : ");
        while ((buffer[n++] = getchar()) != '\n')
            ;
        write(clientSocket, buffer, MAX);
        n = 0;
        bzero(buffer, MAX);
        printf("Enter operation : ");
        while ((buffer[n++] = getchar()) != '\n')
            ;
        write(clientSocket, buffer, MAX);

        bzero(buffer, MAX);
        read(clientSocket, buffer, MAX);
        printf("%s\n", buffer);
    }

    close(clientSocket);
    return 0;
}