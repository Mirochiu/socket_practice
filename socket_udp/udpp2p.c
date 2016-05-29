#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>


#define LOGD printf

int get_socketaddr_by_url(char* url, struct sockaddr_in* addr, int def_type) {
    char *prefix = NULL;
    char *slash = NULL;
    char *semi = NULL;
    char host[128] = "localhost";
    char port[6] = {0};
    char service[16] = {0};
    struct addrinfo hints;
    struct addrinfo* info = NULL;
    if (!url || !addr) return -1;

    // copy url prefix as inet service name
    prefix = strstr(url, "://");
    if (prefix) {
        strncpy(service, url, prefix-url);
        LOGD("Prefix %s\n", service);
        prefix += 3;
    }
    else {
        prefix = url;
    }

    // find port number
    slash = strchr(prefix, '/');
    semi  = strchr(prefix, ':');
    if (semi) {
        if (!slash) {
            strncpy(host, prefix, semi-prefix);
            strncpy(port, semi+1, sizeof(port)-1);
        }
        else {
            if (semi < slash) {
                strncpy(host, prefix, semi-prefix);
                strncpy(port, semi+1, sizeof(port)-1);
            } else {
                strncpy(host, prefix, slash-prefix);
            }
        }
    }
    else { // no port number assigned
        if (slash) {
            strncpy(host, prefix, slash-prefix);
        } else {
            strncpy(host, prefix, sizeof(host)-1);
        }
    }
    LOGD("host=%s port=%d\n", host, atoi(port));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = def_type;
    hints.ai_flags = AI_PASSIVE;
    if (0 > getaddrinfo(host, (strlen(port)?port:service), &hints, &info)) {
        return -1;
    }

    struct sockaddr_in* i = (struct sockaddr_in*)(info->ai_addr);
    LOGD("host ip:%s\n", inet_ntoa(i->sin_addr));
    memcpy(addr, i, sizeof(*addr));
    freeaddrinfo(info);
    return 0;
}
/*
http://man7.org/linux/man-pages/man7/ip.7.html
           struct sockaddr_in {
               sa_family_t    sin_family;
               in_port_t      sin_port;
               struct in_addr sin_addr;
           };

http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
          struct addrinfo {
               int              ai_flags;
               int              ai_family;
               int              ai_socktype;
               int              ai_protocol;
               socklen_t        ai_addrlen;
               struct sockaddr *ai_addr;
               char            *ai_canonname;
               struct addrinfo *ai_next;
           };
*/
#define MAX_BUF_SIZE 7*188

int main(int argc, char** argv)
{
    int len;
    int result;
    char inurl [64] = {0};
    char outurl[64] = {0};
    char buf[MAX_BUF_SIZE];
    int infd, outfd;
    struct sockaddr_in inaddr, outaddr;

    if (argc<=2) {
        printf("usage: udpp2p <in-url> <out-url>\n");
        return -1;
    }

    strncpy(inurl,  argv[1], sizeof(inurl)-1);
    strncpy(outurl, argv[2], sizeof(outurl)-1);

    if(0 > get_socketaddr_by_url(inurl, &inaddr, SOCK_DGRAM)) {
        fprintf(stderr, "get_socketaddr_by url error\n");
        return -1;
    }

    if (0 > get_socketaddr_by_url(outurl, &outaddr, SOCK_DGRAM)) {
        fprintf(stderr, "get_socketaddr_by url error\n");
        return -1;
    }

    infd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (infd < 0) {
        perror(argv[0]);
        return -1;
    }

    if (bind(infd, (struct sockaddr *)(&inaddr), sizeof(inaddr)) < 0) {
        perror(argv[0]);
        close(infd);
        return -1;
    }

    outfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (outfd < 0) {
        perror(argv[0]);
        return -1;
    }

    printf("waiting ....\n");
    while (1) {
        int addrlen;
        int recvlen = recvfrom(infd, buf, MAX_BUF_SIZE, 0, (struct sockaddr*)(&inaddr), &addrlen);
        if (recvlen <= 0) {
            if (recvlen < 0) {
                perror(argv[0]);
                break;
            }
            printf("read nothing\n");
            usleep(50*1000);
            continue;
        }
        printf("%d bytes from server, send to another server\n", recvlen);
        sendto(outfd, buf, recvlen, 0, (struct sockaddr*)(&outaddr), sizeof(outaddr));
    }

    close(infd);
    close(outfd);
    return 0;
}

