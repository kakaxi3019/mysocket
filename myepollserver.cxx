#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <sys/epoll.h> /* epoll function */
#include <fcntl.h>     /* nonblocking */
#include <sys/resource.h> /*setrlimit */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define LISTENQ 100
#define MAXEPOLLSIZE 10000
#define MAXLINE 1024

int socket_bind();
void do_epoll(int socketfd);
void do_process(int epollfds,int readyfds, const struct epoll_event *events, int listenfd, char *buff);
void handle_accept(int listenfd,int epollfds);
void handle_recv(int clientfd,int epollfds,char *buff);
void handle_send(int clientfd,int epollfds,char *buff);
void epoll_event_add(int epollfds,int fd);
void epoll_event_del(int epollfds,int fd);
void epoll_event_mod(int epollfds,int fd, int event);

int main()
{
    int socketfd;
    printf("start init server...\n");
    socketfd = socket_bind();
    if(listen(socketfd,LISTENQ) < 0)
    {
        perror("listen error!\n");
        exit(0);
    }
    printf("server is ready...\n");
    do_epoll(socketfd);
    close(socketfd);
}

int socket_bind()
{
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if( fd < 0 )
    {
        perror("create socket error!\n");
        exit(0);
    }
    sockaddr_in socketaddr;
    bzero(&socketaddr,sizeof(socketaddr));
    socketaddr.sin_family = AF_INET;
    socketaddr.sin_port = htons(6888);
    socketaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(fd,(sockaddr*)&socketaddr,sizeof(socketaddr))<0)
    {
        perror("bind error!\n");
        exit(0);
    }
    return fd;
}

void do_epoll(int socketfd)
{
    char buff[MAXLINE];
    bzero(buff,sizeof(buff));
    int epfd = epoll_create(MAXEPOLLSIZE);
    epoll_event_add(epfd,socketfd);
    int readyfd;
    struct epoll_event events[MAXEPOLLSIZE];
    for(;;)
    {
        readyfd = epoll_wait(epfd,events,MAXEPOLLSIZE-1,-1);
        if(readyfd<0)
        {
            perror("epoll_waite error!\n");
        }
        do_process(epfd,readyfd, events, socketfd,buff);
    }
    
}

void do_process(int epollfds,int readyfds, const struct epoll_event *events, int listenfd, char *buff)
{
    int fd,clientfd;
    for(int i=0;i<readyfds;++i)
    {
        fd = events[i].data.fd;
        if( fd == listenfd && (events[i].events & EPOLLIN))
        {
            handle_accept(listenfd,epollfds);
        }
        else if( events[i].events & EPOLLIN )
        {
            handle_recv(fd,epollfds,buff);
        }
        else if( events[i].events & EPOLLOUT )
        {
            handle_send(fd,epollfds,buff);
        }
    }

}

void handle_accept(int listenfd,int epollfds)
{
    struct sockaddr_in clientaddr;
    bzero(&clientaddr,sizeof(clientaddr));
    socklen_t nlen = sizeof(clientaddr);
    int clientfd;
    if( (clientfd = accept(listenfd,(sockaddr*)&clientaddr,&nlen)) < 0)
    {
        perror("accept error!\n");
        exit(0);
    }
    else
    {
        printf("client(%s:%d) connected to server...\n",inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
        epoll_event_add(epollfds,clientfd);
    }
}

void handle_recv(int clientfd,int epollfds,char *buff)
{
    int n = read(clientfd,buff,MAXLINE);
    if( n == -1 )
    {
        perror("read error!\n");
        close(clientfd);
        epoll_event_del(epollfds,clientfd);
    }
    else if( n ==0 )
    {
        printf("client quit!\n");
        close(clientfd);
        epoll_event_del(epollfds,clientfd);
    }
    else
    {
        printf("recv client msg(%s)\n",buff);
        //修改描述符对应的事件，由读改为写
        epoll_event_mod(epollfds,clientfd,EPOLLOUT);
    }
}

void handle_send(int clientfd,int epollfds,char *buff)
{
    int n;
    if((n = write(clientfd,buff,strlen(buff))) < 0)
    {
        perror("write error!\n");
        epoll_event_del(epollfds,clientfd);
    }
    else
    {
        printf("send msg(%s) to fd(%d)..\n",buff,clientfd);
        epoll_event_mod(epollfds,clientfd,EPOLLIN);
    }
    bzero(buff,sizeof(buff));
}

void epoll_event_add(int epollfds,int fd)
{
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(epollfds,EPOLL_CTL_ADD,fd,&ev);
    printf("add new fd(%d) to epolls..\n",fd);
}

void epoll_event_del(int epollfds,int fd)
{
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    epoll_ctl(epollfds,EPOLL_CTL_DEL,fd,&ev);
    printf("del old fd(%d) from epolls..\n",fd);
}

void epoll_event_mod(int epollfds,int fd, int event)
{
    struct epoll_event ev;
    bzero(&ev,sizeof(ev));
    ev.events = event;
    ev.data.fd = fd;
    epoll_ctl(epollfds,EPOLL_CTL_MOD,fd,&ev);
    printf("change fd(%d) event mod from epolls..\n",fd);
}