#include "sharefile.h"
#include "tcpclient.h"
#include "protocol.h"
#include <QCheckBox>
#include <QMessageBox>
ShareFile::ShareFile(QWidget *parent) : QWidget(parent)
{
    m_pSelectAllPB = new QPushButton("全选");
    m_pCancelSelectAllPB = new QPushButton("取消选择");
    m_pOKPB = new QPushButton("确定");
    m_pCancelPB = new QPushButton("取消");
    m_pSA = new QScrollArea;
    m_pFriendW = new QWidget;
    m_pButtonGroup = new QButtonGroup(m_pFriendW);
    //设置允许多选
    m_pButtonGroup->setExclusive(false);
    m_pFriendWVBL = new QVBoxLayout(m_pFriendW);

    QHBoxLayout *pTopHBL = new QHBoxLayout;
    pTopHBL->addWidget(m_pSelectAllPB);
    pTopHBL->addWidget(m_pCancelSelectAllPB);
    pTopHBL->addStretch();

    QHBoxLayout *pDownHBL = new QHBoxLayout;
    pDownHBL->addWidget(m_pOKPB);
    pDownHBL->addWidget(m_pCancelPB);

    QVBoxLayout* pMainVBL = new QVBoxLayout;
    pMainVBL->addLayout(pTopHBL);
    pMainVBL->addWidget(m_pSA);
    pMainVBL->addLayout(pDownHBL);
    setLayout(pMainVBL);
    connect(m_pCancelSelectAllPB,SIGNAL(clicked(bool)),
            this,SLOT(cancelSelect()));
    connect(m_pSelectAllPB,SIGNAL(clicked(bool)),
            this,SLOT(selectAll()));
    connect(m_pOKPB,SIGNAL(clicked(bool)),
            this,SLOT(okShare()));
    connect(m_pCancelPB,SIGNAL(clicked(bool)),
            this,SLOT(cancelShare()));
}

ShareFile &ShareFile::getinstance()
{
    static ShareFile instance;
    return instance;

}

void ShareFile::updateFriend(QListWidget *friendLW)
{
    if(friendLW == NULL){
        return;
    }
    QAbstractButton* temp = NULL;
    QList<QAbstractButton*> preFriendList =  m_pButtonGroup->buttons();
    for(int i = 0;i < preFriendList.size();++i){
        temp = preFriendList[i];
        m_pFriendWVBL->removeWidget(temp);
        m_pButtonGroup->removeButton(temp);
        preFriendList.removeOne(temp);
        delete temp;
        temp = NULL;
    }
    QCheckBox *pCB = NULL;
    for (int i = 0; i < friendLW->count(); ++i) {
        pCB = new QCheckBox(friendLW->item(i)->text());
        m_pFriendWVBL->addWidget(pCB);
        m_pButtonGroup->addButton(pCB);
    }
    m_pSA->setWidget(m_pFriendW);
}

void ShareFile::cancelSelect()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i = 0; i < cbList.size(); ++i) {
        if(cbList[i]->isChecked()){
            cbList[i]->setChecked(false);
        }
    }
}

void ShareFile::selectAll()
{
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    for (int i = 0; i < cbList.size(); ++i) {
        if(!cbList[i]->isChecked()){
            cbList[i]->setChecked(true);
        }
    }
}

void ShareFile::okShare()
{
    QString strName = TcpClient::getinstance().getUserName();
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QString strShareFileName = OpeWidget::getinstance().getBook()->getShareFileName();
    QString strPath = strCurPath + "/" + strShareFileName;
    QList<QAbstractButton*> cbList = m_pButtonGroup->buttons();
    int num = 0;
    for (int i = 0; i < cbList.size(); ++i) {
        if(cbList[i]->isChecked()){
            num++;
        }
    }
    if(num==0){
       QMessageBox::information(this,"共享文件","请选择共享对象");
       return;
    }
    PDU* pdu = mkPDU(32*num+strPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_SHARE_FILE_REQUEST;
    //输出到第一个参数
    sprintf(pdu->caFileData,"%s %d",strName.toStdString().c_str(),num);
    int j = 0;
    for(int i = 0; i < num; ++i){
        if(cbList[i]->isChecked())
        memcpy((char*)(pdu->iMsg)+(j++)*32,cbList[i]->text().toStdString().c_str(),32);

    }
    memcpy((char*)pdu->iMsg+num*32,strPath.toStdString().c_str(),strPath.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void ShareFile::cancelShare()
{
     this->hide();
}
