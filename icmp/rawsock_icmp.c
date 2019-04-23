#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // errno
#include <unistd.h>
#include <sys/socket.h> // socket
#include <arpa/inet.h> // inet_ntop,inet_pton
#include <netinet/ip.h> // iphdr
#include <netinet/ip6.h> // ip6_hdr
#include <netinet/ip_icmp.h> // icmphdr
#include <netinet/icmp6.h> // icmp6_hdr

#define PKT_LEN 32 

//RAW
//https://www.intervalzero.com/library/RTX/WebHelp/Content/PROJECTS/Application%20Development/Understanding_Network/Using_RAW_Sockets.htm

//https://zh.wikipedia.org/wiki/%E5%8E%9F%E5%A7%8B%E5%A5%97%E6%8E%A5%E5%AD%97

//https://blog.csdn.net/DB_water/article/details/78482237
unsigned short mkcksum(unsigned short *addr, int len)
{
    unsigned int sum = 0, nleft = len;
    unsigned short answer = 0;
    unsigned short *w = addr;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(u_char *) (&answer) = *(u_char *) w;
        sum += answer;
    }
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = (unsigned short)(~sum);
    return (answer);
}

int setup_addr(int domain, char* ip_addr, int port, struct sockaddr *p_addr, socklen_t* p_addr_len) {
    if (AF_INET6 == domain) {
        struct sockaddr_in6 s_dst;
        struct in6_addr     s_addr6;
        memset(&s_addr6, 0, sizeof s_addr6);
        if (inet_pton(AF_INET6, ip_addr, &s_addr6) < 0) {
            return -1;
        }
        memset(&s_dst, 0, sizeof s_dst);
        s_dst.sin6_family       = AF_INET6;
        memcpy(&(s_dst.sin6_addr.s6_addr), &s_addr6, sizeof s_addr6);
        s_dst.sin6_port         = htons(port);
        // output
        *p_addr_len = sizeof(s_dst);
        memcpy(p_addr, &s_dst, sizeof(s_dst));
        return 0;
    } else {
        struct sockaddr_in  s_dst;
        struct in_addr      s_addr4;
        memset(&s_addr4, 0, sizeof s_addr4);
        if (inet_aton(ip_addr, &s_addr4)  < 0) {
            return -1;
        }
        memset(&s_dst, 0, sizeof s_dst);
        s_dst.sin_family      = AF_INET;
        s_dst.sin_addr.s_addr = s_addr4.s_addr;
        s_dst.sin_port        = htons(port);
        // output
        *p_addr_len = sizeof(s_dst);
        memcpy(p_addr, &s_dst, sizeof(s_dst));
        return 0;
    }
    return  -1;
}

#define ACTION_ECHO 0

//https://www.cymru.com/Documents/ip_icmp.h
int setup_icmphdr(int domain, int action, char* buf, int *size)
{
    static int code = 0;
    if (AF_INET6 == domain) {
        struct icmp6_hdr hdr;
        memset(&hdr, 0, sizeof hdr);
        hdr.icmp6_type = (action==ACTION_ECHO)?ICMP6_ECHO_REQUEST:action;
        hdr.icmp6_code = code;
        hdr.icmp6_id = htons(2236);
        hdr.icmp6_seq = htons(1);
        // should be the last line
        hdr.icmp6_cksum = mkcksum((unsigned short *)&hdr, sizeof hdr);
        *size = sizeof hdr;
        memcpy(buf, &hdr, sizeof hdr);
        return 0;
    } else {
        struct icmphdr hdr;
        memset(&hdr, 0, sizeof hdr);
        hdr.type = (action==ACTION_ECHO)?ICMP_ECHO:action;
        hdr.code = code;
        hdr.un.echo.id = htons(2234);
        hdr.un.echo.sequence = htons(1);
        // should be the last line
        hdr.checksum = mkcksum((unsigned short *)&hdr, sizeof hdr);
        *size = sizeof hdr;
        memcpy(buf, &hdr, sizeof hdr);
        return 0;
    }
    return -1;
}

