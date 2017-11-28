
#include"myhead.h"

#define    MAXEVENTS   20
#define    LISTENQ     20



int join(TT server_msg ,int conn_fd) ;     //注册 0 
int  *fun(int *arg) ;
int sign_in(TT server_msg ,int conn_fd)  ;//登录 1
int add_friend(TT server_msg ,int conn_fd) ; //添加好友消息的转发
int delete_friend(TT server_msg,int conn_fd) ;    //delete 好友  num 代表对方的QQ   ||   自己的QQ 是真正的QQ
int list_friend(TT server_msg ,int conn_fd)  ;  //
int chat_with(TT server_msg,int  conn_fd) ;   //flag == 11,验证消息
int  online(int number ) ;    //查看对方是否在线,在线返回其套接字，不在线返回0  
int look_friend(TT server_msg,int conn_fd);
int look_chat_record(TT server_msg,int conn_fd) ;  // 11 和22 的聊天记录
void add_one_massage(TT server_msg,int come_QQ ,int go_QQ) ;
int qun_num_check(TT *server_msg,int conn_fd,int sp);     //解决将群名胡乱输入的问题 state ==  -100

typedef struct User_list
{
    int QQ ;
    int list_conn_fd ;
    struct User_list *next;
}U_L;

U_L *head;
MYSQL *mysql ;
char IP[100];


U_L *init_list()
{
    U_L *temp ;
    temp=(U_L *)malloc(sizeof(U_L)) ;
    temp->next = NULL ;
    return temp ;
}

int search(int number)
{
    U_L *temp = head ;
    while(temp)
    {
        if(temp->QQ ==  number)
            return  1 ;
        temp= temp->next ;
    }
    return 0 ;
}

int add_list(TT server_msg ,int conn_fd)
{
    U_L *temp ;
    temp=(U_L *)malloc(sizeof(U_L));
    temp->QQ = server_msg.QQ;
    temp->list_conn_fd = conn_fd ;
    temp->next = head->next ;
    head->next = temp ;
}
/***********************************************测试函数************************************/
int print()
{
    U_L *temp = head->next ;
    while(temp)
    {
        printf("QQ == %d\n",temp->QQ);
        printf("list_conn_fd == %d\n",temp->list_conn_fd);
        temp = temp->next ;
    }
}

int add_friend(TT server_msg ,int conn_fd)     //添加好友消息的转发中心
{
    int fd ,t ;
    char query1[100];
    char query2[100];
    char query3[100];
    MYSQL_RES *res ;
    MYSQL_ROW row ;

    /*printf("server_msg.state == %d \n",server_msg.state);
    printf("server_msg.QQ == %d\n",server_msg.QQ);
    printf("server_msg.to == %d\n",server_msg.to);*/

    sprintf(query1, "insert into  tb_%d (friend_QQ) values(%d)",server_msg.QQ,server_msg.to) ;
    sprintf(query2, "insert into  tb_%d (friend_QQ) values(%d)",server_msg.to ,server_msg.QQ);
    sprintf(query3, "select  *   from tb_%d  where friend_QQ=%d ;",server_msg.QQ,server_msg.to) ;

    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    if(mysql_num_rows(res) !=  0 ) 
    {
        server_msg.state = -2 ;
        send(conn_fd,&server_msg,sizeof(TT),0) ;
        return 0 ;
    }
    if( (fd = online(server_msg.to) ))  //对方也在线 , flag  ==  4
    {   
        if(server_msg.state  ==  1 )  //加入各自的数据表中,对方同意之后，将两个人的数据都加入到对方的数据表中
        {
            if( mysql_real_query(mysql ,query1,strlen(query1)) !=  0)       myerror("server mysql_real_query",__LINE__);
            if( mysql_real_query(mysql ,query2,strlen(query2)) !=  0)       myerror("server mysql_real_query ",__LINE__);
        }    
        send(fd,&server_msg,sizeof(TT),0) ;
    }
  //else add_one_massage();     //对方不在线,当作离线消息处理 
}

int del_user(int conn_fd)
{
    U_L *pre= head  ;
    U_L *temp = head->next ;
    while(temp)
    {
        if(temp->list_conn_fd  ==  conn_fd)
        {
            pre->next = temp->next ;
            free(temp);
            return 0 ;
        }
        temp= temp->next ;
        pre=pre->next ;
    }
}


int logout(TT server_msg ,int conn_fd)    //flag == 3 
{
    char query1[100] ;
    int t;
    memset(query1,0,sizeof(query1));
    sprintf(query1,"insert into system_log (user_QQ,record )   values( %d,'sign out ');",server_msg.QQ  ); //退出登录

    if(server_msg.state  ==  -1 )     //取消连接,仅仅是连接上了，还没有用户存在，也就是还没有放入在线用户的链表中
    {    
        printf(RED"\t\t  IP(%s)    断  开   连   接   ！   ！\n"END,IP );
        close(conn_fd);
    }
    else  if(server_msg.state == 1)   //退出登录，但套接字还连接着
    {
        del_user(conn_fd);
        printf(GREEN"\t\t %d 下 线 了 ～ ～  \n"END,server_msg.QQ ) ;
        t= mysql_real_query(mysql ,query1,strlen(query1));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    }
    //print() ;
    return 0;
}




