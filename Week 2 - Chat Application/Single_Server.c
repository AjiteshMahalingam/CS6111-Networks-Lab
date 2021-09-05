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
    // Infinite loop for chat
    while (sockfd > 0)
    {
        // Read from client
        bzero(buffer, MAX);
        int readcnt = read(sockfd, buffer, MAX);
        if (readcnt == 0)
        {
            printf("Client disconnected.\n");
            break;
        }
        printf("From Client : %s\n", buffer);

        // Exit the chat
        if (strncmp("Exit", buffer, 4) == 0)
        {
            printf("Server Exit.\n");
            break;
        }

        // Write to client
        bzero(buffer, MAX);
        n = 0;
        printf("From Server : ");
        while ((buffer[n++] = getchar()) != '\n')
            ;
        write(sockfd, buffer, MAX);
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in addr;

    // Create socket
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

    // Bind()
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Socket binding failed.\n");
        exit(1);
    }
    else
    {
        printf("Socket binding.\n");
    }

    // Listen()
    if (listen(sockfd, 5) != 0)
    {
        printf("Socket listen failed.\n");
        exit(1);
    }
    else
    {
        printf("Socket listening at port %d.\n", PORT);
    }

    // Accept()
    int newsockfd;
    struct sockaddr_in claddr;
    int len = sizeof(claddr);
    newsockfd = accept(sockfd, (struct sockaddr *)&claddr, &len);
    if (newsockfd < 0)
    {
        printf("Client request failed.\n");
        exit(1);
    }
    else
    {
        printf("Client connected.\n");
    }

    // Chat() - function for chat between server - client
    chat(newsockfd);

    // Close();
    close(newsockfd);
}