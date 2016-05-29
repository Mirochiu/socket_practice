#include <stdio.h>
#include <sys/un.h>

#include "sock_common.h"

int main()
{
    int len;
    int result;
    char ch ='A';
    int sockfd;
    struct sockaddr_un address;

    // create a socket
    sockfd = socket(AF_UNIX, SOCK_STREAM, 0);

    // name the socket
    address.sun_family = AF_UNIX;
    strcpy(address.sun_path, SERVER_SOCKET_NAME);
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

