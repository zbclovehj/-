#include "mytcpserver.h"
#include <QDebug>
#include <QThread>
MyTcpServer::MyTcpServer()
{

}


/*
通过调用静态方法 getInstance() 来获取 MyTcpServer 类的单例实例。
单例模式是一种设计模式，确保一个类只有一个实例，并提供一个全局访问点。这种方式通常用于管理全局状态或资源。
这里假设 MyTcpServer 类可能实现了单例模式，通过 getInstance() 方法获取其唯一实例。
 */
MyTcpServer &MyTcpServer::getInstance()
{
    static MyTcpServer instance;
    return instance;
}
/*
 incomingConnection(qintptr socketDescriptor)
是QTcpServer类的虚拟槽函数，用于处理新的传入连接。
当有新的客户端连接到QTcpServer时，这个函数会被自动触发。通常，
在自定义的QTcpServer子类中重写这个函数，以便响应新连接的到来。
 */
//来了新的连接会自动执行这个函数 这个函数会被自动触发 会同时有多个这样的函数执行
void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
//    //使用线程池
//    qDebug()<<"new client connected";
//    MyTcpSocket *pTcpSocket = new MyTcpSocket;
//    pTcpSocket->setSocketDescriptor(socketDescriptor);
//    m_tcpSocketList.append(pTcpSocket);
//    QThread *thread=new QThread();
//    connect(pTcpSocket, SIGNAL(disconnected()),thread,SLOT(quit()));
//    connect(pTcpSocket,SIGNAL(offline(MyTcpSocket*)),
//    this, SLOT(deleteSocket(MyTcpSocket*)));
//    pTcpSocket->moveToThread(thread);
//    thread->start();
//    //断开与数据库的链
//    emit newConnection();

    qDebug()<<"new client connected";
    //有客户端连接就创建与之通信的socket
    MyTcpSocket *pTcpSocket = new MyTcpSocket();
    pTcpSocket->setSocketDescriptor(socketDescriptor);
    m_tcpSocketList.append(pTcpSocket);
    // 当 pTcpSocket 对象发出 offline 信号时，
    // 会触发当前对象（this）的 deleteSocket 槽函数，
    // 并且将 pTcpSocket 对象的指针作为参数传递给 deleteSocket 槽函数。
    // 这种机制实现了对象之间的松耦合通信。
    connect(pTcpSocket, SIGNAL(offline(MyTcpSocket *)), this, SLOT(deleteSocket(MyTcpSocket *)));
}

void MyTcpServer::resend(const char *friendname, PDU *pdu)
{
    if(NULL == friendname || NULL == pdu){
        return;
    }
    QString strName = friendname;
    for(int i = 0; i < m_tcpSocketList.size();++i){
        if(strName == m_tcpSocketList.at(i)->getName()){
            m_tcpSocketList.at(i)->write((char*)pdu,pdu->uiPDULen);
            break;
        }
    }
}
void MyTcpServer::deleteSocket(MyTcpSocket *mytcpsocket)
{
    qDebug()<<"delete socket:";
    QList<MyTcpSocket*>::iterator iter = m_tcpSocketList.begin();
    for(;iter!=m_tcpSocketList.end();++iter){
        if(mytcpsocket==*iter){
            QString name = mytcpsocket->getName();
            qDebug()<< name;
            (*iter)->deleteLater();
            *iter = NULL;
            m_tcpSocketList.erase(iter);
            break;
        }
    }

}
