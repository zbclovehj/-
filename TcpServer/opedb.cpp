#include "opedb.h"
#include <QMessageBox>
#include <QDebug>
#include <QSet>
opeDB& opeDB::getinstance()
{
    //静态变量在程序启动时被初始化，保证了类的实例只会被创建一次，
    //从而实现了单例模式的效果。
    //静态变量的生命周期与程序的生命周期相同，
    //因此在整个程序执行过程中，单例实例都可以被访问和使用。
    static opeDB instance;
    return instance;
}

void opeDB::init()
{
    m_db.setHostName("localhost");
    m_db.setDatabaseName("C:\\NetDisk_Project\\TcpServer\\cloud.db");
    if(m_db.open()){
        QSqlQuery query;
        query.exec("select * from userInfo");
        while(query.next())
        {
            QString data = QString("%1, %2, %3, %4").arg(query.value(0).toString()).arg(query.value(1).toString())
                    .arg(query.value(2).toString()).arg(query.value(3).toString());
            qDebug() << data;
        }
    } else {
        QMessageBox::critical(NULL, "数据库打开", "数据库打开失败");
    }
}

bool opeDB::handleRegist(const char *name, const char *pwd)
{
    // 考虑极端情况
    if(NULL == name || NULL == pwd)
    {
        return false;
    }
    // 数据插入数据库
    QString strinsert = QString("insert into usrInfo(name, pwd) values('%1', '%2')").arg(name).arg(pwd);
    QSqlQuery insert;
    //qDebug() << strQuery;
    //执行插入语句
    return insert.exec(strinsert); // 数据库中name索引是unique，所以如果name重复会返回false，插入成功返回true
}
bool opeDB::handleLogin(const char *name, const char *pwd)
{
    // 考虑极端情况
    if(NULL == name || NULL == pwd)
    {
        return false;
    }
    // 查询数据库
    QString strSelect = QString("select * from usrInfo where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    QSqlQuery select;
    //qDebug() << strQuery;
    QString strCount = QString("SELECT COUNT(*) FROM usrInfo WHERE name = '%1' AND pwd = '%2' AND online = 0").arg(name).arg(pwd);
    QSqlQuery countQuery;
    if (countQuery.exec(strCount)) {
        if (countQuery.next()) {// 尝试获取第一行
            /*
调用 QSqlQuery 对象的 value() 方法，并传入参数 0。
这里的 0 表示查询结果集中的第一列（在SQL中，列是从0开始计数的）。
因为 COUNT(*) 查询通常只返回一个列（即计数结果），
所以这里使用 0 来获取这个值。value() 方法返回一个 QVariant 对象
该对象可以存储Qt支持的各种数据类型。
*/
            int rowCount = countQuery.value(0).toInt(); // 假设COUNT(*)是查询结果的第一列
            // 现在rowCount包含了满足条件的记录数
            if(rowCount>1||rowCount==0){
                return false;
            }
        }
    }
    //执行查询语句
    select.exec(strSelect); // 数据库中name索引是unique，所以如果name重复会返回false，插入成功返回true
    if(select.next()){
        QString strUpdate = QString("update usrInfo set online=1 where name=\'%1\' and pwd = "
                                    "\'%2\'").arg(name).arg(pwd);
        QSqlQuery update;
        update.exec(strUpdate);
        return true;
    }
    return false;
}

void opeDB::handleOffline(const char *name)
{
    // 考虑极端情况
    if(NULL == name)
    {
          qDebug() << "name is NULL";
        return;
    }
    QString updates = QString("update usrInfo set online = 0 where name = \'%1\'").arg(name);
    QSqlQuery updatess;
    updatess.exec(updates);
}
//查询好友
int opeDB::handleSearchUser(const char *name)
{
    if(NULL==name){
        return -1;
    }
    QString data = QString("select online from usrInfo where name = \'%1\'").arg(name);
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        int ret = query.value(0).toInt();
        return ret;
    }
    return -1;//不存在
}
//添加好友
int opeDB::handleAddFriend(const char *username, const char *friendname)
{
    if(NULL==username || NULL==friendname){
        return -1;
    }
    QString strQuery = QString("select * from fride "
                                  "where (id = (select id from usrInfo where name = \'%1\') and "
                                  "friendId = (select id from usrInfo where name = \'%2\')) or "  // 好友是双向的，数据库只存了单向，注意是or关系
                                  "(id = (select id from usrInfo where name = \'%3\') and "
                                  "friendId = (select id from usrInfo where name = \'%4\'))")
               .arg(username).arg(friendname).arg(friendname).arg(username);
   QSqlQuery query;
   query.exec(strQuery);
   if(query.next()){
       return 2;//已经是好友
   } else {
       //查询好友是否存在并且在线
       return handleSearchUser(friendname);//不是好友需要添加
   }
}