void  warning_logout(TT server_msg ,int conn_fd)    // flag is not  know 
{
    /*char query1[100] ;
    int t;
    memset(query1,0,sizeof(query1));
    sprintf(query1,"insert into system_log (user_QQ,record )   values( %d,'sign out  wraning ');",server_msg.QQ  ); //退出登录

    printf("server_msg.state == %d \n",server_msg.state);
    printf("server_msg.QQ == %d\n",server_msg.QQ);
    printf("server_msg.to == %d\n",server_msg.to); */
    //printf("conn_fd  == %d\n",conn_fd); 

    //if(server_msg.QQ   ==  0 )     //取消连接,仅仅是连接上了，还没有用户存在，也就是还没有放入在线用户的链表中
    //{    
        del_user(conn_fd);
        printf(RED"\t\t  IP(%s)  异   常  断  开   连   接 ( 该 连 接 的 用 户  被 强 制  性 退 出) ！！ \n"END,IP );
        close(conn_fd);
    //}
    /*else         //退出登录，但套接字还连接着
    {
        del_user(conn_fd);
        printf(GREEN"\t\t %d  异  常  掉   线 ，怀  疑  可 能 是  被  人  干  掉  了 ～ ～  \n"END,server_msg.QQ ) ;
        t= mysql_real_query(mysql ,query1,strlen(query1));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    }*/
    //print() ;
    //return 0;
}





int add_user_list(TT server_msg,int conn_fd)
{
    int t ,i;
    char query1[100];
    MYSQL_RES *res ;
    MYSQL_ROW row ;
    sprintf(query1, "select  friend_QQ  from tb_%d ;",server_msg.QQ) ;
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    t= mysql_num_rows(res);  //行数
    if(t == 0) {server_msg.state = -8 ; send(conn_fd,&server_msg,sizeof(TT),0) ; }

    for(i= 0 ;i< t ;i++)
    {
        row = mysql_fetch_row(res);
        server_msg.to = atoi(row[0]) ;
        //printf("server_msg.to == %d \n",server_msg.to);
        if(i == t-1 ) server_msg.state = -8 ;
        send(conn_fd,&server_msg,sizeof(TT),0) ;
    }
}
/*********************************************************************群的处理***********************************/
int create_group(TT server_msg,int conn_fd)  //flag ==  8
{
    /*printf("server_msg.QQ == %d \n",server_msg.QQ) ;
    printf("server_msg.to == %d \n",server_msg.to) ;*/

    char query1[100];
    char query2[100];
    char query3[100];

    int t ;

    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));
    memset(query3,0,sizeof(query3));

    sprintf(query1,"create table qun_%d(user_QQ int unsigned primary key ,username char(32),mode int default 0 ) ;",server_msg.to);
    sprintf(query2,"insert into  qun_%d   values(%d ,NULL , 1) ;",server_msg.to,server_msg.QQ); 
    sprintf(query3,"insert into  all_qun   values(%d ,%d , NULL) ;",server_msg.to,server_msg.QQ); 
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    t= mysql_real_query(mysql ,query2,strlen(query2));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
}







int my_qun_check(TT *server_msg,int conn_fd,int sp)   //state  ==   -99    
{
    char query1[100];
    int t ;

    MYSQL_RES *res ;
    MYSQL_ROW row ;
    //printf("into qun_num_check and server_msg.to == %d \n",server_msg.to);

    memset(query1,0,sizeof(query1));

    sprintf(query1, "select * from  qun_%d  where user_QQ = %d ;",server_msg->to,server_msg->QQ ) ;

   // printf("query1 == %s \n",query1);

    t= mysql_real_query(mysql ,query1,strlen(query1));

    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;

    if(mysql_num_rows(res)  ==   0)     //胡乱输入
    {
        server_msg->state = -99;
        send(conn_fd,server_msg,sizeof(TT),0) ;
        return -1;
    }
    else     //没有胡乱输入                      
    {
        server_msg->state = sp ;  //   is right
        return 0;
    }
}



