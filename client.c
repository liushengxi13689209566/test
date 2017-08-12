#include"myhead.h"
  
void  client_join(int conn_fd ) ; //注册函数
int client_sign(int conn_fd)  ; //登录
void sign_ok(int conn_fd); //登录成功
void my_friend(int conn_fd); //我的好友
void my_group(int conn_fd) ; //
void my_box(int conn_fd); //

int main(void)
{
    int   i ;
    TT client_msg ;
    int ret ;
    int conn_fd  , choice;
    struct sockaddr_in  serve_addr ;

    memset(&client_msg,0,sizeof(TT));
    memset(&serve_addr,0,sizeof(struct sockaddr_in)) ;
    serve_addr.sin_family = AF_INET ; //TCP 
    serve_addr.sin_port = htons(5201) ; //TCP 
    serve_addr.sin_addr.s_addr= inet_addr("127.0.0.1") ;   //直接设置为本地IP

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
//    send_buf.flag  =  -1 ;
  //  send()
    
}

  void  client_join(int conn_fd ) //注册函数
  {
      char temp[32];
      TT recv_buf ,send_buf ;
      int t =  1  ;
      memset(&recv_buf,0,sizeof(TT));
      memset(&send_buf,0,sizeof(TT));
      printf("QQ :");
      scanf("%d",&send_buf.QQ);
      printf("%d\n",send_buf.QQ);
      printf("username :");
      scanf("%s",send_buf.username);
      printf("%s\n", send_buf.username);
      do{
            printf("passwd :");
            scanf("%s",temp);
            printf("passwd :");
            scanf("%s",send_buf.passwd);
         if(strcmp(temp ,send_buf.passwd) != 0)
         {
             printf("两次密码不同！！\n");
             t = 0 ;
         }
      }while(t ==  0 ) ;
    send_buf.flag = 0 ;  //表示注册
    send(conn_fd,&send_buf,sizeof(TT),0 ) ;//等待验证
    recv(conn_fd,&recv_buf,sizeof(TT),0 ) ;//判断验证
    printf("%s\n",recv_buf.str);
  }
int client_sign(int conn_fd)  //登录
{
    TT send_buf,recv_buf;
    char str[32];
    memset(&send_buf,0,sizeof(TT));
    memset(&recv_buf,0,sizeof(TT));
    printf("QQ :");
    scanf("%d",&send_buf.QQ);
    printf("passwd :");
    scanf("%s",send_buf.passwd);
    send_buf.flag =  1;
    send(conn_fd,&send_buf,sizeof(TT), 0); //登录等待
    recv(conn_fd,&recv_buf,sizeof(TT),0);
    printf("recv is %s \n",recv_buf.str);
    if(recv_buf.flag ==  4)   // 代表登录成功！！
        sign_ok(conn_fd);
}



    

/*********************************************登录成功***********************************/
void main_menu()
{
    printf("\n\n\n\n\n\n\t\t\t1:我的好友\n");
    printf("\t\t\t2:我的群\n");
    printf("\t\t\t3:消息盒子\n");
    printf("\t\t\t4:退出登录\n");
}
void  sign_ok(int conn_fd)
{
    int choice ;
    main_menu(); //打印主菜单
   do
   {
        printf(" 请输入你的选择 ： ");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:my_friend(conn_fd);break;
            case 2:my_group(conn_fd);break;
            case 3:/*my_box(conn_fd) ;*/   break ;
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
    printf("\t\t\t5:退出\n");
}
void friend_list(int conn_fd)
{
    int choice ;
   friend_list_menu()   ;
    do
    {
        printf("请输入你的选择 ：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:/*add_friend(conn_fd);*/break;
            case 2:/*delete_friend(conn_fd);*/break;
            case 3:/*lookup_friends(conn_fd);*/break ;
            case 4:/*send_file(int conn_fd  ;)*/   break ;
            case 5: break ;
        }
    }while(choice != 5);
    return ;
}

void my_friend(int conn_fd)
{
    int choice ;
    my_friend_menu();
    do
    {
        printf("请输入你的选择 ：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:/*add_friend(conn_fd);*/break;
            case 2:/*delete_friend(conn_fd);*/break;
            case 3:friend_list(conn_fd);break ;
            case 4:break ;
        }
    }while(choice != 4 );
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
void my_group(int conn_fd)
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
/*void group(int conn_fd)
{
    int choice ;
    do
    {
        group_menu();
        printf("请输入你的选择：")
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:create_group(conn_fd); break;
            case 2:add_group(conn_fd);  break;
        }
    }while(choice !=  3);
    return ;
}
************************************消息盒子**********************************/
void box(int conn_fd)
{
    printf("消息都再这里哦！！！！～～～\n");
    return ;
}


