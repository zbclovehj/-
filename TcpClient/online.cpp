#include "online.h"
#include "ui_online.h"
#include "tcpclient.h"
#include <QDebug>
#include <QMessageBox>
Online::Online(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Online)
{
    ui->setupUi(this);
}

Online::~Online()
{
    delete ui;
}

void Online::showUser(PDU *pdu)
{
    if(NULL==pdu){
        return;
    }
    uint uiSize = pdu->uiMsgLen/32;
    char caTemp[32];
    for (uint i = 0;i<uiSize;++i) {
       memcpy(caTemp,(char*)(pdu->iMsg)+i*32,32);

       ui->onlineuser_lw->addItem(caTemp);

    }
}




void Online::on_addFriend_pb_clicked()
{
  QListWidgetItem* name = ui->onlineuser_lw->currentItem();
  QString friendName = name->text();
  if(friendName.isEmpty()){
      QMessageBox::warning(this,"添加好友","请选择要添加的好友");
      return;
  }
  QString userName = TcpClient::getinstance().getUserName();
  PDU *pdu = mkPDU(0);
  pdu->uiMsgType = ENUM_MSG_TYPE_ADD_FRIEND_REQUEST;
  memcpy(pdu->caFileData,userName.toStdString().c_str(),userName.size());
  memcpy(pdu->caFileData+32,friendName.toStdString().c_str(),friendName.size());
  TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
  free(pdu);
  pdu = NULL;
}
