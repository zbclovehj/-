#include "friend.h"
#include "tcpclient.h"
#include <QString>
#include <QInputDialog>
#include <QDebug>
#include <QMessageBox> // 消息提示框
#include "privatechat.h"
Friend::Friend(QWidget *parent) : QWidget(parent)
{
    m_pFriendLW = new QListWidget();
    m_pInputMsgLE = new QLineEdit();
    m_pShowMsgTE = new QTextEdit();
    m_pDelFriendPB = new QPushButton("删除好友");
    m_pFlushFriendPB = new QPushButton("刷新好友");
    m_pShowOnlineUserPB = new QPushButton("显示在线用户");
    m_pSearchUserPB = new QPushButton("查找用户");
    m_pSendMsgPB = new QPushButton("发送信息");
    m_pPrivateChatPB = new QPushButton("私聊");

    QVBoxLayout *pLeftVBL = new QVBoxLayout(); // 左侧右部分好友操作按钮布局
    pLeftVBL -> addWidget(m_pPrivateChatPB);
    pLeftVBL -> addWidget(m_pDelFriendPB);
    pLeftVBL -> addWidget(m_pFlushFriendPB);
    pLeftVBL -> addWidget(m_pShowOnlineUserPB);
    pLeftVBL -> addWidget(m_pSearchUserPB);
    QHBoxLayout *pTopHBL = new QHBoxLayout(); // 右侧下方发送消息布局
    pTopHBL -> addWidget(m_pShowMsgTE);
    pTopHBL -> addWidget(m_pFriendLW);
    pTopHBL->addLayout(pLeftVBL);
    QHBoxLayout *pMsgVBL = new QHBoxLayout(); // 右侧聊天布局
    pMsgVBL -> addWidget(m_pInputMsgLE);
    pMsgVBL -> addWidget(m_pSendMsgPB);
    m_Online = new Online();
    QVBoxLayout *pAllHBL = new QVBoxLayout(); // 整体垂直布局
    pAllHBL -> addLayout(pTopHBL);    // 左侧右部分好友操作
    pAllHBL -> addLayout(pMsgVBL);  // 右侧聊天布局
    pAllHBL->addWidget(m_Online);
    m_Online->hide();
    setLayout(pAllHBL); // 将整体布局pAllHBL设置为页面布局
    // 当 m_pShowOnlineUserPB 对象（可能是一个按钮）被点击时，
    // 会触发与当前对象（this）关联的 showOnline() 槽函数，从而执行与显示在线用户相关的操作。
    //通过信号与槽来触发事件
    connect(m_pShowOnlineUserPB,SIGNAL(clicked(bool)),
            this,SLOT(showOnline()));
    connect(m_pSearchUserPB,SIGNAL(clicked(bool)),
            this,SLOT(searchUser()));
    connect(m_pFlushFriendPB,SIGNAL(clicked(bool)),
            this,SLOT(flushFriend()));
    connect(m_pDelFriendPB,SIGNAL(clicked(bool)),
            this,SLOT(deleteFriend()));
    connect(m_pPrivateChatPB,SIGNAL(clicked(bool)),
            this,SLOT(privateChat()));
    connect(m_pSendMsgPB,SIGNAL(clicked(bool)),
            this,SLOT(groupChat()));
}

void Friend::showAllUserOnline(PDU *pdu)
{
    if(NULL==pdu){
        return;
    }
    m_Online->showUser(pdu);
}

void Friend::flushFriendList(PDU *pdu)
{
    m_pFriendLW->clear();
    if(NULL==pdu){
        return;
    }
    uint uiSize = pdu->uiMsgLen / 32;
    char friendName[32] = {'\0'};
    for(uint i = 0; i < uiSize;++i){
        memcpy(friendName,(char*)(pdu->iMsg)+i*32,32);
        m_pFriendLW->addItem(friendName);
    }
}

void Friend::showGroupMsg(PDU *pdu)
{
    m_pShowMsgTE->append(QString((char*)pdu->iMsg));
}

QListWidget* Friend::getFriendList()
{
    return m_pFriendLW;
}

void Friend::deleteFriend()
{
    if(NULL!=m_pFriendLW->currentItem())
    {
        QString deletedFriend = m_pFriendLW->currentItem()->text();
        PDU* pdu = mkPDU(0);
        QString userName = TcpClient::getinstance().getUserName();
        strcpy((char*)pdu->caFileData,userName.toStdString().c_str());
        strcpy(((char*)pdu->caFileData)+32,deletedFriend.toStdString().c_str());
        pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FRIEND_REQUEST;
        TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    } else {
        QMessageBox::information(this,"删除好友","请选择要删除的好友");
    }

}

void Friend::privateChat()
{
    if(NULL!=m_pFriendLW->currentItem())
    {
        QString friendName = m_pFriendLW->currentItem()->text();
        PrivateChat::getinstance().setChatName(friendName);
        if(PrivateChat::getinstance().isHidden()){
            PrivateChat::getinstance().show();
        } else {
            PrivateChat::getinstance().hide();
        }
    } else {
        QMessageBox::information(this,"私聊","请选择私聊的好友");
    }
}

void Friend::groupChat()
{
    QString strMsg = m_pInputMsgLE->text();
    if(!strMsg.isEmpty()){
        m_pShowMsgTE->append(QString("you say: %1").arg(strMsg));
        strMsg = QString("%1 say: %2").arg(TcpClient::getinstance().getUserName()).arg(strMsg);

        PDU* pdu = mkPDU(strMsg.size()+32);
        pdu->uiMsgType = ENUM_MSG_TYPE_GROUP_CHAT_REQUEST;
        strcpy(pdu->caFileData,TcpClient::getinstance().getUserName().toStdString().c_str());
        strcpy((char*)pdu->iMsg,strMsg.toStdString().c_str());
        TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    } else {
        QMessageBox::information(this,"群聊","不能发送空白信息");
    }
}

void Friend::searchUser()
{
    m_strSearchName = QInputDialog::getText(this, "搜索", "用户名："); // 通过输入子页面来获取用户输入返回一个文本类型
    if(!m_strSearchName.isEmpty()){
        qDebug() << m_strSearchName;
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_SEARCH_USER_REQUEST;
        memcpy((char*)pdu->caFileData,m_strSearchName.toStdString().c_str(),m_strSearchName.size());
        TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }
}

void Friend::flushFriend()
{
    QString userName = TcpClient::getinstance().getUserName();
    PDU *pdu = mkPDU(0);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FRIEND_REQUEST;
    strcpy(pdu->caFileData,userName.toStdString().c_str());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu = NULL;
}

void Friend::showOnline()
{
    if(m_Online->isHidden()){
        //点击了查看在线用户按钮，向服务器端请求数据
        //获得当前的socket
        m_Online->show();
        PDU *pdu = mkPDU(0);
        pdu->uiMsgType = ENUM_MSG_TYPE_ALL_ONLINE_REQUEST;
        TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu = NULL;
    }else{
        m_Online->hide();

        // 遍历 m_Online 的子控件并根据类型清空内容
        foreach (QObject *child, m_Online->children()) {
            // 尝试将 QObject* 类型的对象转换为指定类型
            // child 对象是 QListWidget 类型或者是 QListWidget 的派生类才能转换成功
            if (QListWidget *listWidget = qobject_cast<QListWidget*>(child)) {
                // 清空列表框内容
                listWidget->clear();
            }
            // 添加其他控件类型的清空逻辑
        }
    }
}
