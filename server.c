#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct sockaddr *sockaddrp;

struct sockaddr_in src_addr[50];
socklen_t src_len = sizeof(src_addr[0]);

int confd[50] = {};
//member
struct node
{
    char name[10];
    int state; //1=quit  0=join
};
int count = 0;
//char nametable[1000];
int secret = 0;
struct node table[50];

void *broadcast(void *indexp)
{
    int index = *(int *)indexp;
    char buf_rcv[255] = {};
    char buf_rcv2[255] = {};
    char buf_snd[255] = {};
    char buf_snd1[255] = {};
    char buf_snd2[255] = {};
    char buf_snd3[255] = {};
    char buf_snd4[255] = {};
    char name2[30] = {};
    FILE *fp;
    //get username
    char name[20] = {};
    int ret = recv(confd[index], name, sizeof(name), 0);
    strcpy(name2, name);
    name2[strlen(name2)] = '\n';
    strcpy(table[count].name, name2);
    table[count].state = 0;
    if (ret<0)
    {
        perror("recv");
        close(confd[index]);
        return 0;
    }
    sprintf(buf_snd, "ID:%d User %s join the chat",index,name);
    for (int i = 0; i <= count; i++)
        send(confd[i], buf_snd, strlen(buf_snd), 0);
    while (1)
    {
        sprintf(buf_rcv, "");
        sprintf(buf_rcv2, "");
        sprintf(buf_snd, "");
        sprintf(buf_snd2, "");
        sprintf(buf_snd3, "");
        sprintf(buf_snd4, "");

        bzero(buf_rcv, sizeof(buf_rcv));
        recv(confd[index], buf_rcv, sizeof(buf_rcv), 0);

        //判斷是否退出
        if (0 == strcmp("quit", buf_rcv))
        {
            sprintf(buf_snd1, "%s leave this chat room", name);
            for (int i = 0; i <= count; i++)
            {
                if (i == index || 0 == confd[i])
                {
                    continue;
                }
                send(confd[i], buf_snd1, strlen(buf_snd1), 0);
            }
            table[index + 1].state = 1;
            confd[index] = -1;
            pthread_exit(0);
        }
        else if (0 == strcmp("list", buf_rcv))
        {
            sprintf(buf_snd2, "list:\n");
            for (int i = 0; i <= count; i++)
            {
                if (table[i].state != 1)
                    strcat(buf_snd2, table[i].name);
            }
            for (int i = 0; i <= count; i++)
            {
                printf("index = %d\n conf = %d\n", index, *confd);
                if (i == index || 0 == confd[i])
                {
                    send(confd[i], buf_snd2, strlen(buf_snd2), 0);
                    continue;
                }
            }
            continue;
        }
        
	if (strstr(buf_rcv, "to") != NULL || strstr(buf_rcv, "send") || strstr(buf_rcv, "NO") || strcmp(buf_rcv, "YES") == 0)
        {
            char *delim = ",";
            char *num;
            char *talk;
            char *unum;
            int user;
            if (strstr(buf_rcv, "to") != NULL || strstr(buf_rcv, "send"))
            {
                num = strtok(buf_rcv, delim);
                printf("num=%s\n", num);
                unum = strtok(NULL, delim);
                printf("unum=%s\n", unum);
                talk = strtok(NULL, delim);
                printf("talk=%s\n", talk);
                user = atoi(unum);
                sprintf(buf_snd3, "(secret)%s:%s", name, talk);
                printf("%s\n", name);
            }
            if (strstr(buf_rcv, "to") != NULL)
            {
                for (int i = 0; i <= count; i++)
                {
                    printf("index = %d\n conf = %d\n", index, *confd);
                    if (i == index || 0 == confd[i])
                    {
                        continue;
                    }
                    if (i == user)
                    {
                        send(confd[i], buf_snd3, strlen(buf_snd3), 0);
                    }
                }
            }
            continue;
        }

        sprintf(buf_snd, "%s:%s", name, buf_rcv);
        printf("%s\n", buf_snd);
        for (int i = 0; i <= count; i++)
        {
            if (i == index || 0 == confd[i])
            {
                continue;
            }
            send(confd[i], buf_snd, sizeof(buf_snd), 0);
        }
    }
}

int main(int argc, char **argv)
{
    printf("\n");
    for (int i = 0; i < 50; i++)
    {
        table[i].state = 0;
    }
    //create connect
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd<0)
    {
        perror("socket");
        return -1;
    }
    else
	printf("create successfully\n");

    //address
    struct sockaddr_in addr = {AF_INET};
    addr.sin_port = htons(atoi(argv[1]));
    addr.sin_addr.s_addr = inet_addr(argv[2]);

    socklen_t addr_len = sizeof(addr);

    //bind
    int ret = bind(sockfd, (sockaddrp)&addr, addr_len);
    if (ret<0)
    {
        perror("bind");
        return -1;
    }
    else
	printf("bind successfully\n");

    //max waiting
    listen(sockfd, 50);

    int index = 0;

    while (count <= 50)
    {
        confd[count] = accept(sockfd, (sockaddrp)&src_addr[count], &src_len);
        ++count;
        
        index = count - 1;
        pthread_t tid;
        int ret = pthread_create(&tid, NULL, broadcast, &index);
        if (ret<0)
        {
            perror("pthread_create faild");
            return -1;
        }
    }
}
