#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PORT 5000

int main()
{
    int serverSocket;
    struct sockaddr_in servAddr;

    int newSocket, addrlen;
    struct sockaddr_in newAddr;
    int clientSockets[10];
    int maxClients = 10;
    int sd, maxsd;
    int i;

    char *msg = "Welcome to the Chatroom.\n";
    char buffer[1024];
    int n = 0;

    fd_set readfds; // Set of socket descriptors

    for (i = 0; i < maxClients; i++)
        clientSockets[i] = 0;

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
        printf("[-] Socket bind failed.\n");
        exit(1);
    }
    printf("[+] Socket binded.\n");

    if (listen(serverSocket, 10) < 0)
    {
        printf("[-] Socket listen failed.\n");
        exit(1);
    }
    printf("[+] Listening at port %d.\n", PORT);

    addrlen = sizeof(newAddr);

    while (1)
    {
        FD_ZERO(&readfds);              // Clear the socket set
        FD_SET(serverSocket, &readfds); // Add server socket to set
        maxsd = serverSocket;
        // Add child sockets
        for (i = 0; i < maxClients; i++)
        {
            sd = clientSockets[i];
            //if valid socket descriptor then add to read list
            if (sd > 0)
                FD_SET(sd, &readfds);
            //highest file descriptor number, need it for the select function
            if (sd > maxsd)
                maxsd = sd;
        }
        // wait for an activity on one of the sockets , timeout is NULL => wait indefinitely
        int activity = select(maxsd + 1, &readfds, NULL, NULL, NULL);
        if (activity < 0)
            printf("[-] Select() error.\n");

        //If something happened on the master socket, then its an incoming connection
        if (FD_ISSET(serverSocket, &readfds))
        {
            if ((newSocket = accept(serverSocket, (struct sockaddr *)&newAddr, &addrlen)) < 0)
            {
                printf("[-] Client request unable to accept.\n");
                exit(1);
            }
            printf("[+] New client connected %d - %s:%d\n", newSocket, inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
            if (write(newSocket, msg, strlen(msg)) != strlen(msg))
            {
                printf("[-] Welcome message not sent.\n");
            }
            else
            {
                printf("[+] Welcome message sent successfully\n");
            }

            // Add new socket to array of sockets
            for (i = 0; i < maxClients; i++)
            {
                if (clientSockets[i] == 0)
                {
                    clientSockets[i] = newSocket;
                    printf("[+] Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // else it's some I/O operation on some other socket
        for (i = 0; i < maxClients; i++)
        {
            sd = clientSockets[i];
            if (FD_ISSET(sd, &readfds))
            {
                // Check if it was for closing, and also read the incoming message
                bzero(buffer, 1024);
                int readcnt = read(sd, buffer, 1024);
                if (readcnt == 0)
                {
                    //Somebody disconnected , get his details and print
                    getpeername(sd, (struct sockaddr *)&newAddr, &addrlen);
                    printf("[+] Client Disconnected %s : %d\n", inet_ntoa(newAddr.sin_addr), ntohs(newAddr.sin_port));
                    // Close the socket
                    close(sd);
                    clientSockets[i] = 0;
                }
                else
                {
                    write(sd, buffer, 1024);
                }
            }
        }
    }
    return 0;
}