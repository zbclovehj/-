#ifndef FRIEND_H
#define FRIEND_H

#include <QWidget>
#include <QTextEdit>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "online.h"

class Friend : public QWidget
{
    Q_OBJECT
public:
    explicit Friend(QWidget *parent = nullptr);
    void showAllUserOnline(PDU* pdu);
    void flushFriendList(PDU* pdu);
    QString m_strSearchName;
    void showGroupMsg(PDU* pdu);
    QListWidget *getFriendList();
signals:
public slots:
    void showOnline();
    void searchUser();
    void flushFriend();
    void deleteFriend();
    void privateChat();
    void groupChat();
private:
    QListWidget *m_pFriendLW;         // 好友列表
    QPushButton *m_pDelFriendPB;      // 删除好友
    QPushButton *m_pFlushFriendPB;    // 刷新好友列表
    QPushButton *m_pSOrHOnlineUserPB; // 显示/隐藏所有在线用户
    QPushButton *m_pSearchUserPB;     // 查找用户
    QLineEdit *m_pGroupInputLE;       // 群聊信息输入框
    QPushButton *m_pGroupSendMsgPB;   // 群聊发送消息
    QTextEdit *m_pGroupShowMsgTE;     // 显示群聊信息
    QPushButton *m_pPrivateChatPB;    // 私聊按钮，默认群聊
    QPushButton *m_pShowOnlineUserPB;
    // OnlineUserWid *m_pOnlineUserW;    // 所有在线用户页面
    Online *m_Online;
    QLineEdit *m_pInputMsgLE;         // 查找的用户的名字
    QTextEdit *m_pShowMsgTE;
    QPushButton *m_pSendMsgPB;

    // QList<PrivateChatWid*> m_priChatWidList; // 所有私聊的窗口
};

#endif // FRIEND_H
