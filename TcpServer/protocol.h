#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdlib.h>
#include <unistd.h> // Unix库函数，包含了read等系统服务函数
#include <string.h>

typedef unsigned int uint;
//宏定义
#define REGIST_OK "regist ok"
#define REGIST_FAILED "regist failed : name existed"
#define LOGIN_OK "login ok"
#define LOGIN_FAILED "login failed : name no existed or pwd error or logined"
#define SERACH_USER_NO "no such people"
#define SEARCH_USER_ONLINE "online"
#define SEARCH_USER_OFFLINE "offline"
#define UNKNOW_ERROR "unknow error"
#define EXISTED_FRIEND "friend existed"
#define CANT_ADD "user existed but offline, can't add"
#define CAN_ADD "user existed and online, can add"
#define DEL_FRIEND_OK "delete friend success"
#define DEL_FRIEND_FAILED "delete friend failed"
#define DIR_NO_EXIST "current dir not exist"
#define FILE_NAME_EXIST "file name exist"
#define DIR_CREATE_OK "successfully create dir"
#define DIR_DELETE_OK "successfully delete dir"
#define DIR_DELETE_FAILED "failured delete dir: is file"
#define DIR_RENAME_FAILED "failured rename dir or file"
#define DIR_RENAME_OK "successfully rename dir or file"
#define ENTER_DIR_FAILURED "failured enter dir"
#define ENTER_DIR_OK "successfully enter dir"
#define UPLOAD_FILE_OK "successfully upload file"
#define UPLOAD_FILE_FAILURED "failured upload file"
#define FILE_DELETE_OK "successfully delete file"
#define FILE_DELETE_FAILED "failured delete file: is dir"
#define MOVE_FILE_OK "successfully MOVE file"
#define MOVE_FILE_FAILED "failured MOVE file: please select dst dir"
//枚举消息类型 自动增长 0 1 2...
enum ENUM_MSG_TYPE{
    ENUM_MSG_TYPE_MIN=0,
    ENUM_MSG_TYPE_REGIST_REQUEST,
    ENUM_MSG_TYPE_REGIST_RESPOND,
    ENUM_MSG_TYPE_LOGIN_REQUEST,
    ENUM_MSG_TYPE_LOGIN_RESPOND,
    ENUM_MSG_TYPE_ALL_ONLINE_REQUEST,//在线用户请求
    ENUM_MSG_TYPE_ALL_ONLINE_RESPOND,//在线用户回应
    ENUM_MSG_TYPE_SEARCH_USER_REQUEST,//SEARCH_USER_REQUEST
    ENUM_MSG_TYPE_SEARCH_USER_RESPOND,//SEARCH_USER_RESPOND
    ENUM_MSG_TYPE_ADD_FRIEND_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_ADD_FRIEND_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_ADD_FRIEND_AGGREE,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_ADD_FRIEND_REFUSE,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_PRIVATE_CHAT_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_GROUP_CHAT_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_GROUP_CHAT_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_CRATE_DIR_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_CRATE_DIR_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_FLUSH_FILE_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_FLUSH_FILE_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_DELETE_DIR_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_DELETE_DIR_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_NEW_NAME_RESQUEST,
    ENUM_MSG_TYPE_NEW_NAME_RESPOND,
    ENUM_MSG_TYPE_ENTER_DIR_RESQUEST,
    ENUM_MSG_TYPE_ENTER_DIR_RESPOND,
    ENUM_MSG_TYPE_RETURN_DIR_RESQUEST,
    ENUM_MSG_TYPE_RETURN_DIR_RESPOND ,
    ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST,
    ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND,
    ENUM_MSG_TYPE_DELETE_FILE_REQUEST,//ADD_FRIEND_REQUEST
    ENUM_MSG_TYPE_DELETE_FILE_RESPOND,//ADD_FRIEND_RESPOND
    ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST,
    ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND,
    ENUM_MSG_TYPE_SHARE_FILE_REQUEST,
    ENUM_MSG_TYPE_SHARE_FILE_RESPOND,
    ENUM_MSG_TYPE_SHARE_FILE_NOTE,
    ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND,
    ENUM_MSG_TYPE_MOVE_FILE_REQUEST,
    ENUM_MSG_TYPE_MOVE_FILE_RESPOND,
    ENUM_MSG_TYPE_MAX=0x00ffffff
};
struct FileInfo{
    char caName[32];//文件名
    int iFileType;//文件类型

};
// 设计协议数据单元格式 也就是客户端用于通信的信息封装格式
struct PDU
{
    uint uiPDULen;       // 总的协议数据单元大小
    uint uiMsgType;      // 消息类型
    char caFileData[64]; // 文件名称 或者用户名，密码
    uint uiMsgLen;       // 实际消息长度
    int iMsg[];          // 实际消息，主要通过iMsg访问消息数据
};


PDU* mkPDU(uint uiMsgLen); // 创建PDU，uiMsglen是可变的，总大小可有其计算得到

#endif // PROTOCOL_H
