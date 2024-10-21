#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include "protocol.h"
namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    static PrivateChat &getinstance();
    PrivateChat (PrivateChat const &)=delete;
    void operator=(PrivateChat const &) =delete ;
    ~PrivateChat();
    void setChatName(QString name);
    void updateMsg(const PDU *pdu);
private slots:
    void on_sendMsg_clicked();

private:
    explicit PrivateChat(QWidget *parent = nullptr);
    Ui::PrivateChat *ui;
    QString m_strChatName;
    QString m_strUserName;
};

#endif // PRIVATECHAT_H
