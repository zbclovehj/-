#include "tcpserver.h"
#include "ui_tcpserver.h"
#include "mytcpserver.h"
#include <QByteArray>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
TcpServer::TcpServer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TcpServer)
{
    ui->setupUi(this);
    this->loadConfig();
    //通过全局方法去获取tcpserver的实例
    //MyTcpServer *m = new MyTcpServer; 用单例模式，私有构造方法
    //一直listen 负责监听
    MyTcpServer::getInstance().listen(QHostAddress(m_strIP),m_usPort);

}

TcpServer::~TcpServer()
{
    delete ui;
}

void TcpServer::loadConfig()
{
    QFile file(":/server.config");
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


