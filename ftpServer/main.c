#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <unistd.h>
#include "ftp_handler.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 20021

void sig_quit(int signo);
void runServer();
int run_flag = 1;

int main()
{
    if(signal(SIGINT,sig_quit) == SIG_ERR){
        printf("signal quit funcation register failed");
        exit(-1);
    }
    runServer();
    printf("ftp server exit");
    return 0;
}

void sig_quit(int signo){
    run_flag = 0;
}


void runServer(){
    int sfd = -1,cfd = -1;
    socklen_t len = 0;
    struct sockaddr_in addr;
    struct timeval tv;
    fd_set set;
    int ret = -1;
    int opt = 1;
    sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd < 0){
        printf("socket create failed");
        exit(-1);
    }
    addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_family = AF_INET;

    len = sizeof(struct sockaddr_in);

    if(bind(sfd,(struct sockaddr*)&addr,len) < 0){
        printf("bind socket failed");
        close(sfd);
        exit(-1);
    }


    if(listen(sfd,5) < 0){
        printf("listen failed");
        close(sfd);
        exit(-1);
    }


    while(run_flag){
        FD_ZERO(&set);
        FD_SET(sfd,&set);
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        ret = select(sfd+1,&set,NULL,NULL,&tv);

        switch(ret){
        case -1:
            printf("listen socket failed");
            run_flag = 0;
            break;
        case 0:
            continue;
        default:
            cfd = accept(sfd,(struct sockaddr*)&addr,&len);
            if(cfd < 0){
                printf("accept from sfd:%d failed",sfd);
                break;
            }else{
                if(setsockopt(cfd,IPPROTO_TCP,TCP_NODELAY,&opt,(socklen_t)sizeof(opt)) == -1){
                    printf("set socket tcp no delay fails");
                    close(cfd);
                    continue;
                }
                startHandle(cfd);
            }
            break;
        }
    }

    close(sfd);
    if(cfd > 0){
        close(cfd);
    }
}
