#include "mytcpsocket.h"
#include "mytcpserver.h"
#include <QDebug>
#include <QSet>
#include <QFileInfoList>
#include <QFile>
MyTcpSocket::MyTcpSocket()
{
    //与客户端通信 接受到客户端的信号触发相应的函数
    connect(this,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    //监听客户端的断开连接信号
    connect(this,SIGNAL(disconnected()),this,SLOT(clientOffline()));
    m_bUploadT = false;
    m_Timer = new QTimer();
    connect(m_Timer,SIGNAL(timeout()),this,SLOT(sendFileData()));
}

QString MyTcpSocket::getName()
{
    return m_strName;
}

bool MyTcpSocket::copyDir(QString strSrcDir, QString strDstDir)
{
    QDir dir;
    //创建要保存的目录文件
    dir.mkdir(strDstDir);
    //设置要分享的文件目录路径
    dir.setPath(strSrcDir);
    //获取要分享的文件列表
    QFileInfoList fileInfoList = dir.entryInfoList();
    //原文件和目标文件路径
    QString srcTmp;
    QString destTmp;

    for(int i = 0; i< fileInfoList.size(); ++i){
        //如果要分享的是文件则直接复制
        qDebug()<<fileInfoList[i].fileName();
        if(fileInfoList[i].isFile()){

            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDstDir+'/'+fileInfoList[i].fileName();
            bool f = QFile::copy(srcTmp,destTmp);
            if(f==false){
                qDebug()<<"分享文件失败:"<<srcTmp;
                return false;
            }
        }else if(fileInfoList[i].isDir()){
            if(QString(".")==fileInfoList[i].fileName()||
                    fileInfoList[i].fileName()==QString(".."))
                continue;
            //要分享的使目录则递归调用函数来复制
            srcTmp = strSrcDir+'/'+fileInfoList[i].fileName();
            destTmp = strDstDir+'/'+fileInfoList[i].fileName();
            copyDir(srcTmp,destTmp);
        }
    }
    return true;
}

//接受客户端的信息 可以有很多不同种类的信息
void MyTcpSocket::recvMsg()
{
    if(!m_bUploadT){
        qDebug()<<this->bytesAvailable();// 获取接收到的数据大小
        uint uiPDULen = 0;
        //先读取uiPDULen
        this->read((char*)&uiPDULen,sizeof (uint));// 先读取uint大小的数据，首个uint正是总数据大小
        uint uiMsgLen = uiPDULen-sizeof(PDU);// 实际消息大小，sizeof(PDU)只会计算结构体大小，而不是分配的大小
        //以及赋值了 uiPDULen uiMsgLen
        PDU *pdu = mkPDU(uiMsgLen);
        //后面的数据就是 // 消息类型// 文件名称    // 实际消息长度    // 实际消息，主要通过iMsg访问消息数据
        this->read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));// 接收剩余部分数据（第一个uint已读取）并且从pdu对应的内存地址开始存入数据
        /*
在 switch 语句中，如果某个 case 分支内没有使用 break 语句，
那么程序将会继续执行下一个 case 分支的代码，
直到遇到 break 语句或者 switch 语句的末尾。
这种行为被称为“case穿透”（case fall-through）。
*/
        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            //从pdu->caFileData指向的地址取32个字节复制到从caName指向的地址开始的32字节
            strncpy(caName, pdu->caFileData, 32);
            strncpy(caPwd, pdu->caFileData + 32, 32);
            //opeDB p;私有构造器
            bool ret = opeDB::getinstance().handleRegist(caName,caPwd);
            PDU *respdu = mkPDU(0);//回复信息
            respdu->uiMsgType = ENUM_MSG_TYPE_REGIST_RESPOND;

            if(ret){
                strcpy(respdu->caFileData,REGIST_OK);
                // 每次注册成功在服务器里创建对应用户的文件夹
                QDir dir;
                dir.mkdir(QString("./%1").arg(caName));

            } else {
                strcpy(respdu->caFileData,REGIST_FAILED);
            }
            write((char*)respdu,pdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        };
        case ENUM_MSG_TYPE_LOGIN_REQUEST:
        {
            char caName[32] = {'\0'};
            char caPwd[32] = {'\0'};
            //从pdu->caFileData指向的地址取32个字节复制到从caName指向的地址开始的32字节
            strncpy(caName, pdu->caFileData, 32);
            strncpy(caPwd, pdu->caFileData + 32, 32);
            //opeDB p;私有构造器
            bool ret = opeDB::getinstance().handleLogin(caName,caPwd);
            PDU* respdu = mkPDU(0);//回复信息
            respdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_RESPOND;

            if(ret){
                strcpy(respdu->caFileData,LOGIN_OK);
                m_strName = QString(caName);
            } else {
                strcpy(respdu->caFileData,LOGIN_FAILED);
            }
            write((char*)respdu,pdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        };
        case ENUM_MSG_TYPE_ALL_ONLINE_REQUEST:
        {
            QStringList userAllOnline = opeDB::getinstance().handleAllOnline();
            uint uiMsgLen = userAllOnline.size()*32;
            PDU* respdu = mkPDU(uiMsgLen);
            respdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_RESPOND;
            for(int i = 0; i < userAllOnline.size(); ++i){
                //转化为c风格的字符串类型 也就是字符数组 数组名就是首地址
                memcpy((char*)(respdu->iMsg)+i*32,userAllOnline[i].toStdString().c_str(),userAllOnline[i].size());


            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        };
        case ENUM_MSG_TYPE_SEARCH_USER_REQUEST:
        {
            int ret = opeDB::getinstance().handleSearchUser(pdu->caFileData);
            PDU *respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_RESPOND;
            if(-1==ret){
                strcpy(respdu->caFileData,SERACH_USER_NO);
            } else if(1==ret){
                strcpy(respdu->caFileData,SEARCH_USER_ONLINE);
            } else if(0==ret){
                strcpy(respdu->caFileData,SEARCH_USER_OFFLINE);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        };
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {
            //设置空字符串
            char userName[32] = {'\0'};
            char friendName[32] = {'\0'};
            //从pdu->caFileData指向的地址取32个字节复制到从caName指向的地址开始的32字节
            strncpy(userName, pdu->caFileData, 32);
            strncpy(friendName, pdu->caFileData + 32, 32);
            int ret = opeDB::getinstance().handleAddFriend(userName,friendName);
            PDU *respdu = NULL;

            if(-1==ret){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caFileData,UNKNOW_ERROR);
                write((char*)respdu,respdu->uiPDULen);
            }else if(0==ret){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caFileData,CANT_ADD);
                write((char*)respdu,respdu->uiPDULen);
            }else if(1==ret){
                if(!(0==strcmp(userName,friendName)))
                {   respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                    strcpy(respdu->caFileData,CAN_ADD);
                    write((char*)respdu,respdu->uiPDULen);
                    MyTcpServer::getInstance().resend(friendName,pdu);
                } else {
                    write((char*)pdu,pdu->uiPDULen);
                }
            }else if(2==ret){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_RESPOND;
                strcpy(respdu->caFileData,EXISTED_FRIEND);
                write((char*)respdu,respdu->uiPDULen);
            }
            free(respdu);
            respdu = NULL;
            break;
        };
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            //设置空字符串
            char userName[32] = {'\0'};
            char friendName[32] = {'\0'};
            //从pdu->caFileData指向的地址取32个字节复制到从caName指向的地址开始的32字节
            strncpy(userName, pdu->caFileData, 32);
            strncpy(friendName, pdu->caFileData + 32, 32);
            opeDB::getinstance().handleAddFriendAgree(userName,friendName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;
            if(0==strcmp(userName,friendName)){
                strcpy(respdu->caFileData,QString("成功添加自己为好友").toStdString().c_str());
                write((char*)respdu,respdu->uiPDULen);
            }else {
                strcpy(respdu->caFileData,QString("成功添加好友%1").arg(friendName).toStdString().c_str());
                MyTcpServer::getInstance().resend(userName,respdu);
                strcpy(respdu->caFileData,QString("成功添加好友%1").arg(userName).toStdString().c_str());
                write((char*)respdu,respdu->uiPDULen);
            }
            free(respdu);
            respdu = NULL;
            break;
        };
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {
            //设置空字符串
            char userName[32] = {'\0'};
            char friendName[32] = {'\0'};
            //从pdu->caFileData指向的地址取32个字节复制到从caName指向的地址开始的32字节
            strncpy(userName, pdu->caFileData, 32);
            strncpy(friendName, pdu->caFileData + 32, 32);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
            strcpy(respdu->caFileData,QString("%1 refuse your friend requestion").arg(friendName).toStdString().c_str());
            MyTcpServer::getInstance().resend(userName,respdu);
            free(respdu);
            respdu = NULL;
            break;
        };
        case ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST:
        {
            char userName[32] = {'\0'};
            strncpy(userName, pdu->caFileData, 32);
            QSet<QString> friendList = opeDB::getinstance().handleFlushFriend(userName);
            PDU* respdu = mkPDU(friendList.size()*32);
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND;
            int i = 0;
            for(auto friendName:friendList){
                memcpy(((char*)respdu->iMsg)+32*i,
                       friendName.toStdString().c_str(),
                       friendName.size());
                i++;
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char userName[32] = {'\0'};
            char friendName[32] = {'\0'};
            //从pdu->caFileData指向的地址取32个字节复制到从caName指向的地址开始的32字节
            strncpy(userName, pdu->caFileData, 32);
            strncpy(friendName, pdu->caFileData + 32, 32);
            bool deleteFlag = opeDB::getinstance().handleDeleteFriend(userName,friendName);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND;
            if(deleteFlag){

                strcpy(respdu->caFileData,DEL_FRIEND_OK);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
                if(!(0==strcmp(userName,friendName)))
                    MyTcpServer::getInstance().resend(friendName,pdu);
            }
            else{
                strcpy(respdu->caFileData,DEL_FRIEND_FAILED);
                write((char*)respdu,respdu->uiPDULen);
                free(respdu);
                respdu=NULL;
                if(!(0==strcmp(userName,friendName)))
                    MyTcpServer::getInstance().resend(friendName,pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char userName[32] = {'\0'};
            char friendName[32] = {'\0'};
            //从pdu->caFileData指向的地址取32个字节复制到从caName指向的地址开始的32字节
            strncpy(userName, pdu->caFileData, 32);
            strncpy(friendName, pdu->caFileData + 32, 32);
            qDebug()<<userName;
            qDebug()<<friendName;
            //转发的一定是在线的socket 不在线是私聊不了的
            MyTcpServer::getInstance().resend(friendName,pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            char userName[32] = {'\0'};
            strncpy(userName, pdu->caFileData, 32);
            QSet<QString> friendList = opeDB::getinstance().handleFlushFriend(userName);
            for(auto friendName:friendList){
                //将群聊信息转发给所有在线好友
                if(0 != strcmp(userName,friendName.toStdString().c_str()))
                    MyTcpServer::getInstance().resend(friendName.toStdString().c_str(),pdu);
            }
            break;
        }
        case ENUM_MSG_TYPE_CRATE_DIR_REQUEST:
        {
            //管理文件操作系统的目录文件使用B+ 树
            QDir dir;
            QString strCurpath = QString("%1").arg((char*)(pdu->iMsg));
            //从当前服务器查找当前路径
            bool ret = dir.exists(strCurpath);//判断当前路径是否存在
            PDU *respdu = NULL;

            if(ret){//如果当前路径存在
                char newDirName[32] = {'\0'};
                strncpy(newDirName, pdu->caFileData + 32, 32);
                //拼接文件名得到新文件夹的路径
                QString strNewPath = strCurpath+ "/"+newDirName;
                //   qDebug()<<strNewPath;
                //从当前服务器查找当前文件夹的路径是否存在
                ret = dir.exists(strNewPath);
                // qDebug()<<"->"<<ret;
                if(ret){//创建的文件夹文件名重名
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CRATE_DIR_RESPOND;
                    strcpy(respdu->caFileData,FILE_NAME_EXIST);
                }
                else//不重名，则可以创建
                {
                    respdu = mkPDU(0);
                    respdu->uiMsgType = ENUM_MSG_TYPE_CRATE_DIR_RESPOND;
                    strcpy(respdu->caFileData,DIR_CREATE_OK);
                    //不重名就创建新文件夹
                    dir.mkdir(strNewPath);
                }

            }
            else {
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_CRATE_DIR_RESPOND;
                strcpy(respdu->caFileData,DIR_NO_EXIST);

            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu=NULL;
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_REQUEST:
        {
            /*
delete：
delete 是 C++ 中的操作符，用于释放使用 new 运算符动态分配的内存。
delete 会调用对象的析构函数（如果有的话），然后释放内存。
通常用于释放使用 new 分配的单个对象或数组。
在 C++ 中，对于使用 new 分配的内存，应该使用 delete 来释放，以确保正确地调用析构函数并释放内存。例如：delete pCurPath; 或 delete[] pCurPath;。
free()：
free() 是 C 语言中的函数，用于释放动态分配的内存。
free() 只释放内存，不会调用对象的析构函数。
通常用于释放使用 malloc、calloc 或 realloc 分配的内存。
在 C++ 中，通常应该避免混合使用 new/delete 和 malloc/free，因为它们具有不同的内存管理模型。
*/
            //使用 new 运算符在堆内存中动态分配了一个大小为 pdu->uiMsgLen 的字符数组，并将分配的内存地址赋给 pCurPath 指针。
            char *pCurPath = new char[pdu->uiMsgLen];
            //获取当前路劲
            memcpy(pCurPath,(char*)pdu->iMsg,pdu->uiMsgLen);
            QDir dir(pCurPath);

            //得到当前路劲的目录下的所有文件和文件夹的信息
            QFileInfoList fileInfoList = dir.entryInfoList();
            int iFileCount = fileInfoList.size();
            //将要查找的当前目录下的文件信息返回给客户端
            PDU *respdu = mkPDU(sizeof(FileInfo)*iFileCount);
            FileInfo *pFileInfo = NULL;
            QString strFileName;
            respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
            for(int i = 0; i < iFileCount;++i){
                //在C和C++中，指针的算术运算是基于指针指向的类型的大小的。
                //当对指针执行加法操作时，指针会按照其指向类型的字节大小来进行地址的递增或者叫偏移。
                //pFileInfo 指向的是 respdu 中的正确地址,也就是指针赋值
                pFileInfo = (FileInfo*)(respdu->iMsg)+i;
                memcpy((char*)pFileInfo->caName,fileInfoList[i].fileName().toStdString().c_str(),fileInfoList[i].fileName().size());
                if(fileInfoList[i].isDir()){
                    pFileInfo->iFileType = 0;
                }
                else if(fileInfoList[i].isFile()){
                    pFileInfo->iFileType = 1;
                }
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_REQUEST:
        {
            char fileName[32] = {'\0'};
            strcpy(fileName,pdu->caFileData);
            char *pPath = new char[pdu->uiMsgLen];
            strcpy(pPath,(char*)pdu->iMsg);
            QString strPath = QString("%1/%2").arg(pPath).arg(fileName);
            qDebug()<<strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if(fileInfo.isDir()){
                //QDir是Qt中用于操作目录（文件夹）和文件的类
                QDir dir;
                //setPath是QDir类的一个成员函数，用于设置QDir对象要操作的目录路径。
                //这里的strPath是一个字符串变量，包含了要操作的目录的路径。
                dir.setPath(strPath);
                //如果是文件夹则删除目录 removeRecursively是QDir类的一个成员函数，用于递归地删除目录及其包含的所有文件和子目录。
                ret = dir.removeRecursively();
            } else if(fileInfo.isFile()){
                ret = false;
            }
            PDU *respdu = NULL;
            if(ret){
                respdu = mkPDU(QString(DIR_DELETE_OK).size()+1);
                memcpy((char*)respdu->caFileData,DIR_DELETE_OK,QString(DIR_DELETE_OK).size());
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;


            } else {
                respdu = mkPDU(QString(DIR_DELETE_FAILED).size()+1);
                memcpy((char*)respdu->caFileData,DIR_DELETE_FAILED,QString(DIR_DELETE_FAILED).size());
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_RESPOND;
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_NEW_NAME_RESQUEST:
        {
            char OldName[32] = {'\0'};
            char NewName[32] = {'\0'};
            strncpy(OldName,pdu->caFileData,32);
            strncpy(NewName,pdu->caFileData+32,32);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->iMsg,pdu->uiMsgLen);
            QString strOldPath = QString("%1/%2").arg(pPath).arg(OldName);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(NewName);
            QDir dir;
            //通过路径来定位到要重命名的文件或目录，然后进行重命名
            bool ret = dir.rename(strOldPath,strNewPath);
            PDU* respdu = mkPDU(0);
            if(ret){
                respdu->uiMsgType = ENUM_MSG_TYPE_NEW_NAME_RESPOND;
                memcpy(respdu->caFileData,DIR_RENAME_OK,strlen(DIR_RENAME_OK));

            }else {
                respdu->uiMsgType = ENUM_MSG_TYPE_NEW_NAME_RESPOND;
                memcpy(respdu->caFileData,DIR_RENAME_FAILED,strlen(DIR_RENAME_FAILED));
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESQUEST:
        {
            char DirName[32] = {'\0'};
            strncpy(DirName,pdu->caFileData,32);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->iMsg,pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(DirName);
            PDU* respdu = NULL;
            QDir dir;
            QFileInfo qf(strNewPath);
            if(qf.isDir()){
                QDir dir(strNewPath);
                //得到当前路劲的目录下的所有文件和文件夹的信息
                QFileInfoList fileInfoList = dir.entryInfoList();
                int iFileCount = fileInfoList.size();
                //将要查找的当前目录下的文件信息返回给客户端
                respdu = mkPDU(sizeof(FileInfo)*iFileCount);
                FileInfo *pFileInfo = NULL;
                QString strFileName;
                respdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_RESPOND;
                for(int i = 0; i < iFileCount;++i){
                    //在C和C++中，指针的算术运算是基于指针指向的类型的大小的。
                    //当对指针执行加法操作时，指针会按照其指向类型的字节大小来进行地址的递增或者叫偏移。
                    //pFileInfo 指向的是 respdu 中的正确地址,也就是指针赋值
                    pFileInfo = (FileInfo*)(respdu->iMsg)+i;
                    memcpy((char*)pFileInfo->caName,fileInfoList[i].fileName().toStdString().c_str(),fileInfoList[i].fileName().size());
                    if(fileInfoList[i].isDir()){
                        pFileInfo->iFileType = 0;
                    }
                    else if(fileInfoList[i].isFile()){
                        pFileInfo->iFileType = 1;
                    }
                }
            } else if(qf.isFile()){
                respdu = mkPDU(0);
                respdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESPOND;
                strcpy(respdu->caFileData,ENTER_DIR_FAILURED);
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST:
        {
            char FileName[32] = {'\0'};
            qint64 fileSize = 0;
            sscanf(pdu->caFileData,"%s %lld",FileName,&fileSize);
            char* pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,pdu->iMsg,pdu->uiMsgLen);
            QString strNewPath = QString("%1/%2").arg(pPath).arg(FileName);

            delete []pPath;
            pPath = NULL;
            //设置文件路径
            m_file.setFileName(strNewPath);
            //以只写的方式打开文件，若文件不存在，则会自动创建文件
            if(m_file.open(QIODevice::WriteOnly)){
                //设置上传文件的参数
                m_bUploadT = true;
                m_iTotal = fileSize;
                m_iRecved = 0;
            }
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_REQUEST:
        {
            char fileName[32] = {'\0'};
            strcpy(fileName,pdu->caFileData);
            char *pPath = new char[pdu->uiMsgLen];
            strcpy(pPath,(char*)pdu->iMsg);
            QString strPath = QString("%1/%2").arg(pPath).arg(fileName);
            qDebug()<<strPath;
            QFileInfo fileInfo(strPath);
            bool ret = false;
            if(fileInfo.isDir()){
                ret = false;

            } else if(fileInfo.isFile()){
                //QDir是Qt中用于操作目录（文件夹）和文件的类
                QDir dir;
                //setPath是QDir类的一个成员函数，用于设置QDir对象要操作的目录路径。
                //这里的strPath是一个字符串变量，包含了要操作的目录的路径。
                ret = dir.remove(strPath);
                //如果是文件夹则删除目录 removeRecursively是QDir类的一个成员函数，用于递归地删除目录及其包含的所有文件和子目录。

            }
            PDU *respdu = NULL;
            if(ret){
                respdu = mkPDU(QString(FILE_DELETE_OK).size()+1);
                memcpy((char*)respdu->caFileData,FILE_DELETE_OK,QString(FILE_DELETE_OK).size());
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;


            } else {
                respdu = mkPDU(QString(FILE_DELETE_FAILED).size()+1);
                memcpy((char*)respdu->caFileData,FILE_DELETE_FAILED,QString(FILE_DELETE_FAILED).size());
                respdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_RESPOND;
            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST:
        {
            char downloadName[32] = {'\0'};
            strcpy(downloadName,pdu->caFileData);
            char *pPath = new char[pdu->uiMsgLen];
            strcpy(pPath,(char*)pdu->iMsg);
            QString strPath = QString("%1/%2").arg(pPath).arg(downloadName);
            qDebug()<<strPath;
            delete []pPath;
            pPath = NULL;
            QFileInfo fileInfo(strPath);
            qint64 fileSize = fileInfo.size();
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND;
            sprintf(respdu->caFileData,"%s %lld",downloadName,fileSize);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            m_file.setFileName(strPath);
            m_file.open(QIODevice::ReadOnly);
            m_Timer->start(1000);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_REQUEST:
        {
            char caSendName[32] = {'\0'};
            int num = 0;
            sscanf(pdu->caFileData,"%s %d",caSendName,&num);
            int size = num*32;
            PDU *respdu = mkPDU(pdu->uiMsgLen-size);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE;
            strcpy(respdu->caFileData,caSendName);
            memcpy((char*)(respdu->iMsg),(char*)(pdu->iMsg)+size,pdu->uiMsgLen-size);
            char caRecvName[32] = {'\0'};
            for(int i = 0; i < num; ++i){
                memcpy(caRecvName,(char*)(pdu->iMsg)+32*i,32);
                MyTcpServer::getInstance().resend(caRecvName,respdu);
            }
            free(respdu);
            respdu = NULL;
            respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_RESPOND;
            strcpy(respdu->caFileData,"share file ok!");
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:
        {

            QString strShareFilePath = QString("%1").arg((char*)pdu->iMsg);
            int index = strShareFilePath.lastIndexOf('/');
            QString strFileName = strShareFilePath.right(strShareFilePath.size()-index-1);
            QString strRecvPath = QString("./%1/%2").arg(pdu->caFileData).arg(strFileName);
            QFileInfo fileInfo(strShareFilePath);
            bool flag_File=false;
            bool flag_Dir=false;
            if(fileInfo.isFile()){
                flag_File=QFile::copy(strShareFilePath,strRecvPath);
            }else if(fileInfo.isDir()){
                  flag_Dir= copyDir(strShareFilePath,strRecvPath);
            }
            PDU* respdu = mkPDU(0);
            if(flag_File==true)
            strcpy(respdu->caFileData,"接受文件成功");
            else if(flag_Dir==true)
                strcpy(respdu->caFileData,"接受目录成功");
            else {
                strcpy(respdu->caFileData,"接受共享失败");
            }
            respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_REQUEST:{
            char caFileName[32] = {'\0'};
            int srcLen = 0;
            int destLen = 0;
            sscanf(pdu->caFileData,"%d %d %s",&srcLen,&destLen,caFileName);
            char *pSrcPath = new char[srcLen+1];
            char *pDestPath = new char[destLen+1+32];
            memset(pSrcPath,'\0',srcLen+1);
            memset(pDestPath,'\0',destLen+1+32);
            memcpy(pSrcPath,(char*)pdu->iMsg,srcLen);
            memcpy(pDestPath,(char*)pdu->iMsg+srcLen+1,destLen);
            QFileInfo fileInfo(pDestPath);
            PDU* respdu = mkPDU(0);
            respdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_RESPOND;
            if(fileInfo.isDir()){
                strcat(pDestPath,"/");
                strcat(pDestPath,caFileName);
                //将老文件移到新文件
                bool ret = QFile::rename(pSrcPath,pDestPath);
                if(ret){
                      strcpy(respdu->caFileData,MOVE_FILE_OK);
                }else{
                    strcpy(respdu->caFileData,"移动失败，已经存在于当前目录下");

                }

            } else if(fileInfo.isFile()){
                strcpy(respdu->caFileData,MOVE_FILE_FAILED);

            }
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
            break;
        }
        default:break;
        }
        free(pdu);
        pdu = NULL;
    }
    else {
        QByteArray buff = readAll();
        m_file.write(buff);
        m_iRecved += buff.size();
        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        if(m_iTotal==m_iRecved){
            m_file.close();
            m_bUploadT = false;

            strcpy(respdu->caFileData,UPLOAD_FILE_OK);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;

        } else if(m_iTotal<m_iRecved){
            m_file.close();
            m_bUploadT = false;
            strcpy(respdu->caFileData,UPLOAD_FILE_FAILURED);
            write((char*)respdu,respdu->uiPDULen);
            free(respdu);
            respdu = NULL;
        }

    }
    //qDebug() << caName << ' ' << caPwd <<' '<< pdu->uiMsgType; // 输出
}
//用户下线 断开连接要删除socket防止socket资源不够
void MyTcpSocket::clientOffline()
{
    //改变在线状态
    opeDB::getinstance().handleOffline(m_strName.toStdString().c_str());
    emit offline(this);// 发送给mytcpserver该socket删除信号
}

void MyTcpSocket::sendFileData()
{
    m_Timer->stop();
    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(1){
        ret = m_file.read(pBuffer,4096);
        if(ret>0&&ret<=4096){
            write(pBuffer,ret);
        } else if(0==ret){
            m_file.close();
            break;
        }
        else {
            qDebug()<<"发送文件内容给客户端时出现错误";
            m_file.close();
            break;
        }
    }
    delete []pBuffer;
    pBuffer = NULL;
}
