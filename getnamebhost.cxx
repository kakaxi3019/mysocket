#include <stdio.h>

#include <netdb.h>

#include <error.h>

#include <string.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <arpa/inet.h>

 

int main(int arg,char** argv){

  struct hostent *host,*host2;

  if(NULL == (host = gethostbyname(argv[1]))){

          herror("Error");

          return 1;

  }

 

  printf("name = %s\n",host->h_name);

  printf("aliases = %s\n",*host->h_aliases);

  printf("add type = %d\n",host->h_addrtype);

  printf("len = %d\n",host->h_length);

  printf("IP=%s\n",inet_ntoa(*(struct in_addr*)host->h_addr));

 

  printf("=================================\n");

  struct in_addr maddr;

  if(0 == inet_aton(argv[2],&maddr)){

          return 0;

  }

 

  char* c = (char*)&maddr;

  printf("org = %x.%x.%x.%x\n",*(c)&0xff,*(c+1)&0xff,*(c+2)&0xff,*(c+3)&0xff);

 

  if(NULL == (host2 = gethostbyaddr(&maddr,4,2))){

          printf("Error:%s\n",hstrerror(h_errno));

          return 1;

  }

  printf("name = %s\n",host2->h_name);

  printf("aliases = %s\n",*host2->h_aliases);

  printf("add type = %d\n",host2->h_addrtype);

  printf("len = %d\n",host2->h_length);

  printf("IP=%s\n",inet_ntoa(*(struct in_addr*)host2->h_addr));

 

  return 0;

}