#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define MAX_IFACE_NUM 10

int main(int argc, char** argv)
{
    int i;
    int ret;
    int iface_count;
    int sockfd;
    struct ifconf ifc;
    struct ifreq ifr[MAX_IFACE_NUM];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd == -1) {
        perror(argv[0]);
        return -1;
    }

    printf("default ifc_len=%ld\n", sizeof(ifr));
    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_req = ifr;
    ret = ioctl(sockfd, SIOCGIFCONF, &ifc);
    if (ret < 0) {
        close(sockfd);
        perror(argv[0]);
        return -1;
    }

    printf("got actural ifc_len=%d\n", ifc.ifc_len);
    iface_count = ifc.ifc_len/sizeof(struct ifreq);
    for (i=0; i<iface_count; ++i)
    {
        struct sockaddr_in sin;
        struct ifreq *req = &ifr[i];
        memcpy(&sin, &ifr[i].ifr_addr, sizeof(sin));
        printf("INETFACE[%d] name: %10s addr: %s\n", i, req->ifr_name, inet_ntoa(sin.sin_addr));
    }

    close(sockfd);

    return 0;
}
