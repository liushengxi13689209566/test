#include"myhead.h"
#define  SERVER_IP "127.0.0.1"
  
void  client_join(int conn_fd ) ; //注册函数
int client_sign(int conn_fd)  ; //登录
void  sign_ok(TT client_msg,int conn_fd) ;
void my_friend(TT client_msg,int conn_fd); //我的好友
void my_group(TT client_msg,int conn_fd) ; //
void my_box(TT client_msg,int conn_fd); //
void friend_list(TT client_msg,int conn_fd) ;
int lookup_friends(TT client_msg ,int conn_fd) ;

int main(void)
{
    int   i ;
    int ret ;
    int conn_fd  , choice;
    struct sockaddr_in  serve_addr ;

    memset(&serve_addr,0,sizeof(struct sockaddr_in)) ;
    serve_addr.sin_family = AF_INET ; //TCP 
    serve_addr.sin_port = htons(5201) ; //TCP 
    serve_addr.sin_addr.s_addr= inet_addr(SERVER_IP) ;   //直接设置为本地IP

    conn_fd = socket(AF_INET,SOCK_STREAM ,0 );
    if(conn_fd < 0)  myerror("client socket ",__LINE__);

    if(connect(conn_fd,(struct sockaddr *)&serve_addr,sizeof(struct sockaddr))   < 0)
        myerror("client connect ",__LINE__);  //连接套接字成功 

    do
    {
        printf("\n\n\n\t\t\t 0.注册\n");
        printf("\t\t\t 1.登录\n");        //登录完成再进行各种处理
        printf("\t\t\t 2.取消\n");
        printf("\t\t\t 请输入你的选择：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 0: client_join(conn_fd) ;    break ;
            case 1: client_sign(conn_fd) ;  break ;
            default:printf("输入有错～～～\n");break ;
        }
    }while(choice != 2 ) ; //退出了哦
//    client_msg.flag  =  -1 ;
  //  send()
}

void  client_join(int conn_fd )   //注册函数
{
      char temp[32];
      TT   client_msg ;
      memset(&client_msg,0,sizeof(TT));
      printf("QQ :");
      scanf("%d",&client_msg.QQ);
      printf("%d\n",client_msg.QQ);
      printf("username :");
      scanf("%s",client_msg.username);
      printf("%s\n", client_msg.username);
      do{
            printf("passwd :");
            scanf("%s",temp);
            printf("passwd :");
            scanf("%s",client_msg.passwd);
         if(strcmp(temp ,client_msg.passwd) != 0)
             printf("两次密码不同！！\n");
      }while(strcmp(temp,client_msg.passwd) !=  0 ) ;
    client_msg.flag = 0 ;  //表示注册
    send(conn_fd,&client_msg,sizeof(TT),0 ) ; //等待验证
    recv(conn_fd,&client_msg,sizeof(TT),0 ) ; //判断验证
    printf("%s\n",client_msg.str);
}
int client_sign(int conn_fd)  //登录
{
    TT client_msg ;
    char str[32];
    memset(&client_msg,0,sizeof(TT));
    printf("QQ :");
    scanf("%d",&client_msg.QQ);
    printf("passwd :");
    scanf("%s",client_msg.passwd);
    client_msg.flag =  1;

    send(conn_fd,&client_msg,sizeof(TT), 0); //登录等待
    recv(conn_fd,&client_msg,sizeof(TT),0);
     printf("recv is %s \n",client_msg.str);
    if(client_msg.flag ==   2 )   // 代表登录成功！！
        sign_ok(client_msg,conn_fd); //登录成功把该结构体和套接字都传过曲，以以免后面用到
}

void main_menu()
{
    printf("\n\n\n\n\n\n\t\t\t1:我的好友\n");
    printf("\t\t\t2:我的群\n");
    printf("\t\t\t3:消息盒子\n");
    printf("\t\t\t4:退出登录\n");
}
void  sign_ok(TT client_msg,int conn_fd)    // 只要登录了它的QQ 套接字等信息就都会一直用下去
{
    int choice ;
   do
   {
        main_menu(); //打印主菜单
        printf(" 请输入你的选择 ： ");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:my_friend(client_msg,conn_fd); break;
            case 2:my_group(client_msg,conn_fd)   ;break;
            case 3:my_box(client_msg,conn_fd) ;   break ;
            case 4:   break ;
        }
   }while(choice !=  4 );
   return  ;
}
/*************************************我的好友***************************************/
void my_friend_menu()
{
    printf("\n\n\n\n\n\t\t\t1:添加好友\n");
    printf("\t\t\t2:删除好友\n");
    printf("\t\t\t3:好友列表\n");
    printf("\t\t\t4:退出\n");
}
void  friend_list_menu()  //好友最终选项
{
    printf("\n\n\n\n\n\t\t\t1:与他聊天\n");
    printf("\t\t\t2:查看资料\n");
    printf("\t\t\t3:查看聊天记录\n");
    printf("\t\t\t4:发送文件\n");
    printf("\t\t\t5:查看全部好友\n");
    printf("\t\t\t6:退出\n");
}
void friend_list(TT client_msg,int conn_fd)
{
    int choice ;
    do
    {
        friend_list_menu()   ;
        printf("请输入你的选择 ：");
        scanf("%d",&choice);
        printf("choiice  == %d \n",choice);
        switch(choice)
        {
            case 1:chat_with(client_msg ,conn_fd);break;
            case 2:/*lookup_data(client_msg ,conn_fd );*/break;
            case 3:/*lookup_chat_reconds(client_msg ,conn_fd); */  break ;
            case 4:/*send_file(client_msg , conn_fd  ;)*/      break ;
            case 5: lookup_friends(client_msg ,conn_fd);   break ;
            case 6: break ;
        }
    }while(choice != 6);
    return ;
}
void add_friend(TT client_msg ,int conn_fd)
{
   
    printf("请输入你想要添加好友的QQ 号: ") ;
    scanf("%d",&client_msg.num );   //num 代表对方的QQ号
    client_msg.flag=  4 ;  
    send(conn_fd,&client_msg,sizeof(TT),0) ; //发送给服务器消息，服务器进行验证，然后发送消息回来，加入我的好友列表
    recv(conn_fd,&client_msg,sizeof(TT),0) ; // 接受消息,如果对方离线，就会一直阻塞在这里 ,估计要开线程解决----
    printf("recv is %s\n",client_msg.str);

    if(client_msg.flag ==  5)
    {
        printf("添加好友成功！！，现在可以和它愉快的聊天了\n");
    }
    if(client_msg.flag ==   6) 
    {
        printf("对方拒绝了你的请求，并对你放了一个屁\n");
    }
}

