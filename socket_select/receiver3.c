#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define UDP_MAX_NUM 7*188

int InSock = -1;
int RunThread = 0;
int ReceivedCounter = 0;
int PipeFd[2] = {-1, -1};

void* ReceiverThread(void* arg)
{
    int ret;
    struct sockaddr_in from_addr;
    int from_addr_len;
    char buf[UDP_MAX_NUM];
    fd_set reader_set;
    struct timeval timeout;
    int reader_max;
    int read_val;

    printf("Receiver Thread Started! sock %d\n", InSock);

    FD_ZERO(&reader_set);
    FD_SET(InSock, &reader_set);
    FD_SET(PipeFd[0], &reader_set);

    reader_max = (PipeFd[0]>InSock) ? PipeFd[0] : InSock;

    printf("max fd=%d\n", reader_max);

    while (RunThread) {
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        printf("selecting\n");
        ret = select(reader_max+1, &reader_set, 0, 0, &timeout);
        if (ret < 0) {
            puts("select failed");
            continue;
        } else if (0 == ret) {
            puts("select timeout");
            continue;
        }

        if (FD_ISSET(PipeFd[0], &reader_set)) {
            ret = read(PipeFd[0], &read_val, sizeof(read_val));
            if (ret > 0) {
                printf("read value=%d from pipe ret=%d\n", read_val, ret);
            } else {
                printf("read from pipe failed, ret=%d\n", ret);
            }
        }

        if (FD_ISSET(InSock, &reader_set)) {
            ret = recvfrom(InSock, (void*)buf, UDP_MAX_NUM, 0, (struct sockaddr*)&from_addr, &from_addr_len);
            printf("recvfrom ret = %d\n", ret);
            if (ret > 0) {
                ReceivedCounter += ret;
            }
        }
    }

    FD_CLR(PipeFd[0], &reader_set);
    FD_CLR(InSock, &reader_set);

    printf("close socket\n");
    close(InSock);

    printf("Receiver Thread Ended!\n");
    return NULL;
}

int main(int argc, char** argv)
{
    int ret;
    struct sockaddr_in address;
    int port = 55555;
    pthread_t thread;
    char* cmd = NULL;
    size_t cmd_len;
    int main_exit = 0;
    int put_val;

    if (argc > 1) {
        int tmp_port = atoi(argv[1]);
        if (tmp_port>=0 && tmp_port<=65535) {
            port = tmp_port;
        }
    }
    printf("default port=%d\n", port);

    InSock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (InSock < 0) {
        perror(argv[0]);
        return -1;
    }
    printf("socket %d created\n", InSock);

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);
    if (bind(InSock, (struct sockaddr *)(&address), sizeof(address)) < 0) {
        perror(argv[0]);
        close(InSock);
        return -1;
    }
    printf("socket binded with port %d\n", port);

    printf("create pipe for quick exit\n");
    ret = pipe(PipeFd);
    if (0 != ret) {
        perror(argv[0]);
        close(InSock);
        return -1;
    }

    printf("Create a thread for receiveing the send data\n");
    RunThread = 1;
    ret = pthread_create(&thread, NULL, ReceiverThread, NULL);
    if (0 != ret) {
        perror(argv[0]);
        close(InSock);
        close(PipeFd[0]);
        close(PipeFd[1]);
        return -1;
    }
    printf("ReceiverThread %lu created\n", thread);

    while(!main_exit) {
        printf("CMD> ");
        ret = getline(&cmd, &cmd_len, stdin);
        if (-1 == ret) {
            puts("getline error");
            perror(argv[0]);
            continue;
        }
        printf("ret=%d\n", ret);
        printf("your input is '%c'\n", cmd[0]);
        switch (cmd[0]) {
            case 'x':
                RunThread = 0;
                write(PipeFd[1], &put_val, sizeof(put_val));
                printf("wait the thread %lu end\n", thread);
                pthread_join(thread, NULL);
                printf("exit the program, now!\n");
                main_exit = 1;
                break;
            case 'q':
                RunThread = 0;
                printf("wait the thread %lu end\n", thread);
                pthread_join(thread, NULL);
                printf("exit the program, now!\n");
                main_exit = 1;
                break;
            case 's':
                printf("The received data counter is %d\n", ReceivedCounter);
                break;
            case 'i':
                put_val = rand() % 10;
                printf("Write a value %d to pipe\n", put_val);
                ret = write(PipeFd[1], &put_val, sizeof(put_val));
                if (0 != ret) {
                    perror(argv[0]);
                }
                break;
            default:
                puts("==============================");
                puts("| CMD Options:               |");
                puts("==============================");
                puts("s : show the receiver counter ");
                puts("q : quit the program          ");
                puts("i : write a value to pipe     ");
                puts("x : fast quit the program     ");
                puts("==============================");
                break;
        }
        free(cmd);
        cmd = NULL;
    }

    close(PipeFd[0]);
    close(PipeFd[1]);
    printf("main ended\n");
    return 0;
}

