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
int next_send = 0;
int next_write = 0;
int last_ack=0;

void createPacket (int seqNo, struct sockaddr_in recvAddr) {
    // Create a new packet and fill the details
    struct Packet p;
    p.srcPort = PORT;
    p.destPort = recvAddr.sin_port;
    p.seqNum = seqNo;
    p.ackFlag = 0;
    sprintf(p.data, "TCP packet with data");
    printf(">> Want to drop packet %d ? ", seqNo);
    scanf("%d", &(p.dropFlag));
    // Store it in the sender's buffer
    buffer[next_write] = p;
    next_write++;    
}

// Selective reject protocol
void retransmit(int socket, int seq) {
    struct Packet packet= buffer[seq];
    packet.dropFlag = 0;
    write(socket, &packet, sizeof(packet));
    printf(">> Retransmitted packet %d\n", packet.seqNum);
}

int main() {
    int senderSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(senderSocket < 0){
        printf("[-] Unable to create socket.\n");
        exit(1);
    }

    struct sockaddr_in senderAddr;
    bzero(&senderAddr, sizeof(senderAddr));
    senderAddr.sin_family = AF_INET;
    senderAddr.sin_port = htons(PORT);
    senderAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(senderSocket, (struct sockaddr*)&senderAddr, sizeof(senderAddr)) < 0){
        printf("[-] Socket bind failed.\n");
        exit(1);
    }

    if(listen(senderSocket, 5) != 0) {
        printf("[-] Socekt listen failed.\n");
        exit(1);
    }
    printf("[+] Sender socket created and listening at port %d\n", PORT);

    struct sockaddr_in recvAddr;
    int len = sizeof(recvAddr);
    int recvSocket = accept(senderSocket, (struct sockaddr*)&recvAddr, &len);
    if(recvSocket < 0) {
        printf("[-] Unable to accept receiver connection request.\n");
        exit(1);
    }

    // No of packets to send
    int n;
    printf(">> Enter number of packets to send : ");
    scanf("%d", &n);

    // Create packets and store it in the buffer
    int seqNum = 0;
    int send_comp = 0;
    for(int i = 0; i < n; i++){
        createPacket(seqNum, recvAddr);
        printf(">> Packet %d written in buffer.\n", seqNum);
        seqNum = (seqNum + 1) % SEQ_MAX;
    }

    // Send the packets to fill pipe
    for(int i = 0; i < W_SIZE; i++){
        struct Packet packet = buffer[next_send];
        next_send++;
        write(recvSocket, &packet, sizeof(packet));
        printf(">> Packet %d sent\n", packet.seqNum);
    }
    printf(">> Pipe full, waiting for acks.\n");

    int w_size = 0;
    int sent = 0;

    while(1) {
        // Receive ack
        struct Packet ackPkt;
        read(recvSocket, &ackPkt, sizeof(ackPkt));

        // Ack dropped
        if(ackPkt.ackFlag == 1 && ackPkt.dropFlag == 1) {
            printf(">> ACK Lost.\n");
            retransmit(recvSocket, next_ack);
            sent = 1;
            bzero(&ackPkt, sizeof(ackPkt));
            read(recvSocket, &ackPkt, sizeof(ackPkt));
        }
        // Negative ACK recved, trigger retransmission
        if(ackPkt.ackFlag == -1) {
            printf(">> Packet %d lost, start Selective-reject retransmission.\n", ackPkt.seqNum);
            retransmit(recvSocket, ackPkt.seqNum);
            sent = 1;
            bzero(&ackPkt, sizeof(ackPkt));
            read(recvSocket, &ackPkt, sizeof(ackPkt));
        }
        // Else ack recved and next_ack incremented
        printf(">> ACK received for %d\n", ackPkt.seqNum);
        w_size++;

        if(ackPkt.seqNum > last_ack)
            last_ack = ackPkt.seqNum;
        if(next_ack == ackPkt.seqNum) {
            next_ack = last_ack + 1;
            sent = 0;
        }

        // Send the next packet
        if(next_send < n && !sent) {
            while(w_size--){
                struct Packet nextPkt = buffer[next_send];
                next_send++;
                printf(">> Sending next packet. ");
                write(recvSocket, &nextPkt, sizeof(nextPkt));
                printf("Packet %d sent.\n", nextPkt.seqNum);
            }
        } else if (next_send == n && !send_comp) {
            printf(">> All packets sent.\n");
            send_comp = 1;
        }
    }
    return 0;
}