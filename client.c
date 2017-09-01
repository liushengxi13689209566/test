
#include"myhead.h"



typedef struct User_list
{
    int QQ ;
    struct User_list *next;
}U_L;

int  I_conn_fd ;    //全局变量来表示客户端与服务器的连接套接字 ，反正一个客户端就一个套接字嘛
int  I_QQ ;         //代表自己的QQ号
int  to_who ;
U_L  *head ; //好友列表头指针
TT  *mhead ; //消息盒子头指针
int  qun_num ; //群号
 

TT  *init_mhead()
{
    TT *temp ;
    temp=(TT *)malloc(sizeof(TT));
    temp->next = NULL ;
    return temp ;
}

U_L  *init_user_list()
{
    U_L  *temp ;
    temp=(U_L *)malloc(sizeof(U_L));
    temp->next = NULL ;
    return temp ;
}

int  client_join() ;        //注册函数  0
int client_sign()  ;          //登录    1
int  sign_ok(TT client_msg) ; //登录成功 2   
int my_friend(TT client_msg);  //我的好友
int my_group(TT client_msg) ; //我的群
int my_box(TT client_msg );    //我的消息盒子
int friend_list(TT client_msg ) ; //好友选项
int chat(TT client_msg )  ;     //flag  ==  14
int chat_with(TT client_msg ) ;
int search_friend(int number) ;
int add_user_list(int number) ;
int print();  //打印好友链表
int read_mysql_to_list(TT client_msg);  //flag ==  2 
int lookup_data(TT client_msg) ; //flag ==  6 ;
int print_friends(TT client_msg);
int lookup_chat_reconds(TT client_msg) ;
int real_qun_chat(TT client_msg) ;
int  invite_one(TT client_msg) ;
int keep_file(TT client_msg) ;


int add_mhead(TT client_msg)   // 加入消息盒子的函数
{
    TT *temp ;
    temp=(TT *)malloc(sizeof(TT)) ;
    temp->flag = client_msg.flag ;
    temp->state = client_msg.state ;
    temp->QQ = client_msg.QQ ;
    strcpy(temp->username,client_msg.username);
    strcpy(temp->passwd,client_msg.passwd);
    strcpy(temp->str,client_msg.str);
    temp->to = client_msg.to ;
    temp->next = mhead->next ;
    mhead->next = temp ;
}

int  client_join(int conn_fd )     //注册函数
{
    char temp[32];
    TT   client_msg ;
    memset(&client_msg,0,sizeof(TT));

    setbuf(stdin,NULL);

    printf("\033c");
    printf(GREEN"\n\n\n\n\n\n\t\t\t\t注          册\n\n"END) ;
    printf(BLUE"\t\t\t\t   QQ :"END) ;
    scanf("%d",&client_msg.QQ);
    printf(PURPLE"\t\t\t\tusername:"END);
    scanf("%s",client_msg.username);
    system("stty -echo");
    do{
            printf(RED"\t\t\t\tpasswd :"END) ;
            scanf("%s",temp);

            getchar();

            printf(RED"\n\t\t\t\tpasswd :"END) ;
            scanf("%s",client_msg.passwd);
            if(strcmp(temp ,client_msg.passwd) != 0)
             printf("两次密码不同！！\n");
    }while(strcmp(temp,client_msg.passwd) !=  0 ) ;
    system("stty echo");

    client_msg.flag = 100 ;  //表示注册

    send(conn_fd,&client_msg,sizeof(TT),0 ) ; //等待验证

    recv(conn_fd,&client_msg,sizeof(TT),0 ) ; //判断验证
    printf(GREEN"\n\t\t\t\t%s\n"END,client_msg.str);    //打印注册成功或者失败信息
    sleep(2);
}
int client_sign(int conn_fd)    //登录
{
    TT client_msg ;
    char str[32];
    memset(&client_msg,0,sizeof(TT));

    setbuf(stdin,NULL);

    printf("\033c");
    printf(YELLOW"\n\n\n\n\n\n\t\t\t\t登       录\n\n"END) ;
    printf(BLUE"\t\t\t\t   QQ :"END) ;
    scanf("%d",&client_msg.QQ);
    printf(RED"\t\t\t\tpasswd :"END) ;

    getchar();

    system("stty -echo");
    scanf("%s",client_msg.passwd);
    system("stty echo");

    client_msg.flag  =  1;

    send(conn_fd,&client_msg,sizeof(TT), 0); //登录等待

    recv(conn_fd,&client_msg,sizeof(TT),0);
    printf(GREEN"\n\t\t\t\t%s\n"END,client_msg.str);     //打印登录成功或者失败的信息
    sleep(2);
    if(client_msg.flag  ==  2 )     // 代表登录成功！！
    {
       I_QQ = client_msg.QQ ;

       read_mysql_to_list(client_msg);      //把数据库中的好友放入链表中;
       /*printf("所有的好友为：\n");
       print() ;*/

       // read_outline_to_box();   //把离线消息放入消息盒子;
       sign_ok(client_msg ) ;     //登录成功把该结构体传过曲，以免后面用到
    }
}

int read_mysql_to_list(TT client_msg)  //flag ==  2 
{
 
    send(I_conn_fd,&client_msg,sizeof(TT),0);
    while(1)
    {
        recv(I_conn_fd,&client_msg,sizeof(TT),0) ;
        printf("into read_mysql_to_list and client_msg.to == %d\n",client_msg.to);
        add_user_list(client_msg.to) ;
        if(client_msg.state ==  -8)  break;     //发完为止
    }
}

