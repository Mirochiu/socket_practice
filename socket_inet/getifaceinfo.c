#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <string.h>

#define INETFACE_NAME "eth0"

// http://lang.idv.tw/doku.php/program/c/linux%E7%B7%A8%E7%A8%8B%E7%8D%B2%E5%8F%96%E6%9C%AC%E6%A9%9Fip%E5%9C%B0%E5%9D%80

int main(int argc, char** argv) {
    int      sockfd;
    int      ret;
    int      i;
    struct   sockaddr_in *sin;
    struct   ifreq ifr_ip;
    char     ipaddr[64];
    char     netmask[64];
    char     broadcast[64];
    unsigned char macaddr[64];
    char     ifacename[64] = INETFACE_NAME;

    if (argc > 1) {
        strncpy(ifacename, argv[1], sizeof(ifacename)-1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror(argv[0]);
        return -1;
    }

    // http://man7.org/linux/man-pages/man7/netdevice.7.html

    // ip
    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, ifacename, sizeof(ifr_ip.ifr_name) - 1);
    ret = ioctl(sockfd, SIOCGIFADDR, &ifr_ip);
    if (ret < 0) {
        perror(argv[0]);
    } else {
        sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;
        strncpy(ipaddr, inet_ntoa(sin->sin_addr), sizeof(ipaddr)-1);
        printf("%s ip:%s\n", ifacename, ipaddr);
    }

    // netmask
    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, ifacename, sizeof(ifr_ip.ifr_name) - 1);
    ret = ioctl(sockfd, SIOCGIFNETMASK, &ifr_ip);
    if (ret < 0) {
        perror(argv[0]);
    } else {
        sin = (struct sockaddr_in *)&ifr_ip.ifr_netmask;
        strncpy(netmask, inet_ntoa(sin->sin_addr), sizeof(netmask)-1);
        printf("%s netmask:%s\n", ifacename, netmask);
    }

    // broadcast ip
    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, ifacename, sizeof(ifr_ip.ifr_name) - 1);
    ret = ioctl(sockfd, SIOCGIFBRDADDR, &ifr_ip);
    if (ret < 0) {
        perror(argv[0]);
    } else {
        sin = (struct sockaddr_in *)&ifr_ip.ifr_netmask;
        strncpy(broadcast, inet_ntoa(sin->sin_addr), sizeof(broadcast)-1);
        printf("%s broadcast:%s\n", ifacename, broadcast);
    }

    // hwaddr
    memset(&ifr_ip, 0, sizeof(ifr_ip));
    strncpy(ifr_ip.ifr_name, ifacename, sizeof(ifr_ip.ifr_name) - 1);
    ret = ioctl(sockfd, SIOCGIFHWADDR, &ifr_ip);
    if (ret < 0) {
        perror(argv[0]);
    } else {
        memcpy(macaddr, ifr_ip.ifr_hwaddr.sa_data, 6);
        printf("%s mac:%02X:%02X:%02X:%02X:%02X:%02X\n", ifacename,
            macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
    }

    close(sockfd);

    return 0;
}