int real_qun_chat(TT server_msg ,int conn_fd)   //群聊转发
{
   /* printf("群号为：%d\n",server_msg.to);
    printf("QQ 为：%d\n",server_msg.QQ) ;
    printf("消息为：%s\n",server_msg.str); */

    int t,i ,number;
    int temp ;
    int fd ;
    char query1[100];
    char query2[100];
    MYSQL_RES *res ;
    MYSQL_ROW row ;
   

    memset(&query1,0,sizeof(query1));
    sprintf(query1,"select user_QQ from qun_%d ;",server_msg.to) ;

    sprintf(query2,"insert into  qun_chat_record (qun_num,come_QQ ,record) values(%d,%d,'%s') ;",server_msg.to,server_msg.QQ,server_msg.str) ;
    if(server_msg.state  ==   -100)
    {
      
        if(qun_num_check(&server_msg,conn_fd,-99) < 0 )  //代表胡乱输入
              {
                //printf("state == %d \n",server_msg.state);
                return  0 ;
            }
    }       //state = -99

    if(server_msg.state  ==   -99)
    {
        //printf("5555555\n");
       // sp= -98 ;
        if(my_qun_check(&server_msg,conn_fd,-98) < 0 )      //myself is not in this qun
                return  0 ;
    } //state = - 98  ;

    if( server_msg.state  ==   -98 )    //验证群号成功，让客户端进行输入 ；
    {
        //printf("6666\n");
        server_msg.flag =  9 ;
        send(conn_fd,&server_msg,sizeof(TT),0)  ;
        //printf("state == %d \n",server_msg.state);
        return 0 ;
        
    } //state = - 1  ;

   // printf("777 \n");



    t= mysql_real_query(mysql ,query2,strlen(query2));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    t= mysql_num_rows(res);  //t 行

  
    temp = server_msg.to;
    server_msg.state = -1 ;

    for(i = 0 ;i< t ;i++)
    {
        row = mysql_fetch_row(res);
        number = atoi(row[0]);

        if(number ==  server_msg.QQ )   continue ;
        
        if( (fd = online(number) ))  //对方也在线
        {
            // printf("online return == %d \n",online(server_msg)) ;
            // printf("will be send %s\n",server_msg.str) ;
            // printf("fd == %d\n",fd);
            send(fd,&server_msg,sizeof(TT),0)  ;
        }
        else add_one_massage(server_msg , temp, number);     //对方不在线,当作离线消息处理
    }
}





void add_one_massage(TT server_msg,int come_QQ ,int go_QQ)
{
    char query1[100];
    int t ;
    memset(query1,0,sizeof(query1));

    sprintf(query1,"insert into off_line_record values(%d,%d,'%s',%d ) ;",go_QQ,come_QQ,server_msg.str,server_msg.state);

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
}




int invite_one(TT server_msg ,int conn_fd)     //flag ==  10  
{
    int fd ,t ;
    char query1[100];
    char query3[100];
    MYSQL_RES *res ;
    MYSQL_ROW row ;

    /*printf("server_msg.state == %d \n",server_msg.state); // -1 邀请 1同意 2不同意
    printf("server_msg.QQ == %d\n",server_msg.QQ); //我的QQ
    printf("server_msg.to == %d\n",server_msg.to);//群号
    printf("server_msg.num == %d\n",server_msg.num); //对方的QQ*/

    sprintf(query1, "insert into  qun_%d (user_QQ ,mode) values(%d,0) ;",server_msg.to,server_msg.QQ) ;
    sprintf(query3, "select  *   from qun_%d  where user_QQ= %d ;",server_msg.to,server_msg.num) ;

    if(server_msg.state  ==   -100)
    {
        if(qun_num_check(&server_msg,conn_fd,-99) < 0 )  //代表胡乱输入
            return  0 ;
    }
    if(server_msg.state  ==   -99)
    {
        if(my_qun_check(&server_msg,conn_fd,-1 ) < 0 )      //myself is not in this qun
            return  0 ;
    } //state = -1 ;


    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    //printf("mysql_num_rows ==%d \n" ,mysql_num_rows(res)); //  !=  0 ，代表已经加入
    if((server_msg.state == -1 )&&(mysql_num_rows(res) != 0 )) 
    {
        server_msg.state = -2 ;

        send(conn_fd,&server_msg,sizeof(TT),0) ;

        return 0 ;
    }
    if( (fd = online(server_msg.num) ))  //对方也在线 , flag  ==  4
    {   
        if(server_msg.state  ==  1 )  //加入各自的数据表中,对方同意之后，将两个人的数据都加入到对方的数据表中
        {
            if( mysql_real_query(mysql ,query1,strlen(query1)) !=  0)       myerror("server mysql_real_query",__LINE__);
        }    
        send(fd,&server_msg,sizeof(TT),0) ;
    }
  //else add_one_massage();     //对方不在线,当作离线消息处理 
}



int qun_num_check(TT *server_msg,int conn_fd,int sp)     //解决将群名胡乱输入的问题 state ==  -100
{
    char query1[100];
    int t ;

    MYSQL_RES *res ;
    MYSQL_ROW row ;
    //printf("into qun_num_check and server_msg.to == %d \n",server_msg.to);

    memset(query1,0,sizeof(query1));

    sprintf(query1, "select * from  all_qun where qun_num= %d ;",server_msg->to) ;

    //printf("query1 == %s \n",query1);

    t= mysql_real_query(mysql ,query1,strlen(query1));

    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;

    if(mysql_num_rows(res)  ==   0)     //胡乱输入
    {
        server_msg->state = -100 ;
        send(conn_fd,server_msg,sizeof(TT),0) ;
        return -1;
    }
    else     //没有胡乱输入                      
    {
        server_msg->state = sp ;
        return 0;
    }
}


