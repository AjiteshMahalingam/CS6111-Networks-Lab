#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<dirent.h>
#include<fcntl.h>

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
int authorizeClient(int clientSocket){
    struct FTP_Command command;
    struct FTP_Reply reply;
    bzero(&command, sizeof(command));
    bzero(&reply, sizeof(reply));
    
    read(clientSocket, &command, sizeof(command)); // USER username
    printf("C > %s %s\n", command.command, command.args);
    
    char user[100];
    char pwd[100];
    user[0] = pwd[0] = '\0';
    int flag = 0;

    FILE *userFP = fopen("users.txt", "r");
    char buffer[100];
    while(fgets(buffer, 100, userFP) != NULL){
        char *token = strtok(buffer, ",");
        if(strncmp(command.args, token, strlen(token)) == 0){
            sprintf(user, "%s", token);
            token = strtok(NULL, ",");
            sprintf(pwd, "%s", token);
            flag = 1;
            break;
        }
    }
    if(flag){
        reply.reply = 331;
        sprintf(reply.msg, "Username OK, password required");
        printf("S > %d %s\n", reply.reply, reply.msg);
        write(clientSocket, &reply, sizeof(reply));
    } else {
        reply.reply = 430;
        sprintf(reply.msg, "Invalid username or password ");
        printf("S > %d %s\n", reply.reply, reply.msg);
        write(clientSocket, &reply, sizeof(reply));
        return 0;
    }

    read(clientSocket, &command, sizeof(command)); // PASS password
    printf("C > %s %s\n", command.command, command.args);

    if(strncmp(command.args, pwd, 4) == 0){
        reply.reply = 230;
        sprintf(reply.msg, "User logged in");
        printf("S > %d %s\n", reply.reply, reply.msg);
        write(clientSocket, &reply, sizeof(reply));
    } else {
        reply.reply = 430;
        sprintf(reply.msg, "Invalid username or password ");
        printf("S > %d %s\n", reply.reply, reply.msg);
        write(clientSocket, &reply, sizeof(reply));
        return 0;
    }
    return 1;
}

int establishDataConn(){
    int dataSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(dataSocket < 0){
        printf("[-] Unable to create server data socket.\n");
        return dataSocket;
    }

    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(DATA_PORT);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(connect(dataSocket, (struct sockaddr*)&clientAddr, sizeof(clientAddr)) < 0){
        printf("[-] Unable to connect to client data socket.\n");
        return -1;
    }
    printf("\n[+] Data connection established with client at port %d\n", DATA_PORT);
    return dataSocket;
}

void retreiveHandler(int clientSocket, int dataSocket, struct FTP_Command command){
    DIR *dp = opendir("./");
    struct dirent* dirp;
    struct FTP_Reply reply;
    char fname[50];
    int i;
    for(i = 0; i < strlen(command.args); i++){
        if(command.args[i] == '.')
            break;
        fname[i] = command.args[i];
    }
    fname[i] = '\0';
    strcat(fname, ".txt");
    int flag = 0;
    size_t len = strlen(fname) > strlen("users.txt") ? strlen("users.txt") : strlen(fname);
    if(strncmp("users.txt", fname, len) == 0){
        reply.reply = 553;
        sprintf(reply.msg, "Can't open that file: Permission denied");
    } else {
        while((dirp = readdir(dp)) != 0){
            if(strcmp(dirp->d_name, fname) == 0){
                flag = 1;
                break;
            } 
        }
        if(flag){
            FILE *fp = fopen(fname, "r");
            if(fp){
                char c;
                while(1){
                    c = fgetc(fp);
                    if(write(dataSocket, &c, sizeof(c)) < 0){
                            printf("[-] Error writing file.\n");
                            break;
                    }
                    if(c == EOF)
                        break;
                }
                reply.reply = 226;
                sprintf(reply.msg,"Transfer complete, closing data connection");
            } else {
                reply.reply = 450;
                sprintf(reply.msg,"Requested file action not taken, File unavailable");
            }
            fclose(fp);
        } else {
            reply.reply = 450;
            sprintf(reply.msg,"Requested file action not taken, File unavailable");
        }
    }
    printf("S > %d %s\n", reply.reply, reply.msg);
    write(clientSocket, &reply, sizeof(reply));
}

