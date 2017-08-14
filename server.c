
#include"myhead.h"

#define MAXEVENTS   20
#define SERV_PORT 5201
#define LISTENQ 20
// 0-------------注册
// 1------------登录
// 2------------登录成功
//3-------------下线
//4------------添加好友
//5------------添加好友成功
//6------------添加好友失败
//6------------delete 好友

int join(TT server_msg ,int conn_fd) ;    //注册
void  *fun(void *arg) ;
int sign_in(TT server_msg ,int conn_fd)  ;//登录
int add_friend(TT server_msg,int conn_fd)  ;   //添加好友  num 代表对方的QQ  ||  自己的QQ 是真正的QQ
int delete_friend(TT server_msg,int conn_fd) ;    //delete 好友  num 代表对方的QQ   ||   自己的QQ 是真正的QQ
int list_friend(TT server_msg ,int conn_fd)  ;  //
    
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
                //printf("the recv is %s \n",server_msg.str);
            }
        }
    }
    return 0;
}
void  *fun(void *arg)
{
   // printf("*****************************\n");
    TT server_msg ;
    int conn_fd = *(int *)arg;
    int t ;
    memset(&server_msg,0,sizeof(TT));
    t = recv(conn_fd,&server_msg,sizeof(TT),0) ;/////////////////////////////////////////////接受信息!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//    if(t== 0)     修改状态信息为0 
    printf("****************************flag  ==  %d\n",server_msg.flag);
    switch(server_msg.flag)
    {
        case 0: join(server_msg,conn_fd); /*printf("the recv is %s\n",server_msg.str) ; */ break; //根据类型态调用函数
        case 1: sign_in(server_msg,conn_fd);      break ; //登录
        case 3:  break ;//下线就修改状态，清空conn_fd
        case 4:add_friend(server_msg,conn_fd) ;  break;
        case 7:delete_friend(server_msg,conn_fd);  break;
        case 9: list_friend (server_msg,conn_fd) ;   break;
        case 10:  break ;
    }
}
int join(TT server_msg ,int conn_fd)    //注册,1旦注册成功就创建属于它的一张表
{
    char query1[100];
    char query2[100];
//    char conn[50]= "set user_data utf8" ;
    int t ;

    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));

    MYSQL *mysql = mysql_init(NULL);
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;
    sprintf(query1,"insert into user_data values('0',%d,'%s','%s',%d);",server_msg.QQ ,server_msg.username ,server_msg.passwd ,conn_fd );
  //  mysql_real_query(mysql,conn,strlen(conn));;
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0)         
    {
        strcpy(server_msg.str,"重复注册！！");
        send(conn_fd,&server_msg,sizeof(TT),0 ) ;
    }
    else                       //注册成功！！ 
    {
        strcpy(server_msg.str,"注册成功！！,请登录～～\n") ;  
        send(conn_fd,&server_msg,sizeof(TT),0);  
        sprintf(query2,"create table tb_%d(friend_QQ int ,friendname char(32),conn_fd int) ;",server_msg.QQ); //注册成功，创建一张属于它的表
        mysql_real_query(mysql ,query2,strlen(query2));
    }
    close_connection(mysql);
    return 0;
}
int sign_in(TT server_msg ,int conn_fd)      //登录
{
    int t ;
    MYSQL_RES *res ;
    MYSQL_ROW row ;
    char query1[150] ;
    char query2[150] ;
    char query3[150] ;
    char query4[150] ;
  
    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));
    memset(query3,0,sizeof(query3));
    memset(query4,0,sizeof(query4));

    MYSQL *mysql = mysql_init(NULL);
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;

    sprintf(query1 ,"select  * from user_data where QQ =%d ;",server_msg.QQ ) ;
    sprintf(query2 ,"select  * from user_data where QQ =%d and state='0';",server_msg.QQ ) ;
    sprintf(query3 ,"select  * from user_data where QQ =%d and passwd='%s';",server_msg.QQ ,server_msg.passwd) ;


    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;

    if( !res )     myerror("server mysql_store_result",__LINE__) ;

    if(mysql_num_rows(res) ==  0 ) 
    { 
        strcpy(server_msg.str,"用户不存在～～"); 
        send(conn_fd,&server_msg,sizeof(TT),0); 
    }
    else 
    {
        t= mysql_real_query(mysql ,query2,strlen(query2));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        res = mysql_store_result(mysql) ;
    }
    if(mysql_num_rows(res) == 0)
    {
        strcpy(server_msg.str,"重复登录\n");
        send(conn_fd,&server_msg,sizeof(TT),0); 
    }
    else 
    {
        t= mysql_real_query(mysql ,query3,strlen(query3));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        res = mysql_store_result(mysql) ;
    }
    if(mysql_num_rows(res) == 0)
    {
        strcpy(server_msg.str,"密码不匹配\n");
        send(conn_fd,&server_msg,sizeof(TT),0) ;
    }
    else 
    {
        server_msg.flag=  2 ;
        strcpy(server_msg.str,"登录成功！\n") ; //登录成功 ，修改状态为1 
        send(conn_fd,&server_msg,sizeof(TT),0); 
        sprintf(query4 ,"update user_data set state='1'  where QQ =%d and passwd='%s';",server_msg.QQ ,server_msg.passwd) ;
        t= mysql_real_query(mysql ,query4,strlen(query4));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    }
    close_connection(mysql);
}
int add_friend(TT server_msg,int conn_fd)    //添加好友  num 代表对方的QQ  ||  自己的QQ 是真正的QQ
{
    printf("into add_friend\n");
    int    t ;
    char query1[150] ;   
    char query2[150] ;
    MYSQL_RES *res ;
    MYSQL_ROW row ;
    memset(query1,0,sizeof(query1));
     memset(query2,0,sizeof(query2));
  
    MYSQL *mysql = mysql_init(NULL);
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;

    sprintf(query1 ,"select  conn_fd from user_data where QQ =%d ;",server_msg.num ) ;
    sprintf(query2 ,"insert into tb_%d  select QQ,username,conn_fd from user_data where QQ=%d ;",server_msg.QQ ,server_msg.num) ;
    printf("query == %s\n",query1 );
    printf("query == %s\n",query2);
  
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    //printf("t  ==     %d\n",t);

    res = mysql_store_result(mysql)  ;
   // printf("res ==     %d\n",res);

    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    printf("mysql_num_rows == %d\n",mysql_num_rows(res) ) ;
    if(mysql_num_rows(res) ==  0 ) 
    { 
        strcpy(server_msg.str,"用户不存在～～");  //客户端打印这句话 
        send(conn_fd,&server_msg,sizeof(TT),0); 
    }
    else  //find   it ，给他发送消息,进行验证
    {
        /*row=mysql_fetch_row(res);
        printf("frien conn_fd  == %d\n", *row[0]) ;
        sprintf(server_msg.str,"QQ= %d 的朋友想要加你为好友，你愿意吗？？",server_msg.QQ);
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);*/
        t= mysql_real_query(mysql ,query2,strlen(query2));   // 插入自己的表中
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        strcpy(server_msg.str,"添加好友成功");  //客户端打印这句话 
        server_msg.flag =  5 ;
        send(conn_fd,&server_msg,sizeof(TT),0); 

    }
    close_connection(mysql);
}