int add_group(TT server_msg ,int conn_fd)     //加群的转发中心
{
    //printf("***************************server_msg.flag == %d \n",server_msg.flag);
    int fd ,t  ;
    int sp =  -1 ;
    char query1[100];
    char query2[100];
    char query3[100];

    MYSQL_RES *res ;
    MYSQL_ROW row ;

   /* printf("server_msg.state == %d \n",server_msg.state); // -1 申请   1同意   2不同意
    printf("server_msg.QQ == %d\n",server_msg.QQ); //我的QQ
    printf("server_msg.num == %d\n",server_msg.num); 
    printf("server_msg.to == %d\n",server_msg.to);//群号*/

    sprintf(query1, "insert into  qun_%d (user_QQ ,mode) values(%d,0) ;",server_msg.to,server_msg.num) ;
    sprintf(query2, "select  user_QQ from qun_%d  where mode= 1 ;",server_msg.to ) ;
    sprintf(query3, "select  *   from qun_%d  where user_QQ= %d ;",server_msg.to,server_msg.QQ ) ;

    if(server_msg.state  ==   -100)
    {
        if(qun_num_check(&server_msg,conn_fd,sp) < 0 )  //代表胡乱输入
            return  0 ;
    }
    //printf("改变后的state == %d \n",server_msg.state);

  

    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;

    //printf("mysql_num_rows ==%d \n" ,mysql_num_rows(res)); //  !=  0 ，代表已经加入 

    if((server_msg.state ==  -1 )&&( mysql_num_rows(res) != 0 )) 
    {
        server_msg.state = -2 ;

        send(conn_fd,&server_msg,sizeof(TT),0) ;

        return  0;
    }
    if(server_msg.state == -1) 
    {
        t= mysql_real_query(mysql ,query2,strlen(query2));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        res = mysql_store_result(mysql) ;
        if( !res )     myerror("server mysql_store_result",__LINE__) ;

        row=mysql_fetch_row(res);
        server_msg.num = atoi(row[0]);
    }
    if( (fd = online(server_msg.num) ))  //对方也在线 , flag  ==  12
    {   
        if(server_msg.state  ==  1 )    //群主同意加入
        {
            if( mysql_real_query(mysql ,query1,strlen(query1)) !=  0)       myerror("server mysql_real_query",__LINE__);
        }    
        send(fd,&server_msg,sizeof(TT),0) ;
    }
  //else add_one_massage();     //对方不在线,当作离线消息处理   
}


int jiesan(TT server_msg ,int conn_fd)
{
    int t ;
    int sp = -1 ;
    char query1[100];
    char query2[100];
    char query3[100];

    MYSQL_RES *res ;

    /*printf("server_msg.state == %d \n",server_msg.state); // -1 邀请 1同意 2不同意
    printf("server_msg.QQ == %d\n",server_msg.QQ); //我的QQ
    printf("server_msg.to == %d\n",server_msg.to); //群号
    //printf("server_msg.num == %d\n",server_msg.num); //对方的QQ*/

    sprintf(query1, "drop table  qun_%d ;",server_msg.to ) ;
    sprintf(query2, "delete from all_qun where qun_num= %d and qun_zhu_QQ= %d ;",server_msg.to,server_msg.QQ ) ;
    sprintf(query3, "select  *   from qun_%d  where user_QQ= %d and mode= 1 ;",server_msg.to,server_msg.QQ) ;
    //printf("query2 == %s \n",query2);
    if(server_msg.state  ==   -100)
    {
        if(qun_num_check(&server_msg,conn_fd,sp) < 0 )  //代表胡乱输入
            return  0 ;
    }
    /*printf("server_msg.state == %d \n",server_msg.state); // -1 邀请 1同意 2不同意
    printf("server_msg.QQ == %d\n",server_msg.QQ); //我的QQ
    printf("server_msg.to == %d\n",server_msg.to); //群号*/
    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
   // printf("mysql_num_rows ==%d \n" ,mysql_num_rows(res));  //  !=  0 ，代表已经加入

    if( mysql_num_rows(res) ==  0 )
    {
        server_msg.state = -2 ;  //bu shi qun zhu  

        send(conn_fd,&server_msg,sizeof(TT),0) ;

        return 0 ;
    }
    else{   //is qun zhu 
        if( mysql_real_query(mysql ,query1,strlen(query1)) !=  0)       
        myerror("server mysql_real_query",__LINE__);
        if( mysql_real_query(mysql ,query2,strlen(query2)) !=  0)       
        myerror("server mysql_real_query",__LINE__);
        send(conn_fd,&server_msg,sizeof(TT),0) ;
    }
}


