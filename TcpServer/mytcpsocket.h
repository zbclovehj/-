#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include "protocol.h"
#include <QTcpSocket>
#include "opedb.h"
#include <QDir>
#include<QTimer>
//用于控制每个和客户端通信的socket类 ,并存储客户端通信的用户名 可以用来确定socket
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();
    bool copyDir(QString strSrcDir,QString strDstDir);
public slots:
    void recvMsg();// 槽函数，按照协议形式处理传输过来的数据
    void clientOffline();
    void sendFileData();
signals:
    void offline(MyTcpSocket *mysocket);
private:
    QString m_strName;
    QFile m_file;
    qint64 m_iTotal;
    qint64 m_iRecved;
    bool m_bUploadT;
    QTimer *m_Timer;
};

#endif // MYTCPSOCKET_H