int get_one_chat(TT client_msg )
{
    if(client_msg.QQ  ==  to_who )
    {
        //如果是当前用户则直接输出
        printf("\t\t\t\tfriend-\e[1;34m%d\e[0m :\e[1;33m%s\e[0m\n",client_msg.QQ,client_msg.str);
    }else{
            printf(PURPLE"\t\t\t\t%d发来一条消息,请到消息盒子处理\n"END,client_msg.QQ);
            add_mhead(client_msg);
        //不是当前聊天的用户就放进消息盒子*/
    }
}



int get_group_chat( TT client_msg)
{
    if(client_msg.to == qun_num)
    {
        //如果是当前群则直接输出
        printf("\t\t\t\t\e[1;34m%d\e[0m：\e[1;33m%s\e[0m\n",client_msg.QQ,client_msg.str);
        //printf("\t\t%s",client_msg.str);  原来写的语句
        //printf(YELLOW" <-%s"WHITE,mes.from);
    }else{
            printf(PURPLE"\t群%d发来一条消息,请到消息盒子处理\n"END,client_msg.to);
            add_mhead(client_msg);
        //不是当前聊天的群就放进消息盒子
    }
}

int get_one_file( TT client_msg)
{
    printf("**************************************************************************\n");
    if(client_msg.to  ==  to_who)
    {
        printf(PURPLE"\t 好友%d 给你发来文件%s\n"END,client_msg.QQ,client_msg.passwd);
    }else{
            printf(PURPLE"\t好友%d发来一个文件,请到消息盒子处理\n"END,client_msg.QQ );
            add_mhead(client_msg);
        //不是当前聊天的群就放进消息盒子
    }
}

int my_recv(void)
{
    //只用来接收信息的子线程
    while(1) {
        TT  massage ;
        memset(&massage ,0,sizeof(TT ));
        int re ;
        re = recv(I_conn_fd,&massage ,sizeof(TT ),0);
        if(re == 0) {
            puts("服务器开始维护,你已断开连接!");
            exit(0);
        } else if(re< 0){
            myerror("recv",__LINE__);
            exit(0);
        }
        switch(massage.flag) {
        case 11 :
            //私聊消息
            get_one_chat( massage );
            break;
        case 9:
            get_group_chat(massage);  //群聊消息
            break;
        case 12:
            if(massage.state ==  -1) {
                printf(BLUE"\t%d申请加入群%d,请到消息盒子处理\n"END,massage.QQ,massage.to);
                add_mhead(massage);
            } else if(massage.state == 2) {
                printf(RED"群主%d拒绝你进入群%d\n"END,massage.QQ ,massage.to);
            } else if(massage.state == 1) {
                printf(RED"群主%d同意你进入群%d\n"END,massage.QQ ,massage.to);
            } else if(massage.state == -2) {
                printf(RED"你已是该群成员,无需重复添加\n"END);
                sleep(1);
            }
            break;
        case 13:
            if(massage.state ==  -2 ) 
                  printf(BLUE"\t 你 不 是 群 主， 无 法 解 散 群 \n"END,massage.to);
            else  printf(RED"\t群%d  已 被 解散！！！ \n"END,massage.to);
            sleep(1);
            break;
        case 14:
            if(massage.state ==  -2 ) 
                printf(BLUE"\t你不是群主，无法踢人，把你牛逼的\n"END);
            else if(massage.state == -1 )
                printf(RED"\t好友%d 没在群%d 中\n"END,massage.num,massage.to);
            else if(massage.state ==   1) 
                printf(RED"\t已 踢 除%d\n"END,massage.num);
            else if(massage.state == 2)
                printf(RED"\t你被群主%d '踢' 出群%d \n"END,massage.QQ,massage.to);
            sleep(1);
            break;
        case 15:
            if(massage.state ==  -2 ) 
                printf(BLUE"\t你不在该群内，无法查看群成员\n"END);
            else 
                printf(BLUE"\t\tQQ : "END) ;
                printf("  %d \n",massage.num) ;
            break;
        case 16:
            printf("\e[1;33m%d\e[0m :\e[1;34m%s\e[0m at %s\n",massage.QQ,massage.str,massage.passwd);
            break;
        case 18:
            if( massage.state ==   -1 )   //正在传输
                    keep_file(massage);
            else    get_one_file(massage);  //传完文件后
            break;
        case 19:    //查看离线消息，私聊与群聊
           if(massage.state ==  1)
            {
                printf("\e[1;33m好友 %d\e[0m :\e[1;34m%s\e[0m \n",massage.to,massage.str );
            }
            else if(massage.state == -1 )
            {
                printf("\e[1;33m 群%d\e[0m :\e[1;34m%s\e[0m  \n",massage.to,massage.str );
            }
            break;
        case  4:
            //加好友
            /*if(massage.state ==  -2 ) {
                printf("%d已经是你的好友，无需重复添加！！\n",massage.to);
            }
            else*/ if(massage.state ==  -1) {
                printf(BLUE"\t%d请求加你为好友,请到消息盒子处理\n"END,massage.QQ);
                add_mhead(massage);
            } else if(massage.state == 2) {
                printf(RED"%d拒绝了你的好友请求\n"END,massage.QQ);
            } else if(massage.state == 1) {
                printf(YELLOW"%d同意了你的好友请求\n"END,massage.QQ);
                add_user_list(massage.QQ) ;  //加入好友的链表
                //print() ; //打印好友链表
            }
            sleep(1);
            break;
        case 6:
            printf(GREEN"QQ :%d | name :%s\n"END,massage.to,massage.str);
            sleep(1);
            break;
        case 7:
            printf("\e[1;33m%d\e[0m :\e[1;34m%s\e[0m at %s\n",massage.QQ,massage.str,massage.passwd);
            break;
        case 10:
            //邀人
            if(massage.state ==  -1) {
                printf(BLUE"\t%d邀请你加入群%d,请到消息盒子处理\n"END,massage.QQ,massage.to);
                add_mhead(massage);
            } else if(massage.state == 2) {
                printf(RED"\t%d拒绝进入群%d\n"END,massage.num ,massage.to);
            } else if(massage.state == 1) {
                printf(RED"\t%d同意进入群%d\n"END,massage.num ,massage.to);
            } else if(massage.state == -2) {
                printf(RED"\t%d已是该群成员,无需重复邀请\n"END,massage.num);
            }
            break;
        }
    }
}

