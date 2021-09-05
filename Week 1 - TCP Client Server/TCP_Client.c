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

int main()
{
    int sockfd;
    struct sockaddr_in addr;

    // Socket Creation
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        exit(1);
    }
    else
    {
        printf("Socket created.\n");
    }

    // Server socket connection
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Server connection failed.\n");
        exit(1);
    }
    else
    {
        printf("Server connected.\n");
    }

    // Read message from server
    char buffer[MAX];
    int readcnt = read(sockfd, buffer, MAX);
    if (readcnt > 0)
    {
        printf("From Server : %s\n", buffer);
    }
    else
    {
        printf("Couldn't read from server.\n");
    }

    return 0;
}