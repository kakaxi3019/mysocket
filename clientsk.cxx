/*
socket client
*/

#include <stdio.h>
#include <stdlib.h>//exit
#include <string.h>//memset
#include <arpa/inet.h>//inet_pton
#include <unistd.h>//close
int main()
{
    char server_add[] = "127.0.0.1";
    char msg[] = "hello server";
    int clientfd = socket(AF_INET,SOCK_STREAM,0);
    int sendbufsize = 0;
    if(clientfd < 0)
    {
        printf("create socket error!\r\n");
        exit(0);
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8888);
    inet_pton(AF_INET,server_add,&serveraddr.sin_addr);
    if(connect(clientfd,(sockaddr*)&serveraddr,sizeof(serveraddr))<0)
    {
        printf("can't connect to server!\r\n");
        exit(0);
    }
    printf("success to connect server %s:%d\n",inet_ntoa(serveraddr.sin_addr), ntohs(serveraddr.sin_port));
    if((sendbufsize = send(clientfd,msg,strlen(msg),0)) < 0 )
    {
        printf("send msg to server error!\r\n");
        exit(0);
    }
    printf("send msg \"%s\"(%d) to server...\n",msg,sendbufsize);

    close(clientfd);
    printf("client quit!\r\n");
    exit(0);
}