QStringList opeDB::handleAllOnline()
{
QString searchAllUser = QString("select name from usrInfo where online=1");
QSqlQuery query;
QStringList userList;
query.exec(searchAllUser);
userList.clear();
while(query.next())
{
    userList.append(query.value(0).toString());
}
return userList;
}

void opeDB::handleAddFriendAgree(const char *username, const char *friendname)
{
  QString userid = QString("select id from usrInfo where name = \'%1\'").arg(username);
  QString friendid = QString("select id from usrInfo where name = \'%1\'").arg(friendname);
  QSqlQuery query1;
  QSqlQuery query2;
  query1.exec(userid);
  query2.exec(friendid);
  int id_user = -1,id_friend = -1;
  if(query1.next()){
       id_user = query1.value(0).toInt();

  }
  if(query2.next()){
       id_friend = query2.value(0).toInt();

  }
  QString insertData =   QString(" insert into fride(id,friendId) values(%1,%2); ").arg(id_user).arg(id_friend);
  QSqlQuery insert;
  insert.exec(insertData);

}

QSet<QString> opeDB::handleFlushFriend(const char *name)
{
 QSet<QString> strFriendList;
 strFriendList.clear();
 if(NULL==name){
     return strFriendList;
 }
 QString data = QString("select id from usrInfo where name=\'%1\' and online=1").arg(name);
 QSqlQuery query;
 query.exec(data);
  int id;
 if(query.next()){
     id = query.value(0).toInt();
     data = QString("select friendId from fride where id =\'%1\'").arg(id);
     query.clear();
     query.exec(data);
     while(query.next()){
       int friendId = query.value(0).toInt();
       data = QString("select name from usrInfo where id=\'%1\' and online=1").arg(friendId);
       QSqlQuery select;
       select.exec(data);
       if(select.next())
       {
           QString friendName = select.value(0).toString();
           strFriendList.insert(friendName);
       }
     }
     query.clear();
     data = QString("select id from fride where friendId =\'%1\'").arg(id);
     query.exec(data);
     while(query.next()){
       int friendId = query.value(0).toInt();
       data = QString("select name from usrInfo where id=\'%1\' and online=1").arg(friendId);
       QSqlQuery select;
       select.exec(data);
       if(select.next())
       {
           QString friendName = select.value(0).toString();
           strFriendList.insert(friendName);
       }
     }
 } else {
return strFriendList;
}

 return strFriendList;
}

bool opeDB::handleDeleteFriend(const char *username, const char *friendname)
{
    if(NULL == username || NULL == friendname){
        return false;
    }
    QString data  = QString("delete from fride where "
                            "(id=(select id from usrInfo where name = \'%1\') "
                            "and friendId = (select id from usrInfo where name = \'%2\')) "
                            "or(id=(select id from usrInfo where name = \'%2\') and "
                            "friendId = (select id from usrInfo where name = \'%1\'))").arg(username).arg(friendname);

    QSqlQuery deleteS;
    if(deleteS.exec(data)){
        return true;
    }
    return false;
}
opeDB::opeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

           m_db.setHostName("localhost");      //连接数据库主机名，这里需要注意（若填的为”10.129.41.30“，出现不能连接，则改为localhost)


}

opeDB::~opeDB()
{
    m_db.close();
}
