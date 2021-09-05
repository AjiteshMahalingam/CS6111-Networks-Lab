#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000
#define MAX 2048

void chat(int sockfd)
{
    char buffer[MAX];
    int n;
    while (1)
    {
        bzero(buffer, MAX);
        n = 0;
        printf("From Client : ");
        while ((buffer[n++] = getchar()) != '\n')
            ;
        write(sockfd, buffer, MAX);

        if (strncmp("Exit", buffer, 4) == 0)
        {
            printf("Server Exit.\n");
            break;
        }

        bzero(buffer, MAX);
        int readcnt = read(sockfd, buffer, MAX);
        if (readcnt == 0)
        {
            printf("Server disconnected.\n");
            break;
        }
        printf("From Server : %s\n", buffer);
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in addr;

    // Create()
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed.\n");
        exit(1);
    }
    else
    {
        printf("Socket created.\n");
    }

    // Connect()
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Unable to connect to Server.\n");
        exit(1);
    }
    else
    {
        printf("Connected to server.\n");
    }

    // Chat()
    chat(sockfd);

    // Close()
    close(sockfd);
}