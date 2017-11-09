#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <netdb.h> /*gethostbyname function */

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define MAXLINE 1024

void handle(int connfd);

int main(int argc, char **argv)
{
    char * servInetAddr = "127.0.0.1";
    int servPort = 6888;
    char buf[MAXLINE];
    int connfd;
    struct sockaddr_in servaddr;

    if (argc == 2) {
        printf("2 argc...argv[0]=%s,argv[1]=%s\n",argv[0],argv[1]);
        servInetAddr = argv[1];
    }
    if (argc == 3) {
        printf("3 argc...argv[0]=%s,argv[1]=%s,argv[2]=%s\n",argv[0],argv[1],argv[2]);
        servInetAddr = argv[1];
        servPort = atoi(argv[2]);
    }
    if (argc > 3) {
        printf("usage: echoclient <IPaddress> <Port>\n");
        return -1;
    }

    connfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);

    if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }
    printf("welcome to echoclient, please input msg send to server...\n");
    handle(connfd);     /* do it all */
    close(connfd);
    printf("exit\n");
    exit(0);
}

void handle(int sockfd)
{
    char sendline[MAXLINE], recvline[MAXLINE];
    int n;
    for (;;) {
        if (fgets(sendline, MAXLINE, stdin) == NULL) {
            break;//read eof
        }
        char* buff = NULL;
        buff = strchr(sendline, '\n');          //查找换行符  
        if(buff)                            //如果find不为空指针  
            *buff = '\0'; 
        /*
        //也可以不用标准库的缓冲流,直接使用系统函数无缓存操作
        if (read(STDIN_FILENO, sendline, MAXLINE) == 0) {
            break;//read eof
        }
        */
        printf("client send msg(%s) to server..\n",sendline);
        n = write(sockfd, sendline, strlen(sendline));
        n = read(sockfd, recvline, MAXLINE);
        if (n == 0) {
            printf("echoclient: server terminated prematurely\n");
            break;
        }
        recvline[n] = '\0';
        printf("client recv msg(%s) from server..\n",recvline);
        //write(STDOUT_FILENO, recvline, n);
        //如果用标准库的缓存流输出有时会出现问题
        //fputs(recvline, stdout);
    }
}