int logout(TT client_msg ,int conn_fd)  //只发送下线信号
{
    client_msg.flag =  3 ;
    printf("client_msg.state == %d \n",client_msg.state);
    printf(RED"\t\t\t\t下 线 请 求 已 发 送 ，请 耐 心 等 待 ～ ～ ～  "END ) ;
    send(conn_fd,&client_msg,sizeof(TT),0) ;
    sleep(2);
} 




int lookup_offline_record(TT client_msg)
{
    client_msg.flag  = 19 ;
    printf("\n\n\n");
    send(I_conn_fd,&client_msg,sizeof(TT),0);
    sleep(8);
    return 0;

}




int  sign_ok(TT client_msg )    // 只要登录了它的QQ 套接字等信息就都会一直用下去,开线程了
{
    int choice ;
    pthread_t thid;
    pthread_create(&thid,NULL,(void *)my_recv,NULL);
    do
    {
       // printf("\033c");
        printf(GREEN"\n\n\n\n\n\n\t\t\t\t1:我的好友\n\n");
        printf("\t\t\t\t2:我的群\n\n");
        printf("\t\t\t\t3:消息盒子\n\n");
        printf("\t\t\t\t4.查看离线消息\n\n");
        printf("\t\t\t\t5:退出登录\n\n"END);
        printf(YELLOW"\t\t\t\t请输入你的选择： "END);
        scanf("%d",&choice);
        switch(choice)
        {
            case 1: my_friend(client_msg );             break ;
            case 2: my_group(client_msg);               break ;
            case 3: my_box(client_msg ) ;               break ;
            case 4: lookup_offline_record(client_msg);  break ;
            case 5: client_msg.state = 1;    break ;
            default :printf(RED"\n\n\t\t\t\t输入有错～～～,请检查后重新输入！！！\n"END);  sleep(1) ;  break ;
        }
   }while(choice !=  5 );
   logout(client_msg,I_conn_fd);
   return 0 ;
}
/*************************************我的好友***************************************/






int keep_file(TT client_msg)
{
    char file_name[100];

    //printf("into keep_file\n") ;

    /*printf("client_msg.passwd == %s \n",client_msg.passwd);
    printf("client_msg.QQ == %d \n",client_msg.QQ);
    printf("client_msg.to == %d \n",client_msg.to);
    printf("client_msg.num == %d \n",client_msg.num);
    printf("client_msg.state == %d \n",client_msg.state);
    printf("client_msg.str == %s \n",client_msg.str);*/

    int fd  ;
    sprintf(file_name,"/home/liushengxi/test/test_1/%d_%s",I_QQ,client_msg.passwd);

    if( ( fd = open(file_name,O_RDWR | O_APPEND | O_CREAT ,S_IRUSR | S_IWUSR) ) < 0 )
        myerror("in keep_file open ",__LINE__);

    write(fd ,client_msg.str,client_msg.num);

    close(fd);
}

int send_file(TT client_msg ,int file_fd)
{
    int sum = 0 ,file_len = 0,sum_len = 0  ;     //sum_len 总文件长度，file_len 每次发送的文件长度 
    int  i ;
    char temp[32];
    char read_buf[129] ;

    memset(read_buf,0,sizeof(read_buf));
    memset(temp,0,sizeof(temp));

    strcpy(temp,client_msg.passwd); 

    lseek(file_fd,0,SEEK_SET) ;
    if( (sum_len = lseek(file_fd,0,SEEK_END)) < 0 )
        myerror("lseek in send_file",__LINE__);
    lseek(file_fd,0,SEEK_SET) ;
    printf(YELLOW"\n\n\t\t\t\t传 输 文 件 开 始 > > > > > "END);
    while(sum != sum_len )     //服务器不需要知道文件有多大，发完就完了啊
    {
   
        memset(read_buf,0,sizeof(read_buf));
        memset(client_msg.str,0,sizeof(client_msg.str));

        file_len=read(file_fd,read_buf, 128 ) ;

        memcpy(client_msg.str,read_buf,file_len);   //把文件内容拷贝到client.msg.str
        sum = sum + file_len ;

        strcpy(client_msg.passwd,temp);
        client_msg.num = file_len ;
        client_msg.to = to_who ;
        client_msg.QQ = I_QQ ;
        client_msg.flag   = 18 ;
        client_msg.state  = -1 ;

        send(I_conn_fd,&client_msg,sizeof(TT),0) ;
        usleep(100000);
    }
   
    printf(BLUE"\n\n\t\t\t\t传 输 文 件 结 束 < < < < "END);
    strcpy(client_msg.passwd,temp);
    client_msg.to = to_who ;
    client_msg.QQ = I_QQ ;
    client_msg.flag   = 18 ;
    client_msg.state  =  1 ;
   
    send(I_conn_fd,&client_msg,sizeof(TT),0) ;

    //to_who = 0 ;

    return 0;
}

