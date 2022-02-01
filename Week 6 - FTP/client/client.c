#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define DATA_PORT 35030
#define CONTROL_PORT 35031

struct FTP_Command{
    char command[100];
    char args[100];
};

struct FTP_Reply{
    int reply;
    char msg[100];
};

int setDataSocket(){
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(dataSocket < 0){
        printf("[-] Data socket creation failed.\n");
        exit(1);
    }
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(DATA_PORT);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(bind(dataSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0){
        printf("[-] Data socket bind failed.\n");
        exit(1);
    }
    if(listen(dataSocket, 5) < 0){
        printf("[-] Socket listen failed.\n");
        exit(1);
    }
    printf("[+] Client Data socket set-up and running at port %d.\n", DATA_PORT);
    return dataSocket;
}

void listHandler(int controlSocket, int dataSocket, struct FTP_Command command){
    struct FTP_Reply reply;
    write(controlSocket, &command, sizeof(command)); // LIST
    read(controlSocket, &reply, sizeof(reply)); // 200 Command ok
    printf("S > %d  %s\n",reply.reply, reply.msg);
    read(controlSocket, &reply, sizeof(reply)); // 150 Opening ASCII mode data connection.
    printf("S > %d  %s\n",reply.reply, reply.msg);
    
    int serverSocket = accept(dataSocket, NULL, NULL);
    if(serverSocket < 0){
        printf("[-] Unable to accept server request.\n");
        exit(1);
    } 

    printf("\n");
    char buffer[256];
    while(1){
        read(serverSocket, buffer, 256);
        if(strncmp(buffer, "X", sizeof("X")) == 0)
            break;
        printf("%s\n", buffer);
        //break;
    }
    printf("\n");            
     
    read(controlSocket, &reply, sizeof(reply)); // 226 Transfer Complete.
    printf("S > %d  %s\n",reply.reply, reply.msg);
    close(serverSocket);
}

void retrHandler(int controlSocket, int dataSocket, struct FTP_Command command){
    struct FTP_Reply reply;
    write(controlSocket, &command, sizeof(command)); // RETR filename
    read(controlSocket, &reply, sizeof(reply)); // 200 Command ok
    printf("S > %d  %s\n",reply.reply, reply.msg);
    read(controlSocket, &reply, sizeof(reply)); // 150 Opening ASCII mode data connection.
    printf("S > %d  %s\n",reply.reply, reply.msg);
    
    int serverSocket = accept(dataSocket, NULL, NULL);
    if(serverSocket < 0){
        printf("[-] Unable to accept server request.\n");
        exit(1);
    } 

    char fname[50];
    int i;
    for(i = 0; i < strlen(command.args); i++){
        if(command.args[i] == '.')
            break;
        fname[i] = command.args[i];
    }
    fname[i] = '\0';
    strcat(fname, ".txt");

    FILE *fp = fopen(fname, "w");
    char c;
    while(c != EOF){
        int n = read(serverSocket, &c, sizeof(c));
        if(n <= 0)
            break;
        fputc(c, fp);
    }
    fclose(fp);
    
    read(controlSocket, &reply, sizeof(reply)); // 226 Transfer Complete.
    printf("S > %d  %s\n",reply.reply, reply.msg);
    close(serverSocket);
}

void storHandler(int controlSocket, int dataSocket, struct FTP_Command command){
    struct FTP_Reply reply;
    write(controlSocket, &command, sizeof(command)); // STOR filename
    read(controlSocket, &reply, sizeof(reply)); // 200 Command ok
    printf("S > %d  %s\n",reply.reply, reply.msg);
    read(controlSocket, &reply, sizeof(reply)); // 150 Opening ASCII mode data connection.
    printf("S > %d  %s\n",reply.reply, reply.msg);
    
    int serverSocket = accept(dataSocket, NULL, NULL);
    if(serverSocket < 0){
        printf("[-] Unable to accept server request.\n");
        exit(1);
    } 

    char fname[50];
    int i;
    for(i = 0; i < strlen(command.args); i++){
        if(command.args[i] == '.')
            break;
        fname[i] = command.args[i];
    }
    fname[i] = '\0';
    strcat(fname, ".txt");

    FILE *fp = fopen(fname, "r");
    char c;
    while(1){
        c = fgetc(fp);
        if(write(serverSocket, &c, sizeof(c)) < 0){
                printf("[-] Error writing file.\n");
                break;
        }
        if(c == EOF)
            break;
    }
    fclose(fp);
    
    read(controlSocket, &reply, sizeof(reply)); // 226 Transfer Complete.
    printf("S > %d  %s\n",reply.reply, reply.msg);
    close(serverSocket);
}

int main(){
    int clientControlSocket, clientDataSocket;
    struct sockaddr_in serverControlAddr;

    clientControlSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(clientControlSocket < 0){
        printf("[-] Control socket create failed.\n");
        exit(1);
    }

    bzero(&serverControlAddr, sizeof(serverControlAddr));
    serverControlAddr.sin_family = AF_INET;
    serverControlAddr.sin_port = htons(CONTROL_PORT);
    serverControlAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    if(connect(clientControlSocket, (struct sockaddr*)&serverControlAddr, sizeof(serverControlAddr)) < 0){
        printf("[-] Unable to connect server control socket.\n");
        exit(1);
    }
    printf("[+] Connected to the server control socket.\n");
    printf("[+] Enter commands.\n");

    clientDataSocket = setDataSocket();

    struct FTP_Command command;
    struct FTP_Reply reply;

    printf("C > ");
    scanf("%s", command.command);
    scanf("%s", command.args);
    write(clientControlSocket, &command, sizeof(command)); // USER username
    read(clientControlSocket, &reply, sizeof(reply)); // 331 Username ok. Password required
    printf("S > %d %s.\n", reply.reply, reply.msg);
    if(reply.reply == 331) {
        bzero(&command, sizeof(command));
        bzero(&reply, sizeof(reply));
        
        printf("C > ");
        scanf("%s", command.command);
        scanf("%s", command.args);
        write(clientControlSocket, &command, sizeof(command)); // PASS password
        
        read(clientControlSocket, &reply, sizeof(reply)); // 230 User logged in
        printf("S > %d %s.\n", reply.reply, reply.msg);

        if(reply.reply == 230){
            while(1){
                bzero(&command, sizeof(command));
                bzero(&reply, sizeof(reply));
                
                printf("C > ");
                scanf("%s", command.command);

                if(strncmp(command.command, "LIST", 4) == 0){
                    sprintf(command.args, " ");
                    listHandler(clientControlSocket, clientDataSocket, command);
                } else if(strncmp(command.command, "RETR", 4) == 0){
                    scanf("%s", command.args);
                    retrHandler(clientControlSocket, clientDataSocket, command);
                } else if(strncmp(command.command, "STOR", 4) == 0){
                    scanf("%s", command.args);    
                    storHandler(clientControlSocket, clientDataSocket, command);
                } else if(strncmp(command.command, "QUIT", 4) == 0){
                    write(clientControlSocket, &command, sizeof(command)); // QUIT
                    read(clientControlSocket, &reply, sizeof(reply)); // 221 Service closing control connection
                    printf("S > %d  %s\n",reply.reply, reply.msg);
                    break;
                } else {
                    printf("[+] Enter valid command.\n");
                } 
            }                    

        } else {
            printf("[+] User not authorized.\n");
            printf("[+] Exiting.");
        }
    } else {
        printf("[+] User not authorized.\n");
        printf("[+] Exiting.\n");
    }

    close(clientDataSocket);
    close(clientControlSocket);
    return 0;
}