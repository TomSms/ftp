#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "protocol.h"

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 20021
#define GET_COMMAND "get"
#define LIST_COMMAND "list"
#define QUIT_COMMAND  "quit"
#define FFLAG O_RDWR | O_CREAT | O_TRUNC | O_APPEND
#define FFMODE S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH
int create_socket();
void handle_biz(int fd);

void handle_get_file( int fd, char* buf);
void handle_list_file( int fd, char* buf);
void handle_quit(int fd,char* buf);
/*support command type: get file, list file of server dir,quit exit applications*/
typedef enum _type{
    get,
    list,
    quit,
    none = -1
}type;
int main()
{
    int fd = create_socket();
    handle_biz(fd);
    return 0;
}

type get_command_type(char* buf){
    if(strstr(buf,GET_COMMAND)){
        return get;
    }else if(strcmp(buf,LIST_COMMAND) == 0){
        return list;
    }else if(strcmp(buf,QUIT_COMMAND) == 0){
        return quit;
    }
    return none;
}
type getInput( char *buf){
     type ret = 0;
     gets(buf);
     while((ret= get_command_type(buf)) == none){
         printf("Command error,Please input your operate: get FILE or list\n");
         gets(buf);
     }
     return ret;
}
void handle_biz(int fd){
    char buf[1024];
    type t = none;
    while(1){
        printf("input your operate: get FILE or list\n");
        t = getInput(buf);
        if(t == get){
            handle_get_file(fd,buf);
        }else if(t == list){
            handle_list_file(fd,buf);
        }else if(t == quit){
            handle_quit(fd,buf);
            printf("quit app");
            close(fd);
            exit(0);
        }
    }

}
int create_socket(){
    int fd = -1;
    socklen_t len = 0;
    struct sockaddr_in addr;
    //struct timeval tv;
    //fd_set set;
    //int ret = -1;
    fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0){
        printf("socket create failed");
        exit(0);
    }
    addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_family = AF_INET;

    len = sizeof(struct sockaddr_in);

    if(connect(fd,(struct sockaddr*)&addr,len)< 0){
        printf("connect fd:%d failed \n",fd);
        close(fd);
        exit(0);
    }
    return fd;
}

char* get_file_name(char* buf){
     char *p = NULL;
     p = strstr(buf,GET_COMMAND);
     if(p == NULL) return p;
     p =  p+ strlen(GET_COMMAND);
     while(*p ==' '){
         p++;
     }
     return p;
}
void handle_get_file( int fd, char* buf){
    int ret = 0;
    int ffd = 0;
    int len = -1;
    //FILE *file = NULL;
    /*send get request*/
    if(protocol_send(fd,buf,strlen(buf)+1))
        return;
    /*recv get retquest status code*/
    if(protocol_recv(fd,&ret,&len)) return;
    printf("ret val:%d\n",ret);
    if(ret == 0){
        /*recv error from server*/
        protocol_recv(fd,buf,&len);
        printf("server return error:%s\n",buf);
    }else{
        printf("client open file:%s",get_file_name(buf));
        ffd = open(get_file_name(buf),FFLAG,FFMODE);
        while(1){
            if(protocol_recv(fd,buf,&len)) return;
            if(len != 0){
                printf("print write to file OK\n buf:%s\n",buf);
                write(ffd,buf,len);
            }
            else if(len == 0){
                printf("recv from serve over\n");
                close(ffd);
                break;
            }else{
                close(fd);
                close(ffd);
                exit(0);
            }

        }

        /*
        file = fopen(get_file_name(buf),"rw");
        while(1){
            protocol_recv(fd,buf,&len);
            if(len != 0)
                fwrite(buf,len,1,file);
            else if(len == 0){
                fclose(file);
                break;
            }else{
                fclose(file);
                exit(0);
            }
        }
        */

    }

}
void handle_list_file( int fd, char* buf){
    int nums = 0;
    int i = 0;
    int len = 0;
    protocol_send(fd,buf,strlen(buf)+1);
    protocol_recv(fd,&nums,&len);
    printf("total files:%d\n",nums);
    for(i = 0;i<nums;i++){
        protocol_recv(fd,buf,&len);
        printf("%s\n",buf);
    }
}

void handle_quit(int fd, char *buf){
    protocol_send(fd,buf,strlen(buf)+1);
}
