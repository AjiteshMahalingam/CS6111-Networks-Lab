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
    int serverSocket;
    struct sockaddr_in servAddr;

    int newSocket;
    struct sockaddr_in newAddr;

    char buffer[1024];
    pid_t childpid;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("[-] Socket binding failed.\n");
        exit(1);
    }
    printf("[+] Socket binded to port %d.\n", PORT);

    if (listen(serverSocket, 10) != 0)
    {
        printf("[-] Listen failed.\n");
        exit(1);
    }
    printf("[+] Listening ... \n");

    while (1)
    {
        int len = sizeof(newAddr);
        newSocket = accept(serverSocket, (struct sockaddr *)&newAddr, &len);
        if (newSocket < 0)
        {
            printf("[-] Unable to accept client request.\n");
            exit(1);
        }
        printf("[+] Connected to client from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
        if ((childpid = fork()) == 0)
        {
            while (1)
            {
                bzero(buffer, 1024);
                read(newSocket, buffer, 1024);
                if (strncmp(buffer, "exit", 4) == 0)
                {
                    printf("[+] Disconnected client from %s:%d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    break;
                }
                printf("Client %s:%d : %s", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port), buffer);
                bzero(buffer, 1024);
                int n = 0;
                printf("Server to client %s:%d : ", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                while ((buffer[n++] = getchar()) != '\n')
                    ;
                write(newSocket, buffer, 1024);
            }
            close(newSocket);
        }
    }
    close(serverSocket);

    return 0;
}