int init_file(TT client_msg)
{
    char name[256] ;
    int file_fd ;
    int i ,j;
    memset(name,0,sizeof(name));

    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t 请 输 入 你 想 要  发 送 文 件 的 好 友 的 QQ 号 :" ) ;
    scanf("%d",&client_msg.to);

    to_who = client_msg.to ;

    if(search_friend(client_msg.to) == 0 )
    {    
        printf(RED"\n\n\\t\t\t\t 他 / 她 还 不  是 你 的 好 友，无 法 发 送 文 件 \n"END);
        sleep(1);
        return 0;
    }
    printf("\n\n\t\t\t\t请 输 入 你 想 要  发 送 文 件 的 绝 对 路 径 : ");
    scanf("%s",name);
    if( ( file_fd = open(name,O_RDONLY)) < 0 )
    {
        printf(RED"\n\n\t\t\t\t 文 件 不 存 在 啊 ！！ \n"END);
        sleep(1);
        return 0;
    }

    int temp ;
    temp = strlen(name);
    for(i= 0,j=0 ;i < temp;i++)
    {
        if(name[i] == '/')
        {   
            j= 0;
            continue;
        }
        client_msg.passwd[j++] = name[i];
    }
    client_msg.passwd[j]='\0';

    send_file(client_msg,file_fd);

    return 0;
}


int  friend_list(TT client_msg )
{
    int choice ;
    do
    {
        //printf("\033c");
        printf(GREEN"\n\n\n\n\n\n\t\t\t\t1:与他/她聊天\n\n");
        printf("\t\t\t\t2:查看好友资料\n\n");
        printf("\t\t\t\t3:查看私聊记录\n\n");
        printf("\t\t\t\t4:发送给好友文件\n\n");
        printf("\t\t\t\t5:查看全部好友\n\n");
        printf("\t\t\t\t6:退出\n\n"END);
        printf("\t\t\t\t请输入你的选择 ：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:chat_with(client_msg ) ;          break ;
            case 2:lookup_data(client_msg  );        break ;
            case 3:lookup_chat_reconds(client_msg);  break ; 
            case 4:init_file(client_msg );           break ;
            case 5:print_friends(client_msg );       break ;
            case 6: break ;
            default :printf(RED"\n\n\t\t\t\t输入有错～～～,请检查后重新输入！！！\n"END);  sleep(1) ;  break ;
        }
    }while(choice != 6);
    return 0;
}

int lookup_chat_reconds(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你要查看聊天记录的好友的QQ号：");
    scanf("%d",&client_msg.to);
    if(search_friend(client_msg.to) ==  0 )
    {
        printf(RED"\n\n\t\t\t\t对方还不是你的好友，无聊天记录\n"END) ;
        sleep(1);
        return 0;
    }

    client_msg.flag =  7 ;
    send(I_conn_fd,&client_msg,sizeof(TT),0) ;
    sleep(10);
    return 0;
}

int chat_with(TT  client_msg ) 
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t 请 输 入 对 方 QQ ：");
    scanf("%d",&client_msg.to);     //代表对方的QQ号
    to_who  =  client_msg.to ;

    if(search_friend(client_msg.to)   ==   0 )
    {
        printf(RED"\n\n\t\t\t\t对方还不是你的好友,无法聊天 ！\n"END) ;
        sleep(1);
        return  0;
    }

    //printf("to_who  == %d \n",to_who);
    client_msg.flag  =  11 ;
    printf(YELLOW"\n\n\t请输入你要发的消息(按回车键发送,输入exit 退出私聊)：\n"END);
    while(1)
    {
        printf(GREEN"I : "END);
        scanf("%s",&client_msg.str) ;
        if(strcmp(client_msg.str,"exit") ==  0 )
        {
            to_who = 0 ;
            return 0;
        }
        send(I_conn_fd,&client_msg,sizeof(TT),0);
    }
}

int lookup_data(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你想要查看的好友的QQ 号：") ;
    scanf("%d",&client_msg.to) ;
   
    if(search_friend(client_msg.to) ==  0 )
    {
        printf(RED"\n\n\t\t\t\t对方还不是你的好友，无法查看!!\n"END) ;
        sleep(1);
        return 0;
    }

    client_msg.flag =  6 ;   //6  代表查看某一个好友

    send(I_conn_fd ,&client_msg,sizeof(TT),0);     
    sleep(3);
}

int print_friends(TT client_msg)
{
    U_L *temp= head->next ;
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\tfriend-QQ\n");
    while(temp)
    {
        printf(YELLOW"\t\t\t\t %d \n"END,temp->QQ);
        temp=temp->next;
    }
    sleep(6);
    return 0;
}

