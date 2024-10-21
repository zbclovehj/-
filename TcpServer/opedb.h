#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStringList>
class opeDB : public QObject
{
    Q_OBJECT
public:
    explicit opeDB(QObject *parent = nullptr);
    ~opeDB();
    static opeDB& getinstance();
    void init();
    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    void handleOffline(const char *name);
    int handleSearchUser(const char *name);
    int handleAddFriend(const char *username,const char *friendname);
    QStringList handleAllOnline();
    void handleAddFriendAgree(const char *username,const char *friendname);
    QSet<QString> handleFlushFriend(const char *name);
    bool handleDeleteFriend(const char *username,const char *friendname);
private:

signals:

private:
    QSqlDatabase m_db;//连接数据库

};

#endif // OPEDB_H
