/*********************************************登录成功***********************************/
#include<stdio.h>
void my_friend_menu();
void my_group(int conn_fd);
void  sign_ok(int conn_fd) ;
void my_friend(int conn_fd);
void my_box(int conn_fd) ;
int main()
{
    sign_ok(52);
    return 0;
}
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
   do
   {
        main_menu(); //打印主菜单
        printf(" 请输入你的选择 ： ");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:my_friend(conn_fd);break;
            case 2:my_group(conn_fd);break;
            case 3:my_box(conn_fd) ;   break ;
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
void friend_list(int conn_fd)
{
    int choice ;
    do
    {
        friend_list_menu()   ;
        printf("请输入你的选择 ：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:/*add_friend(conn_fd);*/break;
            case 2:/*delete_friend(conn_fd);*/break;
            case 3:/*lookup_friends(conn_fd);*/break ;
            case 4:/*send_file(int conn_fd  ;)*/   break ;
            case 5:     break ;
            case 6: break ;
        }
    }while(choice != 6);
    return ;
}
void my_friend(int conn_fd)
{
    int choice ;
    do
    {
        my_friend_menu();
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
/*************************************消息盒子**********************************/
void my_box(int conn_fd)
{
    printf("消息都再这里哦！！！！～～～\n");
    return ;
}
