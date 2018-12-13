#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>

typedef struct sockaddr *sockaddrp;
int sockfd;
char secret;
int mode = 0; //0 for common  ,1 for send
FILE *fp;
char filename[50] = "download_";
char fname[50];
void *recv_other(void *arg)
{
    char buf[255] = {};
    while (1)
    {
        char yn = 0;
        char buf[255] = {};
        char buf2[255] = {};
        char buf3[55] = {};
        int ret = recv(sockfd, buf, sizeof(buf), 0);
            if (ret<0)
            {
                perror("recv");
                return 0;
            }
            printf("%s\n", buf);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("wrong argument");
        return -1;
    }
    //socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in addr = {AF_INET};
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = inet_addr(argv[2]);

    socklen_t addr_len = sizeof(addr);

    //connect
    int ret = connect(sockfd, (sockaddrp)&addr, addr_len);
    if (ret<0)
    {
        perror("connect");
        return -1;
    }

    //username
    char buf[255] = {};
    char name[255] = {};
    printf("Please input your name:");
    scanf("%s", name);
    strcpy(fname, name);
    ret = send(sockfd, name, strlen(name), 0);
    if (ret<0)
    {
        perror("connect");
        return -1;
    }

    //create child process
    pthread_t tid;
    ret = pthread_create(&tid, NULL, recv_other, NULL);

    if (0 > ret)
    {
        perror("pthread_create");
        return -1;
    }
    //send message
    while (1)
    {
        scanf("%s", buf);
        int ret = send(sockfd, buf, strlen(buf), 0);
        if (ret<0)
        {
            perror("send faild");
            return -1;
        }
        //quit
        if (strcmp("quit", buf)==0)
        {
            printf("%s,quit\n", name);
            return 0;
        }
    }
}
