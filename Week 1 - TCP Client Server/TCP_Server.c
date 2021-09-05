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

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        exit(1);
    }
    else
    {
        printf("Socket created\n");
    }

    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        printf("Socket bind failed\n");
        exit(1);
    }
    else
    {
        printf("Socket binded.\n");
    }

    if (listen(sockfd, 5) != 0)
    {
        printf("Listen failed\n");
        exit(1);
    }
    else
    {
        printf("Socket listening \n");
    }

    int newsockfd;
    struct sockaddr_in clientaddr;
    int len = sizeof(clientaddr);
    newsockfd = accept(sockfd, (struct sockaddr *)&clientaddr, &len);

    char *msg = "Welcome";
    int wcnt = write(newsockfd, msg, strlen(msg));

    close(newsockfd);
}
