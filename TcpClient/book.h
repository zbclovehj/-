#ifndef BOOK_H
#define BOOK_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "protocol.h"
#include <QTimer>
class Book : public QWidget
{
    Q_OBJECT
public:
    explicit Book(QWidget *parent = nullptr);
    void updateFileList(const PDU *pdu);
    void clearEnterDir();
    void setDwonloadStatus(bool status);
    bool gettDwonloadStatus();
    void setTotalSize(qint64 s);
    void setRecvedSize(qint64 s);
    qint64 getTotalSize();
    qint64 getRecvedSize();
    QString getEnterNameDir();
    QString getDwonloadFilePath();
    QString getShareFileName();
public slots:
    void createDir();
    void flushFile();
    void deleteDir();
    void newNameDir();
    void enterDir(const QModelIndex &index);
    void returnPreDir();
    void uploadFile();
    void uploadFileData();
    void deleteFile();
    void downloadFile();
    void shareFile();
    void moveFile();
    void selectDstDir();
signals:
private:
    QListWidget *m_pBookList;
    QPushButton *m_pReturn;
    QPushButton *m_pCreateDir;
    QPushButton *m_pDeleteDir;
    QPushButton *m_pRename;
    QPushButton *m_pFlush;
    QPushButton *m_pUpload;
    QPushButton *m_pDownLoad;
    QPushButton *m_pDelFile;
    QPushButton *m_pShareFile;
    QPushButton *m_pMoveFilePB;
    QPushButton *m_pSelectDirPB;
    QString m_strEnterDir;
    QString m_strUploadFilePath;
    QString m_strDwonloadFilePath;
    QString m_strShareFileName;
    QString m_strMoveFileName;
    QString m_strMoveFilePath;
    QString m_strDstPath;
    QTimer* m_pTimer;
    bool download;
    qint64 m_iTotal = 0;
    qint64 m_iRecved = 0;

};

#endif // BOOK_H