int main(int argc, char *argv[])
{
    int send = -1, recv = -1;
    int ttl = 50; // Time to live, should not equal and lesser than 1
    int send_flags = 0;
    char send_buf[PKT_LEN];
    int send_size = 0;
    char recv_buf[PKT_LEN];
    int recv_len = -1;
    char* ip_addr = "8.8.8.8";
    int isIpv6 = 0;
    int domain = 0;
    int retval = 0;
    char* ptr;
    struct sockaddr *p_sock_addr = malloc(1024);
    socklen_t sock_addr_len;
    struct sockaddr recv_addr;
    socklen_t recv_addr_len;

    printf("isIpv6:%d\n", isIpv6);

    domain = isIpv6?AF_INET6:AF_INET;

    // ICMP socket
    if ((send = socket(domain, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        printf("Could not process socket() [send], %s\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto ERROR_RET;
    }

    // ICMP Socket
    if ((recv = socket(domain, SOCK_RAW, IPPROTO_ICMP)) < 0) {
        printf("Could not process socket() [recv], %s\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto ERROR_RET;
    }

    // Define time to life
    if (setsockopt(send, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) < 0) {
        printf("Could not process setsockopt(), %s\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto ERROR_RET;
    }

    if (setup_addr(domain, ip_addr, 0, p_sock_addr, &sock_addr_len) < 0) {
        printf("Could not process setup_addr(), %s\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto ERROR_RET;
    }

    if (setup_icmphdr(domain, ACTION_ECHO, send_buf, &send_size) < 0 ) {
        printf("Could not process setup_icmphdr(), %s\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto ERROR_RET;
    }

    printf("send size %d\n", send_size);
    if (sendto(send, send_buf, send_size, send_flags, (struct sockaddr*)p_sock_addr, sock_addr_len) < 0) {
        printf("Could not process sendto(), %s\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto ERROR_RET;
    }

    printf("pkt send, wait rsp\n");
    memset(recv_buf, 0, sizeof recv_buf);
    if ((recv_len = recvfrom(recv, recv_buf, sizeof recv_buf, 0, (struct sockaddr*)&recv_addr, &recv_addr_len)) < 0) {
        printf("Could not process recvfrom(), %s\n", strerror(errno));
        retval = EXIT_FAILURE;
        goto ERROR_RET;
    }

    printf("recv pkt\n");
    if (isIpv6) {
        // http://hanteye01.blog.fc2.com/blog-entry-2.html
        struct ip6_hdr *p_ip = (struct ip6_hdr *)recv_buf;
        if (IP6OPT_TYPE_ICMP == p_ip->ip6_nxt) {
            struct icmp6_hdr *p_icmp = (struct icmp6_hdr*)(recv_buf + 40);
            if (ICMP6_ECHO_REPLY == p_icmp->icmp6_type) {
                printf("got icmpv6 echo reply\n");
            } else {
                printf("not icmpv6 echo reply %u\n", (uint32_t)p_icmp->icmp6_type);
            }
        } else {
            printf("ipv6 type != icmp\n");
        }
    } else {
        struct iphdr *p_ip = (struct iphdr *)recv_buf;
        struct icmphdr *p_icmp = (struct icmphdr *)(recv_buf + (p_ip->ihl << 2));
        if (ICMP_ECHOREPLY == p_icmp->type) {
            printf("got icmp echo reply\n");
        } else if (ICMP_DEST_UNREACH == p_icmp->type) {
            printf("received ICMP unreachable, code:%d\n", p_icmp->type);
            switch(p_icmp->code) {
                case ICMP_NET_UNREACH     : printf(" => Network Unreachable\n");break;
                case ICMP_HOST_UNREACH    : printf(" => Host Unreachable\n");break;
                case ICMP_PROT_UNREACH    : printf(" => Protocol Unreachable\n");break;
                case ICMP_PORT_UNREACH    : printf(" => Port Unreachable\n");break;
                case ICMP_FRAG_NEEDED     : printf(" => Fragmentation Needed/DF set\n");break;
                case ICMP_SR_FAILED       : printf(" => Source Route failed\n");break;
                case ICMP_NET_UNKNOWN     : printf(" => Net Unknown\n");break;
                case ICMP_HOST_UNKNOWN    : printf(" => Host Unknown\n");break;
                case ICMP_HOST_ISOLATED   : printf(" => Host Isolated\n");break;
                case ICMP_NET_ANO         : printf(" => Net ANO\n");break;
                case ICMP_HOST_ANO        : printf(" => HOST ANO\n");break;
                case ICMP_NET_UNR_TOS     : printf(" => NET_UNR_TOS\n");break;
                case ICMP_HOST_UNR_TOS    : printf(" => HOST_UNR_TOS\n");break;
                case ICMP_PKT_FILTERED    : printf(" => Packet filtered\n");break;
                case ICMP_PREC_VIOLATION  : printf(" => Precedence violation\n");break;
                case ICMP_PREC_CUTOFF     : printf(" => Precedence cut off\n");break;
                default:break;
            }
        } else {
            printf("not icmp echo reply %u\n", (uint32_t)p_icmp->type);
        }
    }

    printf("dump len:%d\ndump rsp:", recv_len);
    ptr = recv_buf;
    while (recv_len > 0) {
        printf("%02X ", ((*ptr++)&0xff));
        recv_len--;
    }
    printf("\n");

ERROR_RET:
    if (NULL != p_sock_addr) {
        free(p_sock_addr);
        p_sock_addr = NULL;
    }
    if (-1 != send) {
        close(send);
        send = -1;
    }
    if (-1 != recv) {
        close(recv);
        recv = -1;
    }
    return retval;
}