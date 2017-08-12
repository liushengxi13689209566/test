
#include"myhead.h"

#define MAXEVENTS   20
#define SERV_PORT 5201
#define LISTENQ 20
// 0-------------注册
// 1------------登录
// 3------------注册成功
// 4------------登录成功
//
int join(TT join_msg ,int conn_fd) ; //注册
void  *fun(void *arg) ;
int sign_in(TT sign_msg ,int conn_fd)  ;//登录
    
int main(void )
{
    int  i,conn_fd, sock_fd, epfd, nfds  ; //epfd 为epoll 句柄 ,nfds 为发生事件的个数
    int  optval ;
    socklen_t clilen ; 
    struct sockaddr_in clientaddr ;    //客户机地址
    struct sockaddr_in serveraddr ;     //服务器地址
    struct epoll_event ev, events[MAXEVENTS]; 
    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sock_fd  <  0 ) myerror("server socket ",__LINE__);
    
    optval  =  1  ; //设置该套接字使之可以重新绑定端口
    if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(void *)&optval,sizeof(int))    < 0)
        myerror("server setsockopt",__LINE__);


    memset(&serveraddr,0, sizeof(struct sockaddr_in)) ;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERV_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_fd,(struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)) <  0) 
        myerror("server bind ",__LINE__);
    if(listen(sock_fd , LISTENQ) <  0) 
        myerror("server listen ",__LINE__);
    epfd = epoll_create(256);

    ev.data.fd = sock_fd;
    ev.events= EPOLLIN  ;

    epoll_ctl(epfd,EPOLL_CTL_ADD,sock_fd,&ev);  //加入
    pthread_t tid ;
    clilen = sizeof(struct sockaddr_in) ;
    for ( ; ; )
    {
        nfds=epoll_wait(epfd,events,MAXEVENTS,500);
     
        for(i= 0;i< nfds;i++)
        {
            if(events[i].data.fd == sock_fd )//如果新监测到一个SOCKET用户连接到了绑定的SOCKET端口 ， 建立新的连接
            {
                conn_fd = accept(sock_fd,(struct sockaddr *)&clientaddr, &clilen);
                if(conn_fd< 0)       myerror("server accept error",__LINE__);

                printf("accept a connection from %s\n",inet_ntoa(clientaddr.sin_addr)); 
             
                ev.data.fd = conn_fd;
               
                ev.events= EPOLLIN ;
             
                epoll_ctl(epfd,EPOLL_CTL_ADD,conn_fd,&ev); //epfd------epoll 句柄
            }
            else if(events[i].events&EPOLLIN)//如果是已经连接的用户，并且收到数据，那么进行读入
            {
                conn_fd =  events[i].data.fd ;
                //printf("-------------------------------------------lianjie is OK\n");
                if(pthread_create(&tid,NULL ,(void *)fun ,(void *)&conn_fd)   < 0  )      myerror("server pthread_create ",__LINE__);
                //printf("the recv is %s \n",recv_buf.str);
            }
        }
    }
    return 0;
}
void  *fun(void *arg)
{
   // printf("*****************************\n");
    TT recv_buf ;
    int conn_fd = *(int *)arg;
    int t ;
    memset(&recv_buf,0,sizeof(TT));
    t = recv(conn_fd,&recv_buf,sizeof(TT),0) ;/////////////////////////////////////////////接受信息!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    if(t== 0)     修改状态信息为0 
    printf("flag  ==  %d\n",recv_buf.flag);
    switch(recv_buf.flag)
    {
        case 0: join(recv_buf,conn_fd); /*printf("the recv is %s\n",recv_buf.str) ; */ break; //根据类型态调用函数
        case 1: sign_in(recv_buf,conn_fd);      break ; //登录
        case 2: break ;
        case 3: break;
        case 4: break;
    }
}
int join(TT join_msg ,int conn_fd)    //注册,1旦注册成功就创建属于它的一张表
{
    char query1[100];
    char query2[100];
    int t ;
    TT send_buf ;
    memset(&send_buf,0,sizeof(TT));
    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));

    MYSQL *mysql = mysql_init(NULL);
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;
    sprintf(query1,"insert into user_data values('0',%d,'%s','%s',%d);",join_msg.QQ ,join_msg.username ,join_msg.passwd ,conn_fd );

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0)         
    {
        strcpy(send_buf.str,"重复注册！！");
        send(conn_fd,&send_buf,sizeof(TT),0 ) ;
    }
    else                       //注册成功！！ 
    {
        send_buf.flag =  3 ; 
        strcpy(send_buf.str,"注册成功！！,请登录～～\n") ;  
        sprintf(query2,"create table tb_%d(friend_QQ int ,friendname char(32),conn_fd int) ;",join_msg.QQ); //注册成功，创建一张属于它的表
        mysql_real_query(mysql ,query2,strlen(query2));
        send(conn_fd,&send_buf,sizeof(TT),0);  
    }
    close_connection(mysql);
    return 0;
}
int sign_in(TT sign_msg ,int conn_fd)      //登录
{
    int t ;
    MYSQL_RES *res ;
    MYSQL_ROW row ;
    char query1[150] ;
    char query2[150] ;
    char query3[150] ;
    TT send_buf ;
    memset(&send_buf,0,sizeof(TT));
    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));
    memset(query3,0,sizeof(query3));

    MYSQL *mysql = mysql_init(NULL);
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;

    sprintf(query1 ,"select  * from user_data where QQ =%d ;",sign_msg.QQ ) ;
    sprintf(query2 ,"select  * from user_data where QQ =%d and state='0';",sign_msg.QQ ) ;
    sprintf(query3 ,"select  * from user_data where QQ =%d and passwd='%s';",sign_msg.QQ ,sign_msg.passwd) ;


    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;

    if( !res )     myerror("server mysql_store_result",__LINE__) ;

    if(mysql_num_rows(res) ==  0 ) 
    { 
        strcpy(send_buf.str,"用户不存在～～"); 
        send(conn_fd,&send_buf,sizeof(TT),0); 
    }
    else 
    {
        t= mysql_real_query(mysql ,query2,strlen(query2));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        res = mysql_store_result(mysql) ;
    }
    if(mysql_num_rows(res) == 0)
    {
        strcpy(send_buf.str,"重复登录\n");
        send(conn_fd,&send_buf,sizeof(TT),0); 
    }
    else 
    {
        t= mysql_real_query(mysql ,query3,strlen(query3));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        res = mysql_store_result(mysql) ;
    }
    if(mysql_num_rows(res) == 0)
    {
        strcpy(send_buf.str,"密码不匹配\n");
        send(conn_fd,&send_buf,sizeof(TT),0) ;
    }
    else 
    {
        send_buf.flag= 4 ;
        strcpy(send_buf.str,"登录成功！\n") ;
        send(conn_fd,&send_buf,sizeof(TT),0); 
    }
}