int tiren(TT server_msg ,int conn_fd)     //flag ==  14
{
    int t,fd ;
    int sp = -1 ;
    char query1[100];
    char query2[100];
    char query3[100];

    MYSQL_RES  *res ;

   /* printf("server_msg.state == %d \n",server_msg.state);   // -1 邀请 1同意 2不同意
    printf("server_msg.QQ == %d\n",server_msg.QQ); //我的QQ
    printf("server_msg.to == %d\n",server_msg.to); //群号
    printf("server_msg.num == %d\n",server_msg.num); //对方的QQ*/

    sprintf(query1, "delete from qun_%d  where user_QQ = %d  ;",server_msg.to,server_msg.num ) ;
    sprintf(query2, "select *  from  qun_%d  where user_QQ =%d ;",server_msg.to,server_msg.num ) ;
    sprintf(query3, "select  *   from qun_%d  where user_QQ= %d and mode= 1 ;",server_msg.to,server_msg.QQ) ;
    if(server_msg.state  ==   -100)
    {
        if(qun_num_check(&server_msg,conn_fd,sp) < 0 )  //代表胡乱输入
            return  0 ;
    }
    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    //printf("mysql_num_rows ==%d \n" ,mysql_num_rows(res)); //  !=  0 ，代表已经加入

    if(mysql_num_rows(res) == 0 )
    {
        server_msg.state = -2 ; 

        send(conn_fd,&server_msg,sizeof(TT),0) ;

        return 0 ;
    }
    else{ 
        t= mysql_real_query(mysql ,query2,strlen(query2));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        res = mysql_store_result(mysql) ;
        if( !res )     myerror("server mysql_store_result",__LINE__) ;
        if(mysql_num_rows(res) == 0)
        {
            server_msg.state =  -1 ;
            send(conn_fd,&server_msg,sizeof(TT),0) ;
            return  0;
        }
        else 
        {
            server_msg.state = 1 ;  //通过检测
            if( mysql_real_query(mysql ,query1,strlen(query1)) !=  0)       
                myerror("server mysql_real_query",__LINE__); 
            send(conn_fd,&server_msg,sizeof(TT),0) ;

            server_msg.state = 2 ;
            if( (fd = online(server_msg.num) ))  //对方也在线 , flag  ==  14
                send(fd,&server_msg,sizeof(TT),0) ;
            //else add_one_massage();     //对方不在线,当作离线消息处理 
        }
    }
}

int look_group_friend(TT server_msg ,int conn_fd)     //flag ==  15
{
    int t,fd, i ;
    int sp = -1 ;
    char query1[100];
    char query2[100];

    MYSQL_RES  *res ;
    MYSQL_ROW   row ;

    /*printf("server_msg.state == %d \n",server_msg.state);   // -1 邀请 1同意 2不同意
    printf("server_msg.QQ == %d\n",server_msg.QQ); //我的QQ
    printf("server_msg.to == %d\n",server_msg.to); //群号*/

    sprintf(query1, "select user_QQ  from qun_%d   ;",server_msg.to  ) ;
    sprintf(query2, "select *  from  qun_%d  where user_QQ =%d ;",server_msg.to,server_msg.QQ ) ;
    if(server_msg.state  ==   -100)
    {
        if(qun_num_check(&server_msg,conn_fd,sp) < 0 )  //代表胡乱输入
            return  0 ;
    }
    t= mysql_real_query(mysql ,query2,strlen(query2));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
   // printf("mysql_num_rows ==%d \n" ,mysql_num_rows(res)); //  !=  0 ，代表已经加入

    if(mysql_num_rows(res) == 0 )  //不是群成员
    {
        server_msg.state = -2 ; 

        send(conn_fd,&server_msg,sizeof(TT),0) ;

        return  0;
    }
    else 
    {
        t= mysql_real_query(mysql ,query1,strlen(query1));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        res = mysql_store_result(mysql) ;
        if( !res )     myerror("server mysql_store_result",__LINE__) ;
        t= mysql_num_rows(res) ;  //t 行
        for(i= 0 ;i< t;i++)
        {
            row=mysql_fetch_row(res);
            server_msg.num= atoi(row[0]) ;
            send(conn_fd,&server_msg,sizeof(TT),0) ;
        }
    }
}

int lookup_qun_chat_record(TT server_msg ,int conn_fd)   //flag ==  16
{

    int t ,i;
    char query1[100];
   /* printf("into lookup_qun_chat_record ************************\n");
    printf("server_msg.QQ == %d \n",server_msg.QQ);
    printf("server_msg.to == %d \n",server_msg.to);
    printf("server_msg.QQ == %d \n",server_msg.QQ);*/

    MYSQL_RES *res ;
    MYSQL_ROW row ;
    sprintf(query1, "select  *   from  qun_chat_record  where qun_num = %d  ;",server_msg.to) ;
    if(server_msg.state  ==   -100)
    {
        if(qun_num_check(&server_msg,conn_fd,-99) < 0 )  //代表胡乱输入
            return  0 ;
    }

    if(server_msg.state  ==   -99)
    {
        if(my_qun_check(&server_msg,conn_fd,-1 ) < 0 )      //myself is not in this qun
            return  0 ;
    } //state = - 1  ;

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);

    res = mysql_store_result(mysql) ;
    t= mysql_num_rows(res);  //t 行

    for(i= 0 ;i< t ;i++)
    {
        row=mysql_fetch_row(res);
        server_msg.QQ = atoi(row[1]) ;
        //server_msg.to = atoi(row[1]) ;
        strcpy(server_msg.str,row[3]);
        strcpy(server_msg.passwd,row[4]);  // 用passwd 来替代时间
        send(conn_fd,&server_msg,sizeof(TT),0) ; //发送
    }
}




