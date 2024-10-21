#include "tcpclient.h"
#include "ui_tcpclient.h"
#include <QByteArray>
#include <QDebug> // 调试
#include <QMessageBox> // 消息提示框
#include <QHostAddress>
#include "privatechat.h"
#include "protocol.h"
TcpClient::TcpClient(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpClient)
{
    //this表示为当前对象也就是类的指针 用于将设计好的 UI 组件加载到当前的 QWidget 中，以便在界面上显示。
    ui->setupUi(this);
    resize(500,250);
    this->loadConfig();
    //这行代码使用 connect 函数将 m_tcpSocket 对象的 connected() 信号连接到当前对象
    //（可能是一个自定义的类）的 showConnect() 槽函数上。
    //这意味着当 m_tcpSocket 成功连接到远程主机时，showConnect() 槽函数将被触发执行。
    connect(&m_tcpSocket,SIGNAL(connected()),this,SLOT(showConnect()));
    connect(&m_tcpSocket,SIGNAL(readyRead()),this,SLOT(recvMsg()));
    m_tcpSocket.connectToHost(QHostAddress(m_strIP),m_usPort);
}

TcpClient::~TcpClient()
{
    delete ui;
}

void TcpClient::loadConfig()
{
    QFile file(":/client.config");
    if(file.open(QIODevice::ReadOnly)){
        //字节数组读取全部
        QByteArray baData =  file.readAll();
        QString strData = baData.toStdString().c_str();
        //   qDebug() << strData;
        file.close();
        strData.replace("\r\n"," ");
        QStringList strlist = strData.split(" ");
        for (int i = 0;i<strlist.size();++i) {
            qDebug()<<strlist[i];
        }
        m_strIP = strlist.at(0);
        m_usPort = strlist.at(1).toUInt();
        qDebug() << "ip:"<<m_strIP<<" port:"<<m_usPort;

    } else {
        QMessageBox::critical(this,"open config","read failed");
    }

}

TcpClient &TcpClient::getinstance()
{
    //这意味着 TcpClient 的生存周期将贯穿整个程序运行期间，并且在函数调用结束后不会被销毁。
    //每次调用都是这个单例返回
    static TcpClient tcp;
    return tcp;
}

QString TcpClient::getUserName()
{
    return m_userName;
}

QString TcpClient::getCurPath()
{
    return m_strCurPath;
}

void TcpClient::setCurPath(QString s)
{
    m_strCurPath = s;
}

QTcpSocket &TcpClient::getTcpSocket()
{
    return m_tcpSocket;
}

QString TcpClient::getRootPath()
{
    return m_strRootPath;
}

void TcpClient::showConnect()
{
    QMessageBox::information(this,"连接服务器","连接服务器成功");
}
/*
// 客户端点击发送按钮事件
void TcpClient::on_send_pb_clicked()
{
    QString strMsg = ui->lineEdit->text();

    if(!strMsg.isEmpty()) // 消息非空才发送
    {
        //加了一个字节，以防万一 回车空格等等
        PDU *pdu = mkPDU(strMsg.toUtf8().size()+1);
        pdu -> uiMsgType = 0; // 消息类型
        strncpy((char*)pdu->iMsg,strMsg.toUtf8().toStdString().c_str(),strMsg.toUtf8().size()); // 将需要传递的信息拷贝到协议数据单元中
        m_tcpSocket.write((char*)pdu, pdu -> uiPDULen); // 通过socket发送信息
        // 释放空间
        free(pdu);
        pdu = NULL;
    }
    else // 消息为空警告
    {
        QMessageBox::warning(this, "信息发送", "发送的信息不能为空");
    }
}


 */

