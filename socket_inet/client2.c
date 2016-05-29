#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sock_common.h"

int main()
{
    int len;
    int result;
    char ch ='A';
    int sockfd;
    struct sockaddr_in address; // changed

    // create a internet socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // name the socket
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(SERVER_INTERNET_NAME);
    address.sin_port = htons(SERVER_INTERNET_PORT);
    len = sizeof(address);

    result = connect(sockfd, (struct sockaddr*)&address, len);
    if (-1 == result) {
        perror("oops: client1");
        close(sockfd);
        return -1;
    }
    write(sockfd, &ch, 1);
    read(sockfd, &ch, 1);
    printf("char from server = %c\n", ch);

    close(sockfd);
    return 0;
}

