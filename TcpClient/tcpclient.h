#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QWidget>
#include <QFile>
#include <QString>
#include <QTcpSocket>
#include "opewidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class TcpClient; }
QT_END_NAMESPACE

class TcpClient : public QWidget
{
    Q_OBJECT

public:

    ~TcpClient();
    void loadConfig();
    static TcpClient& getinstance();
    TcpClient(TcpClient const&) = delete;
    void operator=(TcpClient const&) = delete;
    QString getUserName();
    QString getCurPath();
    void setCurPath(QString s);
    QTcpSocket& getTcpSocket();
    QString getRootPath();
public slots:
    void showConnect();
private slots:
    //void on_send_pb_clicked();


    void on_login_db_clicked();

    void on_regist_pb_clicked();

    void on_cancel_db_clicked();
    void recvMsg();

private:
    TcpClient(QWidget *parent = nullptr);
    Ui::TcpClient *ui;
    QString m_strIP;
    quint16 m_usPort;
    //客户端与服务器连接的socket
    QTcpSocket m_tcpSocket;
    QString m_userName;
    // 每次登录进来的当前目录路劲为用户名
    QString m_strCurPath;
    //用户名路径 为 每个用户的根路径
    QString m_strRootPath;
    QFile m_file;
};
#endif // TCPCLIENT_H
