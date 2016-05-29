#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv)
{
    int ret;
    char *host;
    char **names;
    char **addrs;
    struct hostent *hostinfo;
    struct addrinfo hints;
    struct addrinfo *remoteinfo;

    if (argc == 1) {
        char myname[256] = {0};
        ret = gethostname(myname, sizeof(myname));
        if (ret == -1) {
            fprintf(stderr, "cannot gethostname\n");
            perror(argv[0]);
            return -1;
        }
        host = myname;
    }
    else {
        host = argv[1];
    }

    hostinfo = gethostbyname(host);
    if (!hostinfo) {
        fprintf(stderr, "cannot get info for host: %s\n", host);
        return -1;
    }

    printf("+gethostbyname\n");
    printf("Name: '%s'\n", hostinfo->h_name);
    printf("Aliases:\n");
    names = hostinfo->h_aliases;
    while (*names) {
        printf(" %s\n", *names);
        names++;
    }

    if (hostinfo->h_addrtype != AF_INET) {
        fprintf(stderr, "not an IP host!\n");
        return -1;
    }

    printf("IPs:\n");
    addrs = hostinfo->h_addr_list;
    while (*addrs) {
        printf(" %s\n", inet_ntoa(*(struct in_addr*)*addrs));
        addrs++;
    }

    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = AF_INET;
    hints.ai_family   = SOCK_DGRAM;
    hints.ai_flags    = AI_PASSIVE;
    remoteinfo        = NULL;
    ret = getaddrinfo(host, "0", &hints, &remoteinfo);
    if (ret == -1) {
        perror(argv[0]);
        return -1;
    }

    if (remoteinfo->ai_socktype != AF_INET) {
        fprintf(stderr, "not an IP host!\n");
    }
    else {
        struct sockaddr_in* addr = (struct sockaddr_in*)remoteinfo->ai_addr;
        printf("+getaddrinfo\nIP:%s\n", inet_ntoa(addr->sin_addr));
    }

    freeaddrinfo(remoteinfo);

    return 0;
}
