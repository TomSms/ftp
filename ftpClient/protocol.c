#include "protocol.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>


int protocol_send( int fd, void *data,  int len){
    int tlen = htonl(len);
    if(send(fd,(void*)&tlen,sizeof(len),0) < 0){
        printf("send len in fd:%d failed",fd);
        close(fd);
        return -1;
    }

    if(send(fd,data,(size_t)len,0)< 0){
        printf("send data in fd:%d failed\n",fd);
        close(fd);
        return -1;
    }
    return 0;
}

int protocol_recv( int fd, void *data,  int *plen){
    //printf("enter recv\n");
    if(recv(fd,data,4,0)<0){
        printf("recv len from fd:%d",fd);
        close(fd);
        return -1;
    }

    *plen = ntohl(*(int*)(data));
    //printf("read network len:%d\n",*plen);
    if(*plen == 0) return 0;
    if(recv(fd,data,(size_t)(*plen),0)<0){
        printf("recv data form fd:%d\n",fd);
        close(fd);
        return -1;
    }
    return 0;
}