void storeHandler(int clientSocket, int dataSocket, struct FTP_Command command){
    struct FTP_Reply reply;
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
    if(fp){
        char c;
        while(c != EOF){
            int n = read(dataSocket, &c, sizeof(c));
            if(n <= 0)
                break;
            fputc(c, fp);
        }
        reply.reply = 226;
        sprintf(reply.msg,"Transfer complete, closing data connection");
    } else {
        reply.reply = 450;
        sprintf(reply.msg,"Requested file action not taken, File unavailable");
    }    
    printf("S > %d %s\n", reply.reply, reply.msg);
    write(clientSocket, &reply, sizeof(reply));
    fclose(fp);
}

void listHandler(int clientSocket, int dataSocket, struct FTP_Command command){
    DIR *dp = opendir("./");
    struct dirent *dirp;
    char buffer[256];
    while((dirp = readdir(dp)) != 0){
        if(strstr(dirp->d_name, ".txt") != NULL){
            sprintf(buffer, "%s", dirp->d_name);
            if(strlen(buffer) > 3)
                write(dataSocket, buffer, sizeof(buffer));
        }
    }
    write(dataSocket, "X", sizeof("X"));
    struct FTP_Reply reply;
    reply.reply = 226;
    sprintf(reply.msg,"Transfer complete, closing data connection"); 
    printf("S > %d %s\n", reply.reply, reply.msg);
    write(clientSocket, &reply, sizeof(reply));
}

int main(){
    int serverControlSocket, serverDataSocket, clientSocket;
    struct sockaddr_in serverControlAddr, clientDataAddr;
    
    serverControlSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(serverControlSocket < 0){
        printf("[-] Server control socket creation failed.\n");
        exit(1);
    }

    int opt = 1;
    if(setsockopt(serverControlSocket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0){
        printf("[-] Control socket setsockopt() failed.\n");
        exit(1);
    }

    bzero(&serverControlAddr, sizeof(serverControlAddr));
    serverControlAddr.sin_family = AF_INET; 
    serverControlAddr.sin_port = htons(CONTROL_PORT);
    serverControlAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(serverControlSocket, (struct sockaddr*)&serverControlAddr, sizeof(serverControlAddr)) < 0){
        printf("[-] Server control socket bind failed.\n");
        exit(1);
    }

    if(listen(serverControlSocket, 5) < 0){
        printf("[-] Server control socket listen failed.\n");
        exit(1);
    }
    printf("[+] Server control socket set-up and running at port %d.\n", CONTROL_PORT);

    while(1){
        clientSocket = accept(serverControlSocket, NULL, NULL);
        if(clientSocket < 0){
            printf("[-] Unable to accept client control request.\n");
            break;
        }
        printf("[+] New client connected.\n");
        int isAuthorized = authorizeClient(clientSocket);
        struct FTP_Command command;
        struct FTP_Reply reply;
        if(isAuthorized){
            while(1){
                bzero(&command, sizeof(command));
                bzero(&reply, sizeof(reply));
                read(clientSocket, &command, sizeof(command));
                printf("C > %s %s\n", command.command, command.args);
                if(strncmp(command.command, "QUIT", 4) == 0){
                    reply.reply = 221;
                    sprintf(reply.msg,"Service closing control connection");
                    printf("S > %d %s\n", reply.reply, reply.msg);
                    write(clientSocket, &reply, sizeof(reply));
                    printf("[+] Client disconnected.\n");
                    break;
                } else {
                    reply.reply = 200;
                    sprintf(reply.msg,"Command OK");
                    printf("S > %d %s\n", reply.reply, reply.msg);
                    write(clientSocket, &reply, sizeof(reply));
                    reply.reply = 150;
                    sprintf(reply.msg,"Opening ASCII mode data connection");
                    printf("S > %d %s\n", reply.reply, reply.msg);
                    write(clientSocket, &reply, sizeof(reply));

                    serverDataSocket = establishDataConn();
                    if(serverDataSocket < 0){
                        break;
                    }
                    if(strncmp(command.command, "RETR", 4) == 0)
                        retreiveHandler(clientSocket, serverDataSocket, command);
                    else if(strncmp(command.command, "STOR", 4) == 0)
                        storeHandler(clientSocket, serverDataSocket, command);
                    else if(strncmp(command.command, "LIST", 4) == 0)
                        listHandler(clientSocket, serverDataSocket, command);
                    close(serverDataSocket);
                    printf("\n[+] Data connection closed\n");
                }
            }
        }
    }
    close(serverControlSocket);
    close(serverDataSocket);
    return 0;
}