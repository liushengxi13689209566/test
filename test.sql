





drop database if exists chatroom ;
create  database chatroom ;     /*创建数据库*/
use chatroom ;

drop table if exists user_data ;    /*用户表*/
create table user_data ( QQ INT UNSIGNED PRIMARY KEY ,username CHAR(32) , passwd VARCHAR(32)  ) ;

drop table if exists chat_record ;   /*个人聊天记录*/
create table chat_record ( come INT UNSIGNED  ,go   INT UNSIGNED ,record VARCHAR(40) , time  TIMESTAMP DEFAULT CURRENT_TIMESTAMP) ;

drop table if exists qun_chat_record ; /*群聊天记录*/
create table qun_chat_record( qun_num INT UNSIGNED  ,come_QQ  INT UNSIGNED , username VARCHAR(32),record VARCHAR(40) , time  TIMESTAMP DEFAULT CURRENT_TIMESTAMP) ;


drop table if exists off_line_record ; /*离线消息，只存储个人聊天的消息和群聊消息*/
create table off_line_record( go_QQ INT UNSIGNED  , come_QQ  INT UNSIGNED , record VARCHAR(40), mode     INT  ) ;


drop table if exists system_log ;    /*系统日志*/
create table system_log( user_QQ  INT UNSIGNED  , username VARCHAR(32), record VARCHAR(40) , time  TIMESTAMP DEFAULT CURRENT_TIMESTAMP) ;