void delete_friend(TT client_msg ,int conn_fd)
{
   
    printf("请输入你想要删除好友的QQ 号: ") ;
    scanf("%d",&client_msg.num );   //num 代表对方的QQ号
    client_msg.flag=  7 ;  
    send(conn_fd,&client_msg,sizeof(TT),0) ; //发送给服务器消息，服务器进行验证，然后发送消息回来，加入我的好友列表
    recv(conn_fd,&client_msg,sizeof(TT),0) ; // 接受消息,如果对方离线，就会一直阻塞在这里 ,估计要开线程解决----
    printf("recv is %s\n",client_msg.str);
}
int lookup_friends(TT client_msg ,int conn_fd) //problems
{
    TT temp ;
    printf("into lookup_friends");
    memset(&temp,0,sizeof(TT));
    client_msg.flag =  9  ;  
    send(conn_fd,&client_msg,sizeof(TT),0) ; 
    printf("\t\t\tfriend_QQ \tfriend_name\n");    
    while(1)
    {
        if(temp.flag  ==  10)          return  0;  //end    of    print
        recv(conn_fd,&temp ,sizeof(TT),0) ;   //接受数据并打印出来
        printf("\t\t\t%d\t%s\n",temp.num ,temp.str);
    }
}
int chat_with(client_msg ,conn_fd) 
{
    printf("please input the QQ you want to chat_with :");
    scanf("%d",&client_msg.num );     //代表对方的QQ号
    send(conn_fd,&client_msg,sizeof(TT),0) ;
    recv(conn_fd,&client_msg,sizeof(TT),0) ;
    printf("%s\n",client_msg.str) ;    //print information
    if(client_msg.flag == 12)   /*   Verification   success*/
    {
        client_msg.flag  = 14 ;
        chat(client_msg,conn_fd);
    }
}
void chat(TT client_msg.int conn_fd)      //flag  ==  14
{
    while(1)
    {
        printf("pleaes  input the message :");
        scanf("%s",client_msg.str);
        if(strcmp(client_msg.str,"exit") == 0 )     return   ;
        send(conn_fd,&client_msg,sizeof(TT),0) ;
        recv(conn_fd,&client_msg,sizeof(TT),0)  ;
        printf("##### %s \n",client_msg.str);   //print message 
    }
}

void my_friend(TT client_msg,int conn_fd)
{
    int choice ;
    do
    {
        my_friend_menu();
        printf("请输入你的选择 ：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:add_friend(client_msg,conn_fd); break;
            case 2:delete_friend(client_msg ,conn_fd); break;
            case 3:friend_list(client_msg ,conn_fd) ; break ;
            case 4:break ;
        }
    }while(choice !=  4 );
    return  ;
}
/***************************************我的群************************************/
void my_group_menu()
{
    printf("\n\n\n\n\n\t\t\t1:创建群\n");
    printf("\t\t\t2:我加入的群\n");
    printf("\t\t\t3:退出\n");
}
void my_add_group_menu()
{
    printf("\n\n\n\n\t\t\t1:查看所有群\n");
    printf("\t\t\t2:邀请新成员进群\n");
    printf("\t\t\t3：查看聊天记录\n");
    printf("\t\t\t4:退出\n");
}
void my_add_group(int conn_fd)
{
    int choice ;
    do
    {
        my_add_group_menu();
        printf("请输入你的选择：") ;
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:/*create_group(conn_fd);*/ break;
            case 2:  break;
            case 3:break;
        }
    }while(choice !=  4 );
    return ;
}
void my_group(TT client_msg,int conn_fd)
{
    int choice ;
    do
    {
        my_group_menu();
        printf("请输入你的选择：") ;
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:/*create_group(conn_fd);*/ break;
            case 2:my_add_group(conn_fd);  break;
            case 3:break;
        }
    }while(choice !=  3 );
    return ;
}
/*************************************消息盒子**********************************/
void my_box(TT client_msg,int conn_fd)
{
    printf("消息都再这里哦！！！！～～～\n");
    return ;
}


    