int exit_qun(TT server_msg ,int conn_fd) //flag == 17 
{
    char query1[100];
    int t ;
    sprintf(query1, "delete  from  qun_%d  where user_QQ  = %d  ;",server_msg.to,server_msg.QQ) ;
    if(server_msg.state  ==   -100)
    {
        if(qun_num_check(&server_msg,conn_fd,-1 ) < 0 )  //代表胡乱输入
            return  0 ;
    }
    server_msg.state = -1 ;
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    send(conn_fd,&server_msg,sizeof(TT),0) ; //发送
}




int send_file(TT server_msg,int conn_fd) //flag  ==  18
{
    int fd ,t ;
    char query1[150] ;
    memset(query1,0,sizeof(query1));
    /*printf("server_msg.passwd == %s \n",server_msg.passwd);
    printf("server_msg.QQ == %d \n",server_msg.QQ);
    printf("server_msg.to == %d \n",server_msg.to);
    printf("server_msg.num == %d \n",server_msg.num);
    printf("server_msg.state == %d \n",server_msg.state);
    printf("server_msg.str == %s \n",server_msg.str);*/
  


    sprintf(query1 ,"insert into chat_record  (come,go,record) values(%d,%d, 'send file :%s');",server_msg.QQ ,server_msg.to,server_msg.passwd ) ; //加入私聊记录
    if(server_msg.state ==  1)     // 1 传完  ，加入聊天记录
    {
        t= mysql_real_query(mysql ,query1,strlen(query1));
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        if( (fd = online(server_msg.to) ))   //对方也在线
           send(fd,&server_msg,sizeof(TT),0)  ;
    }
    else if( (fd = online(server_msg.to) ))   //对方也在线
    {
       // printf("online return == %d \n",online(server_msg)) ;
       //printf("will be send %s\n",server_msg.str) ;
       // printf("fd == %d\n",fd);
        //printf("server_msg.to == %d \n",server_msg.to);
        //printf("server_msg.state == %d \n",server_msg.state);
        send(fd,&server_msg,sizeof(TT),0)  ;
    }
  //else add_one_massage();     //对方不在线,当作离线消息处理
}




int lookup_offline_record(TT server_msg,int conn_fd)  // 发完就把消息全删了
{
    int t ,i;
    char query1[100];
    char query2[100];
    //char query3[100];

    MYSQL_RES *res ;
    MYSQL_ROW row ;

    sprintf(query1, "select  *   from  off_line_record   where go_QQ  = %d  ;",server_msg.QQ) ;
    sprintf(query2, "delete  from  off_line_record   where go_QQ  = %d  ;",server_msg.QQ) ;     //删除与我有关的消息
    //sprintf(query3, "delete  from  off_line_record   where come_QQ  = %d  ;",server_msg.QQ);  //删除群消息

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);

    res = mysql_store_result(mysql) ;
    t= mysql_num_rows(res);  //t 行

    for(i= 0 ;i< t ;i++)
    {
        row=mysql_fetch_row(res);
        server_msg.to = atoi(row[1]) ;
        strcpy(server_msg.str,row[2]);
        server_msg.state = atoi(row[3]) ;
        send(conn_fd,&server_msg,sizeof(TT),0) ; //发送
    }

    t= mysql_real_query(mysql ,query2,strlen(query2));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    //t= mysql_real_query(mysql ,query3,strlen(query3));
    //if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    return 0;
}







int  *fun(int *arg)
{
    TT server_msg ;
    int conn_fd = *(int *)arg ;
    int t ;
    memset(&server_msg,0,sizeof(TT));
    t = recv(conn_fd,&server_msg,sizeof(TT),0) ;   ///////////接受信息!!!!!!!!!!!!!!!
    if(t == 0)   
        warning_logout(server_msg ,conn_fd);

    //printf("****************************flag  ==  %d\n",server_msg.flag);
    switch(server_msg.flag)
    {
        case 100: join(server_msg,conn_fd);   break;      //根据类型调用函数
        case 1: sign_in(server_msg,conn_fd);   break ;  //登录
        case 2: add_user_list(server_msg,conn_fd); break;
        case 3: logout(server_msg ,conn_fd) ;  break ;   //下线就修改状态，清空conn_fd 
        case 4: add_friend(server_msg,conn_fd);  break ; 
        case 5: delete_friend(server_msg,conn_fd);  break ; 
        case 6: look_friend(server_msg,conn_fd);  break ; 
        case 7: look_chat_record(server_msg,conn_fd);  break;
        case 8: create_group(server_msg,conn_fd);  break;
        case 9: real_qun_chat(server_msg,conn_fd) ;   break;
        case 10:invite_one(server_msg,conn_fd) ;break ;
        case 11:chat_with(server_msg,conn_fd);   break;
        case 12:add_group(server_msg,conn_fd);   break;
        case 13:jiesan(server_msg,conn_fd);   break;
        case 14:tiren(server_msg,conn_fd);   break;
        case 15:look_group_friend(server_msg,conn_fd);       break;
        case 16:lookup_qun_chat_record(server_msg,conn_fd);  break;
        case 17:exit_qun(server_msg,conn_fd);                break;
        case 18:send_file(server_msg,conn_fd);               break;
        case 19:lookup_offline_record(server_msg,conn_fd);   break;
        default: break;
    }
}