int search_friend(int number)
{
    U_L *temp = head->next ;
    while(temp)
    {
        if(temp->QQ == number)
            return  1;
        temp=temp->next ;
    }
    return 0 ;
}

int add_friend(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你想要添加的好友的QQ 号： ") ;
    scanf("%d",&client_msg.to) ;
    if(client_msg.to == I_QQ)  
    {
        printf(RED"\n\n\t\t\t\t不 能 添 加 自 己 ！！！\n"END) ;
        sleep(1);
        return 0 ;
    }
    if(search_friend(client_msg.to) ==  1)
    {
        printf(RED"\n\n\t\t\t\t他已经是你的好友了，无需重复添加！\n"END) ;
        sleep(1);
        return 0;
    }

    client_msg.flag = 4 ;  //4  代表添加好友
    client_msg.state = -1 ;

    send(I_conn_fd ,&client_msg,sizeof(TT),0);      //对方可同意也可拒绝～～ 

    printf(YELLOW"\n\n\t\t\t\t好友添加已发送，请耐心等待。。。。\n"END);
    sleep(1);
}

int  destory_one(int number)
{
    U_L *pre =head ;
    U_L *temp = head->next ;
    while(temp)
    {
        if(temp->QQ == number)
        { 
            pre->next = temp->next ;
            free(temp);
        }
        temp=temp->next ;
        pre=pre->next ;
    }
    return 0 ;
}

int delete_friend(TT client_msg)
{
    printf("\033c");
    printf(RED"\n\n\n\n\n\n\t\t\t\t 删 除 好 友 的 同 时 也 将 删 除 你 和 他 的 聊 天 记 录  ！ ！ ！ \n\n "END);
    printf("\t\t\t\t请 输 入 你 想 要 删 除 的好 友 的 QQ   号 ： ") ;
    scanf("%d",&client_msg.to) ;
    if(client_msg.to == I_QQ)  
    {
        printf(RED"\n\n\t\t\t\t无法删除自己！！！\n"END) ;
        sleep(1);
        return 0 ;
    }
    if(search_friend(client_msg.to) ==   0 )
    {
        printf(RED"\n\n\t\t\t\t对方不是你的好友，无法删除\n"END) ;
        sleep(1);
        return 0;
    }

    /*printf("client_msg.QQ == %d\n",client_msg.QQ);
    printf("client_msg.to == %d\n",client_msg.to);*/

    client_msg.flag = 5 ;  //5  代表删除好友
    //client_msg.state = -1 ;
    destory_one(client_msg.to);  //从好友链表中删除他

    send(I_conn_fd ,&client_msg,sizeof(TT),0);       
    printf(YELLOW"\n\n\t\t\t\t 已 删 除 好 友 ～ ～ ， 友 谊 珍 贵 请 珍 惜 \n"END);
    sleep(1);
}

int my_friend(TT client_msg )
{
    int choice ;
    do
    {
        //printf("\033c") ;
        printf(GREEN"\n\n\n\n\n\n\t\t\t\t1:添加好友\n\n");
        printf("\t\t\t\t2:删除好友\n\n");
        printf("\t\t\t\t3:好友选项\n\n");
        printf("\t\t\t\t4:退出\n\n"END);
        printf("\t\t\t\t请输入你的选择 ：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:add_friend(client_msg ); break;
            case 2:delete_friend(client_msg); break;
            case 3:friend_list(client_msg );  break ;
            case 4:                                 break ;
            default :printf(RED"\n\n\t\t\t\t输入有错～～～,请检查后重新输入！！！\n"END);  sleep(1) ;  break ;
        }
    }while(choice !=  4 );
    return 0 ;
}
/*************************************************************我的群***************************************************/



int jiesan(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请 输 入你 想 要 解 散 的 群 号：");
    scanf("%d",&client_msg.to);

    client_msg.flag = 13 ;
    client_msg.state = -1;

    send(I_conn_fd,&client_msg,sizeof(TT),0) ;
    printf(YELLOW"\n\n\t\t\t\t申 请 已 发送 ， 请 耐 心 等  待 。 。 。。\n"END);
    sleep(2);
}



int tiren(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请 输 入  群 号：");
    scanf("%d",&client_msg.to);
    printf("\n\n\t\t\t\t请 输 入 你 所要 踢 的 人 的 QQ 号:");
    scanf("%d",&client_msg.num);
    if(client_msg.num == I_QQ)
    {
        printf(RED"\n\n\t\t\t\t 无 法 踢 自 己 \n"END);
        sleep(1);
        return 0;
    }
    client_msg.flag= 14 ;
    client_msg.state= -1;
    send(I_conn_fd ,&client_msg,sizeof(TT),0) ;
    printf(YELLOW"\n\n\t\t\t\t申 请 已 发送 ， 请 耐 心 等  待 。 。 。。\n"END);
    sleep(2);
}



int my_create_group(TT client_msg)
{
    int choice ;
    do
    {
        //printf("\033c");
        printf(GREEN"\n\n\n\n\n\n\t\t\t\t1.查看所有群\n\n");  //未解决
        printf("\t\t\t\t2.邀请新成员进群\n\n");
        printf("\t\t\t\t3.解 散 群\n\n");
        printf("\t\t\t\t4.踢 人 \n\n");   //需要标识及检查身份
        printf("\t\t\t\t5.退 出 \n\n"END);
        printf("\t\t\t\t请 输 入 你的选择：") ;
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:/*look_create_group() ;*/  break;
            case 2: invite_one(client_msg) ;  break;
            case 3: jiesan(client_msg) ;      break;
            case 4: tiren(client_msg);        break;
            case 5: break;
            default :printf(RED"\n\n\t\t\t\t输入有错～～～,请检查后重新输入！！！\n"END);  sleep(1) ;  break ;
        }
    }while(choice != 5);
    return 0;
}

