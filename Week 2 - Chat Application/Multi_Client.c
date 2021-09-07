#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0)
    {
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("[-] Socket connection failed.\n");
        exit(1);
    }
    printf("[+] Socket connected.\n");

    char buffer[1024];

    while (1)
    {
        int n = 0;
        bzero(buffer, 1024);
        printf("Client : ");
        while ((buffer[n++] = getchar()) != '\n')
            ;
        write(clientSocket, buffer, 1024);
        if (strncmp(buffer, "exit", 4) == 0)
        {
            printf("[+] Exiting.\n");
            close(clientSocket);
            exit(1);
        }

        bzero(buffer, 1024);
        if (read(clientSocket, buffer, 1024) < 0)
        {
            printf("[-] Unable to read from server.\n");
            exit(1);
        }
        else
        {
            printf("Server : %s", buffer);
        }
    }
    return 0;
}