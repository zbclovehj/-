#ifndef MYTCOSERVER_H
#define MYTCOSERVER_H
#include<QTcpServer>
#include<QList>
#include"mytcpsocket.h"

class MyTcpServer : public QTcpServer
{
    // 类既要继承QObject又要写上宏Q_OBJECT，才能支持信号槽
    Q_OBJECT
private:
    MyTcpServer();
public slots:
    void deleteSocket(MyTcpSocket *mytcpsocket);
public:
    /*
在实现服务器端的 TcpServer 时使用单例模式的主要原因包括：

全局访问: 单例模式确保在应用程序的整个生命周期内只有一个 TcpServer 实例存在，这使得所有模块或组件都可以轻松访问相同的 TcpServer 实例，而不需要传递实例或进行复杂的管理。
资源共享: 通过单例模式，可以确保 TcpServer 实例的资源（如端口、线程等）被正确共享和管理。多个模块需要使用相同的 TcpServer 实例时，单例模式能够提供一种简单的方式来实现资源共享。
维护连接状态: 在服务器端编程中，通常需要维护连接状态以处理客户端请求。使用单例模式可以确保所有的连接状态都集中在同一个 TcpServer 实例中，从而更容易管理和维护。
线程安全: 单例模式可以帮助避免多线程环境下的竞态条件，确保 TcpServer 实例在多线程环境下被正确访问和操作。这对于服务器端程序来说尤为重要，因为它需要处理多个客户端连接。
易于扩展和管理: 使用单例模式可以更容易地扩展 TcpServer 类，因为所有模块都依赖于同一个实例。此外，单例模式也使得对 TcpServer 实例的管理更加简单，避免了多个实例导致的混乱。
*/
    // 防止复制和赋值操作
    MyTcpServer(MyTcpServer const&) = delete;
    void operator=(MyTcpServer const&) = delete;
    //通过单例模式，使类实例唯一 返回类的引用，静态变量第一次初始化之后不变
    static MyTcpServer &getInstance();
    void incomingConnection(qintptr socketDescriptor);
    void resend(const char *friendname,PDU* pdu);

private:
    //在线的socket
    QList<MyTcpSocket*> m_tcpSocketList;// 存储服务器所有已经建立的Socket连接,存储与每个客户端通信的socket
};

#endif // MYTCOSERVER_H
