#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>

#define PORT 3503
#define O_PORT 6503
#define MAX 1024
#define N 10

struct Subject{
    char name[40];
    int grade;
};

struct Student{
    char name[30];
    long regNum;
    char dept[10];
    char batch;
    int sem;
    struct Subject subjects[5];
};

struct Student cache[N];

struct Request{
    char requestLine[MAX];
    char requestHeaders[MAX];
    char requestBody[MAX];
};

struct Response{
    char statusLine[MAX];
    char responseHeaders[MAX];
    char responseTime[MAX];
    struct Student responseBody;
};

void initCache(){
    for(int i = 0; i < N; i++){
        bzero(&cache[i], sizeof(cache[i]));
        cache[i].regNum = -1;
    }
}

int findInCache(int sem){
    for(int i = 0; i < N; i++){
        if(cache[i].regNum != -1 && cache[i].sem == sem)
            return i;
    }
    return -1;
}

void storeInCache(struct Student student){
    for(int i = 0; i < N; i++){
        if(cache[i].regNum == -1){
            cache[i] = student;
            break;
        }
    }
}

int main(){
    initCache();
    int proxySocket;
    struct sockaddr_in proxyAddr;

    proxySocket = socket(AF_INET, SOCK_STREAM, 0);
    if(proxySocket < 0){
        printf("[-] Socket creation failed.\n");
        exit(1);
    }    

    bzero(&proxyAddr, sizeof(proxyAddr));
    proxyAddr.sin_family = AF_INET;
    proxyAddr.sin_port = htons(PORT);
    proxyAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(proxySocket, (struct sockaddr*)&proxyAddr, sizeof(proxyAddr)) < 0){
        printf("[-] Socket bind failed.\n");
        exit(1);
    }

    if(listen(proxySocket, 5) < 0){
        printf("[-] Socket listen failed.\n");
        exit(1);
    }
    printf("[+] Proxy server set-up and running at port %d\n", PORT);

    int clientSocket;
    while(1){
        clientSocket = accept(proxySocket, NULL, NULL);
        if(clientSocket < 0){
            printf("[-] Client request not accepted.\n");
            exit(1);
        }
        printf("[+] New Client connected.\n");
        while(1){
            long rollNum;
            int sem;
            struct Request request;
            if(read(clientSocket, &request, sizeof(request)) > 0){
                printf("\n%s%s%s\n", request.requestLine, request.requestHeaders, request.requestBody);
                if(strncmp(request.requestLine, "EXIT", 4) == 0){
                    printf("[+] Client disconnected.\n");
                    close(clientSocket);
                    break;
                }
                char *reqMsg = strtok(request.requestBody, ",");
                rollNum = strtol(reqMsg, NULL, 10);
                reqMsg = strtok(NULL, ",");
                sem= strtol(reqMsg, NULL, 10);
                int cacheIdx = findInCache(sem);
                if(cacheIdx != -1){
                    printf("[+] Client requested for for reg.no %ld - Semester %d and present in cache.\n", rollNum, sem);
                    struct Response response;
                    time_t currTime;
                    time(&currTime);
                    struct Student student = cache[cacheIdx];
                    response.responseBody = cache[cacheIdx];
                    sprintf(response.statusLine, "HTTP/1.1 200 OK\n");
                    sprintf(response.responseTime, "Date : %s", ctime(&currTime));
                    sprintf(response.responseHeaders, "Connection: keep-alive\nContent-Type: text/plain\nContent-Length: %ld\n", sizeof(response.responseBody));
                    write(clientSocket, &response, sizeof(response));
                    printf("[+] Client request completed and connection kept alive.\n");
                } else {
                    printf("[+] Client requested for for reg.no %ld - Semester %d and not present in cache. Requesting origin server.\n", rollNum, sem);
                    
                    int originSocket = socket(AF_INET, SOCK_STREAM, 0);
                    struct sockaddr_in originAddr;
                    bzero(&originAddr, sizeof(originAddr));
                    originAddr.sin_family = AF_INET;
                    originAddr.sin_port = htons(O_PORT);
                    originAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
                    if(connect(originSocket, (struct sockaddr*)&originAddr, sizeof(originAddr)) < 0){
                        printf("[-] Unable to connect to the origin server.\n");
                        exit(1);
                    }
                    printf("[+] Connected to the origin server.\n");
                    
                    struct Request cacheRequest;
                    sprintf(cacheRequest.requestLine, "GET /%ld/%d HTTP/1.1\n", rollNum, sem);
                    sprintf(cacheRequest.requestHeaders, "Host: localhost\nUser-Agent: Ubuntu/20.4\nAccept: text/html\nAccept-Language: en-US\nConnection: close\n\n");
                    sprintf(cacheRequest.requestBody, "%ld,%d", rollNum, sem);
                    write(originSocket, &cacheRequest, sizeof(cacheRequest));

                    struct Response cacheResponse;
                    read(originSocket, &cacheResponse, sizeof(cacheResponse));
                    printf("[+] Requested object received from origin server.\n");
                    storeInCache(cacheResponse.responseBody);

                    struct Response response;
                    time_t currTime;
                    time(&currTime);
                    struct Student student = cacheResponse.responseBody;
                    response.responseBody = cacheResponse.responseBody;
                    sprintf(response.statusLine, "HTTP/1.1 200 OK\n");
                    sprintf(response.responseTime, "Date : %s", ctime(&currTime));
                    sprintf(response.responseHeaders, "Connection: keep-alive\nContent-Type: text/plain\nContent-Length: %ld\n", sizeof(response.responseBody));
                    write(clientSocket, &response, sizeof(response));
                    printf("[+] Client request completed and connection kept alive.\n");
                }
            }
        }
    }
    close(proxySocket);
    return 0;
}
