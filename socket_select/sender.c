#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUF 3

int main(int argc, char** argv)
{
    int len;
    int sockfd;
    int port = 55555;
    struct sockaddr_in address; // changed
    char buf[MAX_BUF] = {'C', 'A', 'B'};

    if (argc>=2) {
        int p = atoi(argv[1]);
        if (p>0 && p<=65535) {
            port = p;
        } else {
            fprintf(stderr, "Invalid port number=%d\n", p);
            return -1;
        }
    }

    // create a internet socket
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror(argv[0]);
        return -1;
    }

    // target
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(port);

    printf("start send data to port %d\n", port);
    while(1) {
        int send_len = (rand() % MAX_BUF) + 1;
        sendto(sockfd, buf, send_len, 0, (struct sockaddr*)&address, sizeof(address));
        printf("\nsend %d bytes\n", send_len);
        usleep(500*1000);
    }

    printf("sender end\n");
    close(sockfd);
    return 0;
}