int real_qun_chat(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你想要聊天的群的群号：");
    scanf("%d",&client_msg.to);

    client_msg.flag  =  9 ;
    client_msg.state =  -1 ;

    printf(YELLOW"\n\n\t请输入你要发的消息(按回车键发送,输入exit 退出群聊)：\n"END);
    qun_num  =   client_msg.to ;
    while(1)
    {
        printf(GREEN"I : "END);
        scanf("%s",&client_msg.str) ;
        if(strcmp(client_msg.str,"exit") ==  0 )
        {
            qun_num  = 0 ;
            return  0;
        }
        send(I_conn_fd,&client_msg,sizeof(TT),0);
    }
}



int  invite_one(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请 输 入 所 邀 群 号：");
    scanf("%d",&client_msg.to);
    printf("\n\t\t\t\t请 输 入 对 方 的 QQ 号：");  //在我的好友里面找对方
    scanf("%d",&client_msg.num);
    if(client_msg.num == I_QQ)
    {
        printf(RED"\n\n\t\t\t\t无 法 邀 请 自 己 \n"END);
        sleep(1);
        return 0;
    }
    if(search_friend(client_msg.num) == 0 ) 
    {
        printf(RED"\n\n\t\t\t\t她 还 不 是你 的 好 友 ， 无 法 邀 请 \n"END) ;
        sleep(1);
        return 0;
    }

    client_msg.flag = 10 ;
    client_msg.state = -1 ;

    send(I_conn_fd ,&client_msg,sizeof(TT),0);      //对方可同意也可拒绝～～ 

    printf(YELLOW"\n\n\t\t\t\t好友邀请已发送，请耐心等待。。。。\n"END);
    sleep(2);
}



int lookup_qun_chat_record(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你要查看聊天记录的群号：");
    scanf("%d",&client_msg.to);       //验证我是不是再这个群里面????????????????????????????

    client_msg.flag =  16 ;
    client_msg.state = -1 ;

    send(I_conn_fd,&client_msg,sizeof(TT),0) ;
    sleep(10);
}





int lookup_group_friend(TT client_msg)
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你要查看群成员的群号：");
    scanf("%d",&client_msg.to);

    client_msg.flag =  15 ;
    client_msg.state = -1 ;

    send(I_conn_fd,&client_msg,sizeof(TT),0 ) ;
    printf(YELLOW"\n\n\t\t\t\t请求已发送，请耐心等待。。。。\n"END);
    sleep(10);
}

int exit_qun(TT client_msg) //flag = 17 
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请 输 入 你 想 要 退 出 的 群 的 群 号：");
    scanf("%d",&client_msg.to) ;

    client_msg.flag  =  17 ;
    send(I_conn_fd,&client_msg,sizeof(TT),0) ;

    sleep(1);
    printf(YELLOW"\n\n\t\t\t\t你 已 成 功 退 出 群%d\n"END,client_msg.to);
    sleep(2);
}



int my_add_group(TT client_msg)
{
    int choice ;
    do
    {
        //printf("\033c");
        printf(GREEN"\n\n\n\n\n\n\t\t\t\t1:查看所有群\n\n");
        printf("\t\t\t\t2:邀请新成员进群\n\n");
        printf("\t\t\t\t3: 群   聊 \n\n");
        printf("\t\t\t\t4:查看群聊天记录\n\n");
        printf("\t\t\t\t5.查 看 群 成 员\n\n") ;
        printf("\t\t\t\t6.  退   群\n\n") ;
        printf("\t\t\t\t7: 返  回 \n\n"END);
        printf("\t\t\t\t请输入你的选择：") ;
        scanf("%d",&choice);
        switch(choice)
        {
            case 1:/*lookup_add__group(conn_fd);*/      break;
            case 2:invite_one(client_msg) ;             break;
            case 3:real_qun_chat(client_msg) ;          break;
            case 4:lookup_qun_chat_record(client_msg) ; break;
            case 5:lookup_group_friend(client_msg);     break ;
            case 6:exit_qun(client_msg) ;               break;
            case 7:break;
            default :printf(RED"\n\n\t\t\t\t输入有错～～～,请检查后重新输入！！！\n"END);  sleep(1) ;  break ;
        }
    }while(choice !=  7 );
    return 0;
}

int create_group(TT client_msg)  //创建群
{
    //printf("into create_group and client_msg.QQ == %d\n",client_msg.QQ);
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你想要创建的群的群号：");
    scanf("%d",&client_msg.to) ; // to   代表群号
    client_msg.flag =  8 ;
    send(I_conn_fd,&client_msg,sizeof(TT),0) ;
    sleep(1);
    printf(BLUE"\n\n\t\t\t\t建 群 成 功， 快 去 邀 请 新 成 员 加 入 吧 ！ ！\n"END) ;
    sleep(2);
}


