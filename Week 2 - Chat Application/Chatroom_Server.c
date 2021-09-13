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
#define MAX 256
#define N 5

int client_sockets[N];

void *manageClients(void *client_socket)
{
    int c_socket = *((int *)client_socket);
    int cur_index = 0;
    char buffer[2 * MAX];
    char msg[MAX];
    for (int i = 0; i < N; i++)
    {
        if (client_sockets[i] == c_socket)
        {
            cur_index = i;
            break;
        }
    }
    printf("[+] New client %d connected.\n", cur_index);
    char *welcome = "Welcome to chatroom!";
    send(c_socket, welcome, strlen(welcome), 0);
    while (1)
    {
        msg[0] = '\0';
        int len = recv(c_socket, msg, sizeof(msg), 0);
        msg[len] = '\0';
        if (strlen(msg) > 0)
        {
            printf("From client %d : %s\n", cur_index, msg);
            if (strncmp(msg, "EXIT", 4) == 0)
            {
                printf("[+] Client %d disconnected.\n", cur_index);
                char *term = "TERM";
                send(c_socket, term, strlen(term), 0);
                close(c_socket);
                client_sockets[cur_index] = 0;
                return NULL;
            }
            else {
                for (int i = 0; i < N; i++)
                {
                    if (client_sockets[i] != 0)
                    {
                        buffer[0] = '\0';
                        if (i == cur_index)
                        {
                            sprintf(buffer, "SENT : %s", msg);
                            send(client_sockets[i], buffer, strlen(buffer), 0);
                            continue;
                        }
                        else
                        {
                            sprintf(buffer, "From client %d : %s", cur_index, msg);
                            send(client_sockets[i], buffer, strlen(buffer), 0);
                            printf("Sent to client %d.\n", i);
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

int main()
{
    pthread_t client_threads[N];
    int serverSocket;
    struct sockaddr_in serverAddr;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        printf("[-] Socket creation failed.\n");
        exit(1);
    }
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        printf("[-] Socket set option failed.\n");
        exit(1);
    }
    bzero(&serverAddr, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        printf("[-] Socket bind failed.\n");
        exit(1);
    }
    if (listen(serverSocket, N) < 0)
    {
        printf("[-] Socket listen failed.\n");
        exit(1);
    }
    printf("[+] Server running and listening on port %d.\n", PORT);
    for (int i = 0; i <= N; i++)
        client_sockets[i] = 0;

    int index = N;
    while (1)
    {
        for (int i = 0; i < N; i++)
        {
            if (client_sockets[i] == 0)
            {
                index = i;
                break;
            }
        }
        if (index == N)
            continue;
        client_sockets[index] = accept(serverSocket, NULL, NULL);

        if (client_sockets[index] < 0)
        {
            printf("[-] Unable to accept client request.\n");
            break;
        }
        pthread_create(&client_threads[index], NULL, manageClients, (void *)&client_sockets[index]);
    }
    for (int i = 0; i < N; i++)
        pthread_join(client_threads[i], NULL);
    close(serverSocket);
    return 0;
}