#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sock_common.h"

int main()
{
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address; // changed
    struct sockaddr_in client_address; // changed

    printf("server");

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // bind the server socket with port and connect ip
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(SERVER_INTERNET_NAME);
    server_address.sin_port = SERVER_INTERNET_PORT;
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