int look_chat_record(TT server_msg,int conn_fd)  // 11 和22 的聊天记录,flag == 7 
{
    int t ,i;
    char query1[100];

    MYSQL_RES *res ;
    MYSQL_ROW row ;
    sprintf(query1, "select  *   from  chat_record where come=%d or come=%d ;",server_msg.QQ,server_msg.to) ;

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);

    res = mysql_store_result(mysql) ;
    t= mysql_num_rows(res);  //t 行
    for(i= 0 ;i< t ;i++)
    {
        row=mysql_fetch_row(res);
        server_msg.QQ = atoi(row[0]) ;
        //server_msg.to = atoi(row[1]) ;
        strcpy(server_msg.str,row[2]);
        strcpy(server_msg.passwd,row[3]);  // 用passwd 来替代时间
        send(conn_fd,&server_msg,sizeof(TT),0) ; //发送
    }
}


int join(TT server_msg ,int conn_fd)       //注册, 一旦注册成功就创建属于它的一张好友表 && 加入系统日志 && 加入用户表
{
    char query1[100];
    char query2[100];
    char query3[100];

    int t ;
    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));
    memset(query3,0,sizeof(query3));
   
    sprintf(query1,"insert into user_data values(%d,'%s','%s');",server_msg.QQ ,server_msg.username ,server_msg.passwd  );
    sprintf(query2,"create table tb_%d(friend_QQ int ,friend_name char(32)) ;",server_msg.QQ) ;      //注册成功，创建一张属于它的表
    sprintf(query3,"insert into system_log (user_QQ,record )   values( %d,'regist  a account ');",server_msg.QQ  );

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0)         
    {
        strcpy(server_msg.str,"重 复 注 册 ！ ！！\n");
        send(conn_fd,&server_msg,sizeof(TT),0 ) ;
    }
    else                       //注册成功！！ 
    {
        strcpy(server_msg.str,"注 册 成 功 ！！,请 登 录 ～ ～\n") ; 
        printf(BLUE" \t\t %d 注 册 了 一 个 帐 号 \n"END,server_msg.QQ);
        send(conn_fd,&server_msg,sizeof(TT),0);  

        mysql_real_query(mysql ,query3,strlen(query3));
        mysql_real_query(mysql ,query2,strlen(query2));
    }
    return 0;
}

int sign_in(TT server_msg ,int conn_fd)    //登录
{
    int t ;
    MYSQL_RES *res ;
    MYSQL_ROW row ;

    char query1[150] ;
    char query2[150] ;
    char query3[150] ;

    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));
    memset(query3,0,sizeof(query3));

    sprintf(query1 ,"select  * from user_data where QQ =%d ;",server_msg.QQ ) ;
    sprintf(query2,"insert into system_log (user_QQ,record )   values( %d,'sign in ');",server_msg.QQ  );
    sprintf(query3 ,"select  * from user_data where QQ =%d and passwd='%s';",server_msg.QQ ,server_msg.passwd) ;

    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;

    if( !res )     myerror("server mysql_store_result",__LINE__) ;

    if(mysql_num_rows(res) ==  0 ) 
    { 
        strcpy(server_msg.str,"用 户 不 存 在 ,请 先 注 册 再 登 录\n"); 
        send(conn_fd,&server_msg,sizeof(TT),0);
        return 0;
    }

    if(search(server_msg.QQ) ==  1)    // 1 返回1 ，代表已登录，返回0 ，代表未登录  search 函数 
    {
        strcpy(server_msg.str,"重 复 登 录 ～ ～ \n");
        send(conn_fd,&server_msg,sizeof(TT),0);
        return 0 ;
    }
   
    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    res = mysql_store_result(mysql) ;

    if(mysql_num_rows(res) == 0)
    {
        strcpy(server_msg.str,"密 码 不 匹 配 ～ ～ \n");
        send(conn_fd,&server_msg,sizeof(TT),0) ;
    }
    else 
    {
        server_msg.flag=  2 ;
        strcpy(server_msg.str," 登  录  成  功 ！ ！ ！\n") ;      // 登录成功，加入链表

        add_list(server_msg,conn_fd) ;
        t= mysql_real_query(mysql ,query2,strlen(query2));  //加入系统日志
        if(t != 0 )    myerror("server mysql_real_query",__LINE__);
        printf(PURPLE" \t\t %d   上  线  了  ～  ～ \n"END,server_msg.QQ);

        //print() ;打印在线用户的函数
        send(conn_fd,&server_msg,sizeof(TT),0); 
    }
    return 0;
}


