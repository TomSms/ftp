#include "ftp_handler.h"
#include "protocol.h"
#include <pthread.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
extern int run_flag;

#define GET_COMMAND "get"
#define LIST_COMMAND "list"
#define QUIT_COMMAND "quit"

//#define ROOT_DIR "/home/min"

#define ERR_NOSUCHEFILE "server can't find such file"

#define FFLAG O_RDWR

char ROOT_DIR[256];
char* get_file_name(char* buf);
void* handleRequest(void *data);
void handleGetRequest(int fd,char *name);
void handleListRequest(int fd);
int startHandle(int fd){
   pthread_t pt;
   printf("start handle fd:%d\n",fd);
   if(pthread_create(&pt,NULL,handleRequest,fd)){
        printf("pthread create handle thread failed");
        return -1;
   }
   return 0;
}

void* handleRequest(void *data){
    int fd = (int)data;
    struct timeval tv;
    fd_set set;
    int ret = -1;
    char buf[1024];
    int len;
    char *p = NULL;
    printf("entern handle request fd:%d\n",fd);
    getcwd(ROOT_DIR,256);
    while(run_flag){
        FD_ZERO(&set);
        FD_SET(fd,&set);
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        ret = select(fd+1,&set,&set,NULL,&tv);
        switch(ret){
        case -1:
            break;
        case 0:
            break;
        default:
            printf("recv data for handleRequest\n");
            protocol_recv(fd,buf,&len);
            if(len == 0){
                printf("have been close by other client");
                close(fd);
                return NULL;
            }
            printf("command:%s\n",buf);
            if(strstr(buf,GET_COMMAND) != NULL){
                handleGetRequest(fd,buf);
            }else if(strstr(buf,LIST_COMMAND) != NULL){
                handleListRequest(fd);
            }else if(strstr(buf,QUIT_COMMAND) != NULL){
                printf("exit by client thread");
                close(fd);
                return NULL;
            }
            memset(buf,0,1024);
            break;
        }
    }
    return NULL;
}

int isExitFile(const char* name){
    DIR * rootdir = opendir(ROOT_DIR);
    struct dirent *tmpdir = NULL;
    while((tmpdir = readdir(rootdir))!=NULL){
        printf("current d_name:%s name:%s\n",tmpdir->d_name,name);
        if(strcmp(name,tmpdir->d_name) == 0){
            closedir(rootdir);
            return 1;
        }
    }
    closedir(rootdir);
    return 0;
}
char* get_file_name(char* buf){
     char *p = NULL;
     p = strstr(buf,GET_COMMAND);
     if(p == NULL) return NULL;
     p = p+strlen(GET_COMMAND);

     while(*p ==' '){
         p++;
     }
     printf("get file_name:%s\nbuf:%s\n",p,buf);
     return p;
}

static void get_global_file_name(char* name){
    char n[200];
    strcpy(n,name);
    sprintf(name,"%s/%s",ROOT_DIR,get_file_name(n));
    printf("file global name:%s\n",name);
}
void handleGetRequest(int fd,char* name){
    char buf[1024];
    char n[1024];
    int ret = 0;
    int tfd = 0;
    //FILE *file = 0;
    //printf("filename:%s\n",name);
    if(!isExitFile(get_file_name(name))){
        protocol_send(fd,&ret,sizeof(ret));
        protocol_send(fd,ERR_NOSUCHEFILE,strlen(ERR_NOSUCHEFILE)+1);
    }else{
       ret = 1;
       protocol_send(fd,&ret,sizeof(ret));
       get_global_file_name(name);

       tfd =  open(name,FFLAG);
       printf("tfd:%d\n",tfd);
       while((ret = read(tfd,buf,1024)) != 0){
           printf("handle read file len:%d\n",ret);
           if(ret < 0){
               printf("read from file:%s failed\n",strerror(errno));
               close(tfd);
               return;
           }else{
               if(protocol_send(fd,buf,ret)){
                   break;
               }
           }
       }
       printf("%d: ret :%d \n",__LINE__, ret);
       if(ret == 0){
           printf("read from file:%s over\n",name);
           protocol_send(fd,&ret,ret);
       }
       /*
       file = fopen(name,"r");
       printf("file name:%s,file:%p\n",name,file);
       while((ret = fread(buf,1024,n,file)) != EOF){
           printf("read from buf:%s",buf);
           if(ret < 0){
               printf("read from file:%s failed",strerror(errno));
               fclose(file);
               return;
           }else if(ret == 0){
               printf("read from file:%s over",name);
               fclose(file);
           }else{
               protocol_send(fd,buf,ret);
           }
       }
       */
    }

}

void handleListRequest(int fd){
     int nums = 0;
     DIR * rootdir = opendir(ROOT_DIR);
     struct dirent *tmpdir = NULL;
     while((tmpdir = readdir(rootdir))!=NULL){
        nums++;
     }
     closedir(rootdir);
     protocol_send(fd,&nums,sizeof(nums));
     rootdir = opendir(ROOT_DIR);
     while((tmpdir = readdir(rootdir))!=NULL){
         protocol_send(fd,tmpdir->d_name,strlen(tmpdir->d_name)+1);
     }
}
