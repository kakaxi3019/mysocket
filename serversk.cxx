/*
socket server
*/

#include <stdio.h>
#include <stdlib.h>//exit
#include <string.h>//memset
#include <arpa/inet.h>//inet_pton
#include <unistd.h>//close
#include <errno.h>
#include<signal.h>//signal(SIGCHLD, SIG_IGN);

#define MAXLINE 10
void lcl_process(int connfd, const sockaddr_in &rClientAdd);

int main()
{
    signal(SIGCHLD, SIG_IGN);
    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    int connectfd;
    int childpid;
    if(listenfd < 0)
    {
        printf("create socket error!\r\n");
        exit(0);
    }
    sockaddr_in socketaddr,clientaddr;
    socklen_t socklen = sizeof(clientaddr);
    memset(&socketaddr,0,sizeof(socketaddr));
    socketaddr.sin_family = AF_INET;
    socketaddr.sin_port = htons(8888);
    socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(listenfd,(sockaddr*)&socketaddr,sizeof(socketaddr))<0)
    {
        printf("bind error!\r\n");
        exit(0);
    }
    printf("init server ip=%s,port=%d..\n",inet_ntoa(socketaddr.sin_addr), ntohs(socketaddr.sin_port));
    if(listen(listenfd,10) == -1)
    {
        printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);  
        exit(0);
    }
    printf("server start...\n");

    while(true)
    {
        memset(&clientaddr,0,sizeof(clientaddr));
        if((connectfd = accept(listenfd,(sockaddr*)&clientaddr,&socklen)) == -1)
        {
            printf("accpet socket error: %s (errno :%d)\n",strerror(errno),errno); 
            continue;
        }
        printf("accept client:%s:%d, connectfd = %d...\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port),connectfd);
        childpid = fork();
        if(childpid == 0)
        {
            printf("pid=%d to process client request...\n",getpid());
            close(listenfd);
            lcl_process(connectfd,clientaddr);
            close(connectfd);
            exit (0);
        }
        else if( childpid>0 )
        {
            close(connectfd);
        }
        else
        {
            printf("server fork error!\n");
        }
        
        close(connectfd);
    }
    close(listenfd);
}

void lcl_process(int nConnfd, const sockaddr_in &rClientAdd)
{
    char buff[10];
    memset(buff,0,sizeof(buff));
    int nsize = 0;
    while((nsize = recv(nConnfd,buff,MAXLINE,0)) > 0)
    {
        buff[nsize] = '\0';
        printf(" receive client(%s:%d) message : %s\n", inet_ntoa(rClientAdd.sin_addr), rClientAdd.sin_port,buff);
    }
}