int delete_friend(TT server_msg,int conn_fd)    //delete 好友  num 代表对方的QQ   ||   自己的QQ 是真正的QQ
{
    int    t ;
    char query1[150] ;   
    char query2[150] ;
    MYSQL_RES *res ;
    MYSQL_ROW row ;
    memset(query1,0,sizeof(query1));
     memset(query2,0,sizeof(query2));
  
    MYSQL *mysql = mysql_init(NULL);
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;

    sprintf(query1 ,"select  conn_fd from user_data where QQ =%d ;",server_msg.num ) ;
    sprintf(query2 ,"delete  from tb_%d  where friend_QQ=%d;",server_msg.QQ ,server_msg.num) ;
    printf("query == %s\n",query1 );
    printf("query == %s\n",query2);
  
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    //printf("t  ==     %d\n",t);

    res = mysql_store_result(mysql)  ;
   // printf("res ==     %d\n",res);

    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    printf("mysql_num_rows == %d\n",mysql_num_rows(res) ) ;
    if(mysql_num_rows(res) ==  0 ) 
    { 
        strcpy(server_msg.str,"用户不存在～～");  //客户端打印这句话 
        send(conn_fd,&server_msg,sizeof(TT),0); 
    }
    else  //find   it ，给他发送消息,进行验证
    {
       
        t= mysql_real_query(mysql ,query2,strlen(query2));  
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        strcpy(server_msg.str,"删除好友成功，友谊可贵望珍惜\n");  //客户端打印这句话 
         //server_msg.flag =  8 ;
        send(conn_fd,&server_msg,sizeof(TT),0); 
    }
    close_connection(mysql);
}
int list_friend(TT server_msg ,int conn_fd)    //
{
    char query1[ 150];
    memset(query1,0,sizeof(query1));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    int t ;

    MYSQL *mysql = mysql_init(NULL);
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;

    sprintf(query1,"   select    *   from  tb_%d  ;",server_msg.QQ) ;
    printf("into list_friend and query1 == %s\n",query1);

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);

     res = mysql_store_result(mysql)  ;
     if( !res )     myerror("server mysql_store_result",__LINE__) ;

     while (row = mysql_fetch_row(res))      //找到数据 
     {  
            printf("*row[0] ==  %d\n",atoi(row[0]));
            printf("row[1] == %s\n",row[1] ) ;
            server_msg.num = atoi(row[0] );
            strcpy(server_msg.str,row[1]) ;
            send(conn_fd,&server_msg ,sizeof(TT),0) ;
     }  
     server_msg.flag   =   10  ;
     send(conn_fd,&server_msg ,sizeof(TT),0) ;
     close_connection(mysql);
}









