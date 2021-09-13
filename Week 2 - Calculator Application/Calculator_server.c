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

int client_sockets[3];

int calculate(int a, int b, char op)
{
    switch (op)
    {
    case '+':
        return a + b;
    case '-':
        return a - b;
    case '*':
        return a * b;
    case '/':
        return a / b;
    }
}

void *calculator(void *cl_socket)
{
    //pthread_detach(pthread_self());
    int clientSocket = *((int *)cl_socket);
    char buffer[MAX];
    int idx = 0;
    for (int i = 0; i <= 2; i++)
    {
        if (client_sockets[i] == clientSocket)
        {
            idx = i;
            break;
        }
    }
    while (1)
    {
        bzero(buffer, MAX);
        int len = read(clientSocket, buffer, MAX);
        if (strncmp(buffer, "X", 1) == 0)
        {
            close(client_sockets[idx]);
            client_sockets[idx] = 0;
            break;
        }
        buffer[len] = '\0';
        int op1 = atoi(buffer);

        bzero(buffer, MAX);
        len = read(clientSocket, buffer, MAX);
        buffer[len] = '\0';
        int op2 = atoi(buffer);

        bzero(buffer, MAX);
        len = read(clientSocket, buffer, MAX);
        char operator= buffer[0];

        int result = calculate(op1, op2, operator);

        bzero(buffer, MAX);
        sprintf(buffer, "Result : %d", result);
        write(clientSocket, buffer, MAX);
    }
    printf("[+] Client %d disconnected.\n", idx);
    //pthread_exit(NULL);
}

int main()
{
    pthread_t client_threads[2];

    int serverSocket;
    struct sockaddr_in servAddr;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    printf("[+] Socket created.\n");
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        printf("[-] Unable to set socket options.\n");
        exit(1);
    }

    for (int i = 0; i <= 2; i++)
        client_sockets[i] = 0;

    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (bind(serverSocket, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        printf("[-] Socket unable to bind.\n");
        exit(1);
    }

    if (listen(serverSocket, 5) < 0)
    {
        printf("[-] Socket unable to listen.\n");
        exit(1);
    }
    printf("[+] Server listening at port %d\n", PORT);

    int idx = 3;
    while (1)
    {
        for (int i = 0; i <= 2; i++)
        {
            if (client_sockets[i] == 0)
            {
                idx = i;
                break;
            }
        }

        client_sockets[idx] = accept(serverSocket, NULL, NULL);
        if (client_sockets[idx] < 0)
        {
            printf("[-] Unable to accept client request.\n");
            exit(1);
        }

        pthread_create(&client_threads[idx], NULL, calculator, (void *)&client_sockets[idx]);
    }
    for (int i = 0; i < 2; i++)
        pthread_join(client_threads[i], NULL);
    close(serverSocket);
    return 0;
}
