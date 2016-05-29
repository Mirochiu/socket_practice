#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUF_SIZE 7*188

int main(int argc, char** argv)
{
    int i;
    int len;
    int port = 11111;
    int sockfd;
    int addrlen;
    struct sockaddr_in address;
    char buf[BUF_SIZE];

    if (argc>=2) {
        int p = atoi(argv[1]);
        if (p>0 && p<=65535) {
            port = p;
        } else {
            fprintf(stderr, "Invalid port number=%d\n", p);
            return -1;
        }
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror(argv[0]);
        return -1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *)(&address), sizeof(address)) < 0) {
        perror(argv[0]);
        close(sockfd);
        return -1;
    }

    printf("start recving port %d...\n", port);
    while(1) {
        len = recvfrom(sockfd, buf, sizeof(buf), 0, (struct sockaddr *)&address, &addrlen);
        if (len<=0) {
            if (len<0) {
                perror(argv[0]);
                break;
            }
            printf("no data, sleep 30ms");
            usleep(30*1000);
            continue;
        }

        printf("recv %d byte data:", len);
        for (i=0; i<len; ++i)
            printf("%c ", buf[i]);
        printf("\n");
    }

    printf("reciver end\n");
    close(sockfd);
    return 0;
}

