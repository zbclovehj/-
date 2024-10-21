#include "privatechat.h"
#include "ui_privatechat.h"
#include "protocol.h"
#include "tcpclient.h"
#include <QMessageBox>
PrivateChat::PrivateChat(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrivateChat)
{
    ui->setupUi(this);
}

PrivateChat &PrivateChat::getinstance()
{
    static PrivateChat pc;
    return pc;
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::setChatName(QString name)
{
    m_strChatName = name;
    m_strUserName = TcpClient::getinstance().getUserName();
}

void PrivateChat::updateMsg(const PDU *pdu)
{
if(NULL==pdu){
    return;
}
char sendName[32]={'\0'};
memcpy(sendName,pdu->caFileData,32);
QString strMsg = QString("%1 says :%2").arg(sendName).arg((char*)pdu->iMsg);
ui->showMsg->append(strMsg);

}

void PrivateChat::on_sendMsg_clicked()
{
    QString strMsg = ui->inputMsg->text();
    if(strMsg.size()!=0)
    ui->showMsg->append(QString("you say: %1").arg(strMsg));
    if(!strMsg.isEmpty()){
        PDU* pdu = mkPDU(strMsg.size()+32);
        pdu->uiMsgType=ENUM_MSG_TYPE_PRIVATE_CHAT_REQUEST;
        strcpy(pdu->caFileData,m_strUserName.toStdString().c_str());
        strcpy(pdu->caFileData+32,m_strChatName.toStdString().c_str());
        memcpy((char*)pdu->iMsg,strMsg.toStdString().c_str(),strMsg.size());
        TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
        free(pdu);
        pdu=NULL;
    } else {
        QMessageBox::warning(this,"私聊","不能发送空白信息");
    }

}