int delete_friend(TT server_msg,int conn_fd)     // 直接删除
{
    int    t ;
    char query1[150] ;   
    char query2[150] ;
    char query3[150] ;

    memset(query1,0,sizeof(query1));
    memset(query2,0,sizeof(query2));
    memset(query3,0,sizeof(query3));
 
    sprintf(query1 ,"delete from tb_%d where friend_QQ=%d;",server_msg.QQ,server_msg.to ) ;
    sprintf(query2 ,"delete  from tb_%d  where friend_QQ=%d;",server_msg.to,server_msg.QQ) ;
    sprintf(query3 ,"delete  from  chat_record   where come=%d or go= %d;",server_msg.to,server_msg.to) ;
   // printf("query == %s\n",query1 );
    //printf("query == %s\n",query2);
  
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    //printf("t  ==     %d\n",t);
    t= mysql_real_query(mysql ,query2,strlen(query2));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    t= mysql_real_query(mysql ,query3,strlen(query3));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
}

int look_friend(TT server_msg,int conn_fd)
{
    int t ;
    MYSQL_RES *res ;
    MYSQL_ROW row ;

    char query1[150] ;
    memset(query1,0,sizeof(query1));

    sprintf(query1 ,"select  username  from user_data where QQ =%d ;",server_msg.to ) ;
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);

    res = mysql_store_result(mysql) ;
    if( !res )     myerror("server mysql_store_result",__LINE__) ;
    row=mysql_fetch_row(res);
    strcpy(server_msg.str,row[0]) ;
    send(conn_fd,&server_msg,sizeof(TT),0) ;
}


int  online(int number )  //查看对方是否在线,在线返回其套接字，不在线返回0  
{
    U_L *temp ;
    temp = head ;
    while(temp)
    {
        if(temp->QQ  ==  number )
           return temp->list_conn_fd ;
        else
            temp= temp->next ;
    }
    return 0 ;
}



int chat_with(TT server_msg,int  conn_fd)    //flag == 11
{
    int fd ,t ;
    char query1[150] ;
    memset(query1,0,sizeof(query1));

sprintf(query1 ,"insert into chat_record  (come,go,record) values(%d,%d, '%s');",server_msg.QQ ,server_msg.to,server_msg.str ) ; //加入私聊记录
   
    t= mysql_real_query(mysql ,query1,strlen(query1));
    if(t != 0 )    myerror("server mysql_real_query",__LINE__);
    server_msg.state =  1;

    if( (fd = online(server_msg.to) ))  //对方也在线
    {
       // printf("online return == %d \n",online(server_msg)) ;
       //printf("will be send %s\n",server_msg.str) ;
       // printf("fd == %d\n",fd);
        send(fd,&server_msg,sizeof(TT),0)  ;
    }
  else add_one_massage(server_msg,server_msg.QQ,server_msg.to);     //对方不在线,当作离线消息处理
}


int main(void )     //服务器端忽略客户端异常下线的情况
{
    int  i,conn_fd, sock_fd, epfd, nfds  ; //epfd 为epoll 句柄 ,nfds 为发生事件的个数
    int  optval ;
    socklen_t clilen ; 
    struct sockaddr_in clientaddr ;    //客户机地址
    struct sockaddr_in serveraddr ;     //服务器地址
    struct epoll_event ev, events[MAXEVENTS];

    head = init_list() ;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if(sock_fd  <  0 ) myerror("server socket ",__LINE__);
    
    optval  =  1  ; //设置该套接字使之可以重新绑定端口
    if(setsockopt(sock_fd,SOL_SOCKET,SO_REUSEADDR,(int *)&optval,sizeof(int))    < 0)
        myerror("server setsockopt",__LINE__);

    memset(&serveraddr,0, sizeof(struct sockaddr_in)) ;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(SERVER_PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sock_fd,(struct sockaddr *)&serveraddr, sizeof(struct sockaddr_in)) <  0) 
        myerror("server bind ",__LINE__);
    if(listen(sock_fd , LISTENQ) <  0) 
        myerror("server listen ",__LINE__);

    epfd = epoll_create(256);
    ev.data.fd = sock_fd;
    ev.events= EPOLLIN  ;

    mysql = mysql_init(NULL);   //打开数据库
    if(!mysql)   myerror("server mysql_init",__LINE__);
    mysql_connect(mysql) ;
//    char conn[50]= "set user_data utf8" ;  //使数据库支持中文
  //  mysql_real_query(mysql,conn,strlen(conn));
    epoll_ctl(epfd,EPOLL_CTL_ADD,sock_fd,&ev);  //加入

    //signal(SIGPIPE,SIG_IGN);

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

                printf(YELLOW"\t\t accept a connection from %s\n"END,inet_ntoa(clientaddr.sin_addr));
                strcpy(IP,inet_ntoa(clientaddr.sin_addr));     //记录该IP

                ev.data.fd = conn_fd;
                ev.events= EPOLLIN ;
                epoll_ctl(epfd,EPOLL_CTL_ADD,conn_fd,&ev); //epfd------epoll 句柄
            }
            else if(events[i].events&EPOLLIN) //如果是已经连接的用户，并且收到数据，那么进行读入
            {
                //conn_fd =  events[i].data.fd ;
                if(pthread_create(&tid,NULL ,(void *)fun ,(void *)&events[i].data.fd )   < 0  )      
                          myerror("server pthread_create ",__LINE__);
                pthread_detach(tid);                 //在线程外部调用,回收资源
            }
        }
    }
    close_connection(mysql);
    return 0;
}

