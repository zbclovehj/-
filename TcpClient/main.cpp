#include "tcpclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //    OpeWidget w;
    //    w.show();
    TcpClient::getinstance().show();
    //    Friend f;
    //    f.show();
    //    Online o;
    //    o.show();
    return a.exec();
}
