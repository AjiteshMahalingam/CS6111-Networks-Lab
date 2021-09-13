#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 3503
#define MAX 1024

int client_socket;

void *receiveMessage(void *client_socket)
{
    int socket = *((int *)client_socket);
    char buffer[MAX];
    while (1)
    {
        buffer[0] = '\0';
        int len = recv(socket, buffer, sizeof(buffer), 0);
        buffer[len] = '\0';
        if (strlen(buffer) > 0)
        {
            if (strncmp(buffer, "TERM", 4) == 0)
                break;
            printf("%s \n", buffer);
        }
    }
    return NULL;
}
int main()
{
    pthread_t client_thread;

    struct sockaddr_in serverAddr;
    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_socket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("[-] Connection request failed.\n");
        exit(1);
    }
    printf("[+] Connected to the server.\n");

    pthread_create(&client_thread, NULL, receiveMessage, (void *)&client_socket);

    char buffer[MAX];
    while (1)
    {
        buffer[0] = '\0';
        int n = 0;
        while ((buffer[n++] = getchar()) != '\n')
            ;
        buffer[n] = '\0';
        if (strlen(buffer) > 0)
        {
            send(client_socket, buffer, strlen(buffer), 0);
            if (strncmp(buffer, "EXIT", 4) == 0)
            {
                printf("[+] Exiting..\n");
                break;
            }
        }
    }
    pthread_join(client_thread, NULL);
    close(client_socket);
    return 0;
}