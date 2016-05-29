#include <stdio.h>
#include <sys/un.h>
#include <errno.h>

#include "sock_common.h"

int main()
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_un server_address;
    struct sockaddr_un client_address;
    int ret;

    // remove old socket file
    ret = unlink(SERVER_SOCKET_NAME);
    // skip the file not exist error
    if (ret != 0 && (errno != ENOENT)) {
        perror("server");
    }

    // named the server socket
    server_sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, SERVER_SOCKET_NAME);
    server_len = sizeof(server_address);
    bind(server_sockfd, (struct sockaddr*)&server_address, server_len);

    listen(server_sockfd, 5);
    while (1) {
        char ch;
        printf("server waiting\n");

        client_len = sizeof(client_address);
        client_sockfd = accept(server_sockfd, (struct sockaddr*)&client_address, &client_len);

        read(client_sockfd, &ch, 1);
        ch++;
        write(client_sockfd, &ch, 1);
        close(client_sockfd);
    }

    return 0;
}