void TcpClient::on_login_db_clicked()
{
    QString strName = ui->name_lab->text();
    QString strPwd = ui->pwd_lab->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        m_userName = strName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_LOGIN_REQUEST;
        //通过转化为c风格字符串也就是字符数组 然后通过首地址来复制
        strncpy(pdu->caFileData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caFileData+32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{
        QMessageBox::warning(this, "登录", "登录失败：用户名或密码为空");
    }
}

void TcpClient::on_regist_pb_clicked()
{
    QString strName = ui->name_lab->text();
    QString strPwd = ui->pwd_lab->text();
    if(!strName.isEmpty() && !strPwd.isEmpty()){
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_REGIST_REQUEST;
        //通过转化为c风格字符串也就是字符数组 然后通过首地址来复制
        strncpy(pdu->caFileData, strName.toStdString().c_str(), 32);
        strncpy(pdu->caFileData+32, strPwd.toStdString().c_str(), 32);
        m_tcpSocket.write((char*)pdu, pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{
        QMessageBox::warning(this, "注册", "注册失败：用户名或密码为空");
    }
}

void TcpClient::on_cancel_db_clicked()
{
    //m_tcpSocket.disconnected();
}

void TcpClient::recvMsg()
{
    if(!OpeWidget::getinstance().getBook()->gettDwonloadStatus()){
        qDebug()<<m_tcpSocket.bytesAvailable();// 获取接收到的数据大小
        uint uiPDULen = 0;
        //先读取uiPDULen
        m_tcpSocket.read((char*)&uiPDULen,sizeof (uint));// 先读取uint大小的数据，首个uint正是总数据大小
        uint uiMsgLen = uiPDULen-sizeof(PDU);// 实际消息大小，sizeof(PDU)只会计算结构体大小，而不是分配的大小
        //以及赋值了 uiPDULen uiMsgLen
        PDU *pdu = mkPDU(uiMsgLen);
        //后面的数据就是 // 消息类型// 文件名称    // 实际消息长度    // 实际消息，主要通过iMsg访问消息数据
        m_tcpSocket.read((char*)pdu+sizeof(uint),uiPDULen-sizeof(uint));// 接收剩余部分数据（第一个uint已读取）并且从pdu对应的内存地址开始存入数据
        switch (pdu->uiMsgType) {
        case ENUM_MSG_TYPE_REGIST_RESPOND:
        {
            if(0 == strcmp(pdu->caFileData,REGIST_OK)){
                QMessageBox::information(this,"注册",REGIST_OK);
            }else if(0 == strcmp(pdu->caFileData,REGIST_FAILED)){
                QMessageBox::information(this,"注册",REGIST_FAILED);
            }

            break;
        };
        case ENUM_MSG_TYPE_LOGIN_RESPOND:
        {
            if(0 == strcmp(pdu->caFileData,LOGIN_OK)){
                //登录进来保存当前目录路径
                //每次登录进来的当前目录路劲为用户名
                m_strCurPath = QString("./%1").arg(m_userName);
                m_strRootPath = QString("./%1").arg(m_userName);
                QMessageBox::information(this,"登录",LOGIN_OK);
                //登录成功后显示界面通过静态对象去获取操作界面
                OpeWidget::getinstance().show();
                //隐藏客户端的登录注册界面
                this->hide();
                //刷新好友
                PDU *pdu = mkPDU(0);
                pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
                strcpy(pdu->caFileData,m_userName.toStdString().c_str());
                m_tcpSocket.write((char*)pdu,pdu->uiPDULen);
                free(pdu);
                pdu = NULL;
                break;
            }else if(0 == strcmp(pdu->caFileData,LOGIN_FAILED)){
                QMessageBox::information(this,"登录",LOGIN_FAILED);
            }

            break;
        };
        case ENUM_MSG_TYPE_ALL_ONLINE_RESPOND:
        {
            OpeWidget::getinstance().getfriend()->showAllUserOnline(pdu);
            break;
        };
        case ENUM_MSG_TYPE_SEARCH_USER_RESPOND:
        {
            if(0 == strcmp(SERACH_USER_NO,pdu->caFileData)){
                QMessageBox::information(this,"搜索",QString("%1:no exist").arg(OpeWidget::getinstance().getfriend()->m_strSearchName));
            } else if(0 == strcmp(SEARCH_USER_ONLINE,pdu->caFileData)){
                QMessageBox::information(this,"搜索",QString("%1: exist and online").arg(OpeWidget::getinstance().getfriend()->m_strSearchName));
            }else if(0 == strcmp(SEARCH_USER_OFFLINE,pdu->caFileData)){
                QMessageBox::information(this,"搜索",QString("%1: exist but offline").arg(OpeWidget::getinstance().getfriend()->m_strSearchName));
            }

            break;
        };
        case ENUM_MSG_TYPE_ADD_FRIEND_REQUEST:
        {

            char pername[32] = {'\0'};
            strncpy(pername,pdu->caFileData,32);
            char username[32] = {'\0'};
            strncpy(username,pdu->caFileData+32,32);
            if(0!=strcmp(username,pername)){
                int ret = QMessageBox::information(this,"添加好友",QString("%1 want to add you as friend?").arg(pername),QMessageBox::Yes,QMessageBox::No);
                PDU* resPDU = mkPDU(0);
                memcpy(resPDU->caFileData,pdu->caFileData,64);
                if(ret==QMessageBox::Yes){
                    resPDU->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;

                } else {
                    resPDU->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
                }
                m_tcpSocket.write((char*)resPDU,resPDU->uiPDULen);
                free(resPDU);
                resPDU=NULL;
            } else {
                int ret = QMessageBox::information(this,"添加好友",QString("you want to add yourself?"),QMessageBox::Yes,QMessageBox::No);
                PDU* resPDU = mkPDU(0);
                memcpy(resPDU->caFileData,pdu->caFileData,64);
                if(ret==QMessageBox::Yes){
                    resPDU->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_AGGREE;

                } else {
                    resPDU->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REFUSE;
                }
                m_tcpSocket.write((char*)resPDU,resPDU->uiPDULen);
                free(resPDU);
                resPDU=NULL;
            }

            break;
        };
        case ENUM_MSG_TYPE_ADD_FRIEND_RESPOND:
        {
            if(0 == strcmp(pdu->caFileData,CAN_ADD)){
                QMessageBox::information(this,"添加好友","已发送添加好友请求");
            } else {
                QMessageBox::information(this,"添加好友",pdu->caFileData);
            }
            break;
        };
        case ENUM_MSG_TYPE_ADD_FRIEND_AGGREE:
        {
            QMessageBox::information(this,"添加好友",pdu->caFileData);
            break;
        };
        case ENUM_MSG_TYPE_ADD_FRIEND_REFUSE:
        {

            QMessageBox::information(this,"添加好友",pdu->caFileData);

            break;
        };
        case ENUM_MSG_TYPE_FLUSH_FRIEND_RESPOND:
        {
            //flushFriendList(pdu);
            OpeWidget::getinstance().getfriend()->flushFriendList(pdu);

            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_RESPOND:
        {
            QMessageBox::information(this,"删除好友",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST:
        {
            char username[32]={'\0'};
            strncpy(username,pdu->caFileData,32);
            QMessageBox::information(this,"删除好友",QString("%1 delete you").arg(username));
            break;
        }
        case ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST:
        {
            char username[32]={'\0'};
            memcpy(username,pdu->caFileData,32);
            //显示私聊界面
            if(PrivateChat::getinstance().isHidden())
            {
                int ret = QMessageBox::information(this,"私聊",QString("%1 send message to you,is see?").arg(username),QMessageBox::Yes,QMessageBox::No);
                if(ret==QMessageBox::Yes){
                    PrivateChat::getinstance().show();
                }

            }
            PrivateChat::getinstance().setChatName(QString(username));
            PrivateChat::getinstance().updateMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_GROUP_CHAT_REQUEST:
        {
            OpeWidget::getinstance().getfriend()->showGroupMsg(pdu);
            break;
        }
        case ENUM_MSG_TYPE_CRATE_DIR_RESPOND:
        {
            QMessageBox::information(this,"创建文件夹",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_FLUSH_FILE_RESPOND:
        {
            if(!OpeWidget::getinstance().getBook()->getEnterNameDir().isEmpty()){
                m_strCurPath =m_strCurPath +"/"+ OpeWidget::getinstance().getBook()->getEnterNameDir();
            }
            OpeWidget::getinstance().getBook()->updateFileList(pdu);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_DIR_RESPOND:
        {
            QMessageBox::information(this,"删除文件夹",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_NEW_NAME_RESPOND:
        {
            QMessageBox::information(this,"重命名文件夹",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_ENTER_DIR_RESPOND:
        {
            QMessageBox::information(this,"进入文件夹",pdu->caFileData);
            OpeWidget::getinstance().getBook()->clearEnterDir();
            break;
        }
        case ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND:{
            QMessageBox::information(this,"上传文件",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_DELETE_FILE_RESPOND:
        {
            QMessageBox::information(this,"删除文件夹",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_DOWNLOAD_FILE_RESPOND:
        {
            qDebug()<<pdu->caFileData;
            char fileName[32] = {'\0'};
            qint64 fileSize;
            sscanf(pdu->caFileData,"%s %lld",fileName,
                   &fileSize);
            OpeWidget::getinstance().getBook()->setTotalSize(fileSize);
            if(strlen(fileName)>0&&fileSize>0){
                OpeWidget::getinstance().getBook()->setDwonloadStatus(true);
                m_file.setFileName(OpeWidget::getinstance().getBook()->getDwonloadFilePath());
                //如果没用这个文件则创建
                qDebug()<<"保存路径为："<<OpeWidget::getinstance().getBook()->getDwonloadFilePath();
                if(!m_file.open(QIODevice::WriteOnly)){
                    QMessageBox::information(this,"下载文件","下载失败，获取不到下载位置");
                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_RESPOND:
        {
            QMessageBox::information(this,"共享文件",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE:
        {
            char *pPath = new char[pdu->uiMsgLen];
            memcpy(pPath,(char*)pdu->iMsg,pdu->uiMsgLen);
            char *pos = strrchr(pPath,'/');
            if(NULL != pos){
                pos++;
                QString strNote = QString("%1 share file-> %2!\n Do you accept?").arg(pdu->caFileData).arg(pos);
                int ret = QMessageBox::question(this,"共享文件",strNote);
                if(QMessageBox::Yes==ret){
                    PDU* respdu = mkPDU(pdu->uiMsgLen);
                    respdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND;
                    memcpy((char*)respdu->iMsg,(char*)pdu->iMsg,pdu->uiMsgLen);
                    QString strName = TcpClient::getinstance().getUserName();
                    strcpy((char*)respdu->caFileData,strName.toStdString().c_str());
                    m_tcpSocket.write((char*)respdu,respdu->uiPDULen);
                    free(respdu);
                    respdu=NULL;
                } else {
                    QMessageBox::information(this,"共享文件","您已拒绝接受共享");


                }
            }
            break;
        }
        case ENUM_MSG_TYPE_SHARE_FILE_NOTE_RESPOND:{
            QMessageBox::information(this,"共享文件",pdu->caFileData);
            break;
        }
        case ENUM_MSG_TYPE_MOVE_FILE_RESPOND:{
            QMessageBox::information(this,"移动文件",pdu->caFileData);
            break;
        }
        default:break;
        }
        free(pdu);
        pdu = NULL;
    }
    else{
        QByteArray buff = m_tcpSocket.readAll();
        m_file.write(buff);
        Book* pBook = OpeWidget::getinstance().getBook();
        qint64 nowSize = pBook->getRecvedSize();
        nowSize+=buff.size();
        pBook->setRecvedSize(nowSize);

        PDU* respdu = mkPDU(0);
        respdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_RESPOND;
        if(pBook->getTotalSize()==pBook->getRecvedSize()){
            m_file.close();
            pBook->setDwonloadStatus(false);
            pBook->setTotalSize(0);
            pBook->setRecvedSize(0);
            QMessageBox::information(this,"下载文件","下载成功");
        } else if(pBook->getTotalSize() < pBook->getRecvedSize()){
            m_file.close();
            pBook->setDwonloadStatus(false);
            pBook->setTotalSize(0);
            pBook->setRecvedSize(0);
            QMessageBox::critical(this,"下载文件","下载失败");

        }
    }
}
