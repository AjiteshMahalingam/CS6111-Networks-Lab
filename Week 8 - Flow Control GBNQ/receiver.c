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

void storePacket(struct Packet packet){
    // Packet arrived in-order, store it in the buffer
    if(packet.seqNum == (next_recv % SEQ_MAX)){
        buffer[next_recv] = packet;
        printf(">> Packet %d stored in buffer at %d\n", packet.seqNum, next_recv);
        next_recv++;
    }
    // Termination packet
    else if (packet.seqNum == -1){
        buffer[next_recv] = packet;
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

    // Recv and store window full packets whether drop or not
    for(int i = 0; i < W_SIZE; i++){
        struct Packet packet;
        read(recvSocket, &packet, sizeof(packet));
        printf(">> Packet %d received.\n", packet.seqNum);
        storePacket(packet);
    }

    int retransmit = 0;
    while(1) {
        // Next packet from buffer
        struct Packet packet = buffer[next_ack];
        // Create ack packet
        struct Packet ackPkt;
        ackPkt.srcPort = packet.destPort;
        ackPkt.destPort = PORT;
        ackPkt.seqNum = packet.seqNum;
        ackPkt.dropFlag = 0;

        // If packet not dropped, send ACK and inc next_ack
        if(packet.dropFlag == 0){
            ackPkt.ackFlag = 1;
            printf(">> ACK %d sent.\n", ackPkt.seqNum);
            write(recvSocket, &ackPkt, sizeof(ackPkt));
            next_ack++;
        }
        // If packet dropped, send NACK
        else {
            ackPkt.ackFlag = -1;
            printf(">> NACK %d sent.\n", ackPkt.seqNum);
            write(recvSocket, &ackPkt, sizeof(ackPkt));
            retransmit = 1;
        }

        // Recv next packet
        struct Packet nextPkt;
        read(recvSocket, &nextPkt, sizeof(nextPkt));
        printf(">> Packet %d received.\n", nextPkt.seqNum);
        
        // Packet dropped -> retransmit
        if(retransmit){
            if(nextPkt.seqNum != -1){
                printf(">> %d retransmit update.\n", nextPkt.seqNum);
                // Replacing the new packet
                buffer[next_ack] = nextPkt; 
                retransmit = 0;
            } else {
                next_ack++;
                storePacket(nextPkt);
            }
        } else {
            storePacket(nextPkt);
        }
    }

    return 0;
}