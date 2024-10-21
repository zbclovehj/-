#ifndef OPEWIDGET_H
#define OPEWIDGET_H
#include <QWidget>
#include <QListWidget>
#include "book.h"
#include <QStackedWidget>
#include "friend.h"
class OpeWidget : public QWidget
{
    Q_OBJECT
private:
    explicit OpeWidget(QWidget *parent = nullptr);
public:
    static OpeWidget& getinstance();
    OpeWidget(OpeWidget const&) = delete;
    void operator=(OpeWidget const&) = delete;
    Friend *getfriend();
    Book *getBook();
signals:

private:
    QListWidget *m_pListW;
    Friend *m_pFriend;
    Book *m_pbook;
    QStackedWidget *m_pSW;
};

#endif // OPEWIDGET_H
