#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

#define PORT 3503
#define MAX 100
#define W_SIZE 4
#define SEQ_MAX 7

struct Packet {
    int srcPort;
    int destPort;
    int seqNum;
    int ackFlag;
    int dropFlag;
    char data[MAX];
};

struct Packet buffer[MAX];
int next_ack = 0;
int next_recv = 0;
int send_comp = 0;

void storePacket(struct Packet packet, int nextExp){
    // Packet arrived in-order, store it in the buffer
    if(packet.seqNum <= (nextExp + W_SIZE -1 )){
        int index = packet.seqNum;
        buffer[index] = packet;
        printf(">> Packet %d stored in buffer at %d\n", packet.seqNum, index);
    }
    // Packet arrived out-of-order or duplicate packet, discard
    else {
        printf(">> Packet out-of-order or duplicate. Discard.\n");
    }
}

int main() {
    int recvSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(recvSocket < 0){
        printf("[-] Unable to create socket.\n");
        exit(1);
    }

    struct sockaddr_in senderAddr;
    bzero(&senderAddr, sizeof(senderAddr));
    senderAddr.sin_family = AF_INET;
    senderAddr.sin_port = htons(PORT);
    senderAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(connect(recvSocket, (struct sockaddr*)&senderAddr, sizeof(senderAddr)) < 0){
        printf("[-] Unable to connect to the sender.\n");
        exit(1);
    }
    printf("[+] Connected to the sender\n");

    int lastRecv = 0;
    int nextExp = 0;
    while(1) {
        // Recv next packet
        struct Packet packet;
        read(recvSocket, &packet, sizeof(packet));
        printf(">> Packet %d received.\n", packet.seqNum);
        
        // Create ack packet
        struct Packet ackPkt;
        ackPkt.srcPort = packet.destPort;
        ackPkt.destPort = PORT;
        ackPkt.seqNum = packet.seqNum;
        
        printf(">> Drop ACK %d ? ", ackPkt.seqNum);
        scanf("%d", &(ackPkt.dropFlag));

        // If ack dropped
        if(ackPkt.dropFlag) {
            printf(">> ACK lost.\n");
            ackPkt.ackFlag = 1;
            write(recvSocket, &ackPkt, sizeof(ackPkt));
        }
        // If packet not dropped
        if(packet.dropFlag == 0){
            ackPkt.ackFlag = 1;
            if(!ackPkt.dropFlag){
                printf(">> ACK %d sent.\n", ackPkt.seqNum);
                write(recvSocket, &ackPkt, sizeof(ackPkt));
            }
            if(ackPkt.seqNum > lastRecv)
                lastRecv = ackPkt.seqNum;
            if(nextExp == packet.seqNum){
                storePacket(packet, nextExp);
                nextExp = lastRecv + 1;
            }
            else
                storePacket(packet, nextExp);
        }
        // If packet dropped
        else {
            ackPkt.ackFlag = -1;
            if(!ackPkt.dropFlag){
                printf(">> NACK %d sent.\n", ackPkt.seqNum);
                write(recvSocket, &ackPkt, sizeof(ackPkt));
            }
        }

    }


    return 0;
}