int add_group(TT client_msg)   //加群
{
    printf("\033c");
    printf("\n\n\n\n\n\n\t\t\t\t请输入你想要加入的群的群号：");
    scanf("%d",&client_msg.to);
    client_msg.flag = 12 ;
    client_msg.state = -1 ;
    send(I_conn_fd,&client_msg,sizeof(TT),0);
    printf(BLUE"\n\n\t\t\t\t加 群 申 请 已 发送 ， 请 耐 心 等 待 群 主 同 意 \n"END);
    sleep(2);
}


int my_group(TT client_msg )
{
    int choice ;
    do
    {
        //printf("\033c");
        printf(GREEN"\n\n\n\n\n\n\t\t\t\t1:创建群 \n\n");
        printf("\t\t\t\t2:我创建的群 \n\n");
        printf("\t\t\t\t3:我加入的群 \n\n");
        printf("\t\t\t\t4.加群\n\n");
        printf("\t\t\t\t5:退出\n\n"END);
        printf("\t\t\t\t请输入你的选择：") ;
        scanf("%d",&choice);
        switch(choice)
        {
            case 1: create_group(client_msg);     break; //create_group 函数
            case 2: my_create_group(client_msg);  break;
            case 3: my_add_group(client_msg) ;    break;
            case 4: add_group(client_msg) ;       break;  //加群时得到群主的同意,与添加好友类似
            case 5: break;
            default :printf(RED"\n\n\t\t\t\t输入有错～～～,请检查后重新输入！！！\n"END);  sleep(1) ;  break ;
        }
    }while(choice !=  5);
    return  0;
}
/*****************************************消息盒子*******************************************/


int add_user_list(int number)
{
    //printf("into add_user_list number ==%d \n",number);
    U_L *temp ;
    temp=(U_L *)malloc(sizeof(U_L));
    temp->QQ  =  number ;        //好友的链表只存储QQ 即可
    temp->next = head->next ;
    head->next = temp ;
}

int print()
{
    U_L *temp=head->next ;
    while(temp)
    {
        printf("QQ == %d\n",temp->QQ);
        temp=temp->next ;
    }
}

int solve_add_friend(TT client_msg)
{
    char ch ;
    getchar() ;
   // printf("client_msg.QQ == %d\n",client_msg.QQ);
    //printf("client_msg.to == %d\n",client_msg.to);
    printf(GREEN"\n\t\t\t\t 输 入'Y' 代 表 同 意 该 申请 ， 输 入'N' 代 表 不同 意 该 申 请 \n"END);
    scanf("%c",&ch);
    if(ch == 'Y' || ch == 'y')
    {
        client_msg.state =  1 ;
        add_user_list(client_msg.to) ;  //加入好友的链表
        print() ;
    }
    if(ch == 'N' || ch == 'n')
        client_msg.state =  2 ;

    client_msg.flag = 4 ;

    send(I_conn_fd,&client_msg,sizeof(TT),0 );   //同意或者不同意该申请
}


int solve_invite(TT client_msg)
{
    char ch ;
    getchar() ;
   /* printf("client_msg.QQ == %d\n",client_msg.QQ);
    printf("client_msg.to == %d\n",client_msg.to);
    printf("client_msg.num == %d\n",client_msg.num);*/
    printf(GREEN"\n\t\t\t\t 输 入'Y' 代 表 同 意 该 申请 ， 输 入'N' 代 表 不同 意 该 申 请 \n\t\t\t\t"END);
    scanf("%c",&ch);
    if(ch == 'Y' || ch == 'y')
        client_msg.state =  1 ;
    if(ch == 'N' || ch == 'n')
        client_msg.state =  2 ;
    client_msg.flag =  10 ;

    send(I_conn_fd,&client_msg,sizeof(TT),0 );   //同意或者不同意该申请
}

int solve_add_group(TT client_msg)
{
    char ch ;
    getchar() ;
    /*printf("client_msg.QQ == %d\n",client_msg.QQ);
    printf("client_msg.to == %d\n",client_msg.to);
    printf("client_msg.num == %d\n",client_msg.num);*/
    printf(GREEN"\n\t\t\t\t 输 入'Y' 代 表 同 意 该 申请 ， 输 入'N' 代 表 不同 意 该 申 请 \n\t\t\t\t"END);
    scanf("%c",&ch);
    if(ch == 'Y' || ch == 'y')
        client_msg.state =  1 ;
    if(ch == 'N' || ch == 'n')
        client_msg.state =  2 ;
    client_msg.flag =  12 ;
    send(I_conn_fd,&client_msg,sizeof(TT),0 );   //同意或者不同意该申请
}


