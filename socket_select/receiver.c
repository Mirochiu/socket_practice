#include <stdio.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define UDP_MAX_NUM 7*188

int InSock = -1;
int RunThread = 0;
int ReceivedCounter = 0;

void* ReceiverThread(void* arg)
{
    int ret;
    struct sockaddr_in from_addr;
    int from_addr_len;
    char buf[UDP_MAX_NUM];
    printf("Receiver Thread Started!\n");

    while (RunThread) {
        printf("recvfrom\n");
        ret = recvfrom(InSock, (void*)buf, UDP_MAX_NUM, 0, (struct sockaddr*)&from_addr, &from_addr_len);
        printf("ret = %d\n", ret);
        if (ret > 0) {
            ReceivedCounter += ret;
        }
    }

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
    char cmd;
    int main_exit = 0;

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

    printf("Create a thread for receiveing the send data\n");
    RunThread = 1;
    ret = pthread_create(&thread, NULL, ReceiverThread, NULL);
    if (0 != ret) {
        perror(argv[0]);
        close(InSock);
        return -1;
    }
    printf("ReceiverThread %lu created\n", thread);

    while(!main_exit) {
        printf("CMD> ");
        cmd = getchar();
        fflush(stdin); // clear the inputs
        printf("your input is '%c'\n", cmd);
        switch (cmd) {
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
            default:
                puts("==============================");
                puts("| CMD Options:               |");
                puts("==============================");
                puts("s : show the receiver counter ");
                puts("q : quit the program          ");
                puts("==============================");
                break;
        }
    }

    printf("main ended\n");
    return 0;
}
