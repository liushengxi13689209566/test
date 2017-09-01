/*************************************************************************
	> File Name: myhead.h
	> Author: Linuxer_liu
	> Mail: 
	> Created Time: 2017年08月08日 星期二 08时24分02秒
 ************************************************************************/

#ifndef _MYHEAD_H //防止重复包含该头文件
#define _MYHEAD_H
#endif
#include<sys/socket.h>
#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<netinet/in.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<arpa/inet.h>
#include<errno.h>
#include<stdlib.h>
#include<mysql/mysql.h>
#include <sys/epoll.h>
#include<pthread.h>


/*定义的宏*/
#define HOST    "localhost"
#define USER    "root"
#define DB_NAME  "chatroom"      // 以 root  的身份登录，只控制 chatroom 数据库
#define	PASSWD    "thq520&iwwfyf"

#define END          "\e[0m"
#define RED           "\e[1;31m"
#define GREEN       "\e[1;32m"
#define YELLOW      "\e[1;33m"
#define BLUE         "\e[1;34m"
#define PURPLE     "\e[1;35m"

#define  SERVER_IP "127.0.0.1"
#define  SERVER_PORT   5201


typedef struct information{
    int flag ;  
    int  state ;    //上线与离线的状态 0 离线， 1 上线 ,避免重复登录及注册,m 默认为 0
    int QQ ;
    char username[32] ; 
    char passwd[32] ;
    char str[130];
    int to  ;             //代表对方的QQ 号
    int num ;
    struct information *next ;
}TT ;        //代表结构体


/*各个函数说明*/

void myerror(const char *str ,int line)
{
    perror(str);
    printf("at %d \n",line);
    exit(1);
}
void mysql_connect(MYSQL *mysql)
{  
    if(!mysql_real_connect(mysql, HOST, USER, PASSWD, DB_NAME, 0, NULL, 0)) {  
             myerror("server mysql_real_connect", __LINE__);
    }  
}  

void close_connection(MYSQL *mysql)
{
    mysql_close(mysql);
}
char *my_time() 
{
    time_t nowtime;
    nowtime = time(NULL);
    char *p = ctime(&nowtime);
    return p ;
}