int my_box(TT client_msg)
{
    char ch ;
    TT *zanshi ;
    TT *pre  =  mhead ;
    TT *temp = mhead->next ;
    printf("\033c");
    if(temp == NULL)     printf(RED"\n\n\n\n\n\n\t\t\t\t  消   息   盒   子   为    空   ～   ～   \n\t\t\t\t"END);
    while(temp)
    {
        switch(temp->flag)
        {
            case 11:
                printf(PURPLE"\n\t\t\t\t 好 友 %d  给 你 发 了 一 条 消  息： %s\n"END ,temp->QQ,temp->str) ; 
                zanshi= temp ;
                pre->next = temp->next ;
                temp = temp->next ;
                free(zanshi);
                break;
            case 4 :
                printf(BLUE"\n\t\t\t\t 好 友 %d  请 求 添 加 你 为 好 友  \n"END,temp->QQ);
                client_msg.to = temp->QQ ;
                //getchar() ;
                setbuf(stdin,NULL);
                printf(YELLOW"\n\t\t\t\t回 车 默 认 不 处 理 ， 按 其 他 键 进 行 处 理 \n\t\t\t\t"END) ;
                scanf("%c",&ch);
                if(ch !=  '\n')
                {
                    solve_add_friend(client_msg) ;
                    zanshi= temp ;
                    pre->next = temp->next ;
                    temp = temp->next ;
                    free(zanshi);
                }
                else{
                    pre=pre->next ;
                    temp= temp->next ;
                }
                break;
            case 10:
                printf("\n\t\t\t\t 好 友 %d 邀 请 你 进 入 群 %d \n",temp->QQ,temp->to);
                client_msg.num = temp->QQ ; //11
                client_msg.to = temp->to ; //8888
                /*printf("client_msg.QQ == %d \n",client_msg.QQ);
                printf("client_msg->to == %d \n",client_msg.to);
                printf("client_msg.num == %d \n",client_msg.num);*/
                //getchar() ;
                setbuf(stdin,NULL);
                printf(YELLOW"\n\t\t\t\t回 车 默 认 不 处 理 ， 按 其 他 键 进 行 处 理 \n\t\t\t\t"END) ;
                scanf("%c",&ch);
                if(ch !=  '\n')
                {
                    solve_invite(client_msg) ;
                    zanshi= temp ;
                    pre->next = temp->next ;
                    temp = temp->next ;
                    free(zanshi);
                }
                else{
                    pre=pre->next ;
                    temp= temp->next ;
                }
                break;
            case 9:
                printf(PURPLE"\n\t\t\t\t 群 %d  收  到  %d  发 的 一 条 消 息 ：%s\n"END ,temp->to,temp->QQ,temp->str) ; 
                zanshi= temp ;
                pre->next = temp->next ;
                temp = temp->next ;
                free(zanshi);
                break;
            case 18:
                printf(PURPLE"\n\t\t\t\t 好 友 %d  发 来 一 个 文 件：%s\n"END ,temp->QQ ,temp->passwd ) ; 
                zanshi= temp ;
                pre->next = temp->next ;
                temp = temp->next ;
                free(zanshi);
                break;
            case 12:
                printf(RED"\n\t\t\t\t  %d  申  请 加 入 群   %d \n"END,temp->QQ ,temp->to);
                client_msg.QQ = I_QQ ;  // 11
                client_msg.num = temp->QQ ; // 22
                client_msg.to = temp->to ; //8888
               /* printf("client_msg.QQ == %d \n",client_msg.QQ);
                printf("client_msg->to == %d \n",client_msg.to);
                printf("client_msg.num == %d \n",client_msg.num);*/
                //getchar() ;
                setbuf(stdin,NULL);
                printf(YELLOW"\n\t\t\t\t回 车 默 认 不 处 理 ， 按 其 他 键 进 行 处 理 \n\t\t\t\t"END) ;
                scanf("%c",&ch);
                if(ch !=  '\n')
                {
                    solve_add_group(client_msg) ;
                    zanshi= temp ;
                    pre->next = temp->next ;
                    temp = temp->next ;
                    free(zanshi);
                }
                else{
                    pre=pre->next ;
                    temp= temp->next ;
                }
                break;
            default:
                    pre=pre->next ;
                    temp= temp->next ;
                    break;
        }
    }
}

int main(void)
{
    int   i ;
    TT temp ;
    int conn_fd  , choice ;
    struct sockaddr_in  serve_addr ;

    memset(&serve_addr,0,sizeof(struct sockaddr_in)) ;
    memset(&temp,0,sizeof(TT)) ;

    serve_addr.sin_family = AF_INET ;  
    serve_addr.sin_port = htons(SERVER_PORT) ; 
    serve_addr.sin_addr.s_addr= inet_addr(SERVER_IP) ;    //设置IP
    conn_fd = socket(AF_INET,SOCK_STREAM ,0 );

    if(conn_fd < 0)  myerror("client  socket ",__LINE__);
    if(connect(conn_fd,(struct sockaddr *)&serve_addr,sizeof(struct sockaddr))   < 0)
        myerror("client connect ",__LINE__);  //连接套接字成功

    head = init_user_list() ;//初始化用户列表
    mhead= init_mhead();   //初始化消息盒子
    I_conn_fd =  conn_fd ;


   do
    {
        printf("\033c");
        printf(GREEN"\n\n\n\n\n\n\n\n\t\t\t\t\t\t欢 迎 来 到 为 你 精 心 打 造 的 聊 天 室 \n\n"END);
        printf(YELLOW"\t\t\t\t\t\t\t    0.  注   册\n"END);
        printf(BLUE  "\t\t\t\t\t\t\t    1.  登   录\n"END);        //登录完成再进行各种处理
        printf(RED   "\t\t\t\t\t\t\t    2.  取   消\n"END);
        printf(      "\t\t\t\t\t\t\t    请输入你的选择：");
        scanf("%d",&choice);
        switch(choice)
        {
            case 0:  client_join(conn_fd) ;  break ;
            case 1:  client_sign(conn_fd) ;  break ;
            case 2:  temp.state = -1 ;       break ;
            default :printf(RED"\n\n\t\t\t\t\t\t\t输入有错～～～,请检查后重新输入!!!\n"END); sleep(1) ;  break ;
        }
    }while(choice !=  2 );     //取消 
    logout(temp,I_conn_fd) ;
    return  0 ;
}
