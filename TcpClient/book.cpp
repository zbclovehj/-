#include "book.h"
#include "tcpclient.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include "opewidget.h"
#include "sharefile.h"
Book::Book(QWidget *parent) : QWidget(parent)
{
    //NEW 会动态地分配内存以创建一个新的对象，并返回指向该对象的指针。
    m_strEnterDir.clear();
    download = false;
    m_pTimer = new QTimer();
    m_pBookList = new QListWidget();
    m_pReturn =new QPushButton("返回");
    m_pCreateDir=new QPushButton("创建文件夹");
    m_pDeleteDir=new QPushButton("删除文件夹");
    m_pRename=new QPushButton("重命名文件");
    m_pFlush=new QPushButton("刷新文件");
    m_pUpload=new QPushButton("上传文件");
    m_pDownLoad=new QPushButton("下载文件");
    m_pDelFile=new QPushButton("删除文件");
    m_pShareFile=new QPushButton("分享文件");
    m_pMoveFilePB = new QPushButton("移动文件");
    m_pSelectDirPB = new QPushButton("目标目录");
    m_pSelectDirPB->setEnabled(false);
    QVBoxLayout *pDirVBL = new QVBoxLayout;
    pDirVBL->addWidget(m_pReturn);
    pDirVBL->addWidget(m_pCreateDir);
    pDirVBL->addWidget(m_pDeleteDir);
    pDirVBL->addWidget(m_pRename);
    pDirVBL->addWidget(m_pFlush);
    QVBoxLayout *pFileVBL = new QVBoxLayout();
    pFileVBL->addWidget(m_pUpload);
    pFileVBL->addWidget(m_pDownLoad);
    pFileVBL->addWidget(m_pDelFile);
    pFileVBL->addWidget(m_pShareFile);
    pFileVBL->addWidget(m_pMoveFilePB);
    pFileVBL->addWidget(m_pSelectDirPB);
    QHBoxLayout *pMain = new QHBoxLayout;
    pMain->addWidget(m_pBookList);
    pMain->addLayout(pDirVBL);
    pMain->addLayout(pFileVBL);
    setLayout(pMain);
    connect(m_pCreateDir,SIGNAL(clicked(bool)),
            this,SLOT(createDir()));
    connect(m_pFlush,SIGNAL(clicked(bool)),
            this,SLOT(flushFile()));
    connect(m_pDeleteDir,SIGNAL(clicked(bool)),
            this,SLOT(deleteDir()));
    connect(m_pRename,SIGNAL(clicked(bool)),
            this,SLOT(newNameDir()));
    //当双击对象时会发出doubleClicked信号，并传出QModelIndex给enterDir
    /*

指定了m_pBookList对象要发射的信号。
doubleClicked是一个信号，当用户在列表视图中双击某个项时，
这个信号会被发射。QModelIndex是一个参数，表示被双击的项的模型索引。
enterDir是槽函数的名称，它接受一个QModelIndex类型的参数，
这个参数对应于被双击的项的模型索引。
     */
    connect(m_pBookList,SIGNAL(doubleClicked(QModelIndex)),
            this,SLOT(enterDir(QModelIndex)));
    connect(m_pReturn,SIGNAL(clicked(bool)),
            this,SLOT(returnPreDir()));
    connect(m_pUpload,SIGNAL(clicked(bool)),
            this,SLOT(uploadFile()));
    connect(m_pDownLoad,SIGNAL(clicked(bool)),
            this,SLOT(downloadFile()));
    connect(m_pTimer,SIGNAL(timeout()),
            this,SLOT(uploadFileData()));

    connect(m_pDelFile,SIGNAL(clicked(bool)),
            this,SLOT(deleteFile()));

    connect(m_pShareFile,SIGNAL(clicked(bool)),
            this,SLOT(shareFile()));

    connect(m_pMoveFilePB,SIGNAL(clicked(bool)),
            this,SLOT(moveFile()));
    connect(m_pSelectDirPB,SIGNAL(clicked(bool)),
            this,SLOT(selectDstDir()));
}

void Book::updateFileList(const PDU *pdu)
{
    if(NULL == pdu){
        return;
    }
    m_pBookList->clear();
    FileInfo *pFileInfo = NULL;
    int iLen = pdu->uiMsgLen/sizeof(FileInfo);
    for(int i = 0; i < iLen; ++i){
        pFileInfo = (FileInfo*)(pdu->iMsg) + i;
        if(QString("%1").arg(pFileInfo->caName)==".."||
                QString("%1").arg(pFileInfo->caName)==".")
        {
            continue;
        }

        QListWidgetItem *pItem = new QListWidgetItem();
        if(pFileInfo->iFileType==0){
            pItem->setIcon(QIcon(QPixmap(":/dir.png")));
        }
        else if(pFileInfo->iFileType==1){
            pItem->setIcon(QIcon(QPixmap(":/file.jpg")));
        }
        pItem->setText(pFileInfo->caName);
        m_pBookList->addItem(pItem);
    }

}

void Book::clearEnterDir()
{
    m_strEnterDir.clear();

}

void Book::setDwonloadStatus(bool status)
{
    download=status;
}

bool Book::gettDwonloadStatus()
{
    return download;
}

void Book::setTotalSize(qint64 s)
{
    m_iTotal = s;
}

void Book::setRecvedSize(qint64 s)
{
    m_iRecved = s;
}

qint64 Book::getTotalSize()
{
    return m_iTotal;
}

qint64 Book::getRecvedSize()
{
    return m_iRecved;
}

QString Book::getEnterNameDir()
{
    return m_strEnterDir;

}

QString Book::getDwonloadFilePath()
{
    return m_strDwonloadFilePath;
}

QString Book::getShareFileName()
{
    return m_strShareFileName;
}

void Book::createDir()
{
    QString strNewDir = QInputDialog::getText(this,"新建文件夹","输入文件夹名字");
    if(!strNewDir.isEmpty()){
        if(strNewDir.size()<=32){
            QString strName = TcpClient::getinstance().getUserName();
            QString curPath = TcpClient::getinstance().getCurPath();
            PDU* pdu = mkPDU(curPath.size()+1);
            pdu->uiMsgType = ENUM_MSG_TYPE_CRATE_DIR_REQUEST;
            memcpy(pdu->caFileData,strName.toStdString().c_str(),strName.size());
            memcpy(pdu->caFileData + 32,strNewDir.toStdString().c_str(),strNewDir.size());
            memcpy((char*)pdu->iMsg,curPath.toStdString().c_str(),curPath.size());
            TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
            free(pdu);
            pdu = NULL;
        } else {
            QMessageBox::warning(this,"新建文件夹","文件夹名字不能过长");

        }
    } else {
        QMessageBox::information(this,"新建文件夹","文件夹名字不能为空");
    }
}

void Book::flushFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    PDU* pdu = mkPDU(strCurPath.size()+32);
    pdu->uiMsgType = ENUM_MSG_TYPE_FLUSH_FILE_REQUEST;
    strncpy((char*)pdu->iMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu= NULL;
}

void Book::deleteDir()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"删除文件夹","请选择要删除的文件夹");
        return;
    }
    QString deletedFileName = pItem->text();
    PDU* pdu = mkPDU(strCurPath.size()+32);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_DIR_REQUEST;
    memcpy((char*)pdu->iMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    memcpy((char*)pdu->caFileData,deletedFileName.toStdString().c_str(),deletedFileName.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::newNameDir()
{

    QListWidgetItem* pItem = m_pBookList->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"重命名文件","请选择要重命名的文件");
        return;
    }
    QString strOldNameDir = pItem->text();
    QString strReNameDir = QInputDialog::getText(this,"重命名文件","输入文件名字，以及文件后缀");
    if(strReNameDir.isEmpty()){
        QMessageBox::warning(this,"重命名文件","重命名不能为空");
        return;
    }
    QString strCurPath = TcpClient::getinstance().getCurPath();
    PDU* pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_NEW_NAME_RESQUEST;
    memcpy(pdu->caFileData+32,strReNameDir.toStdString().c_str(),strReNameDir.size());
    memcpy(pdu->caFileData,strOldNameDir.toStdString().c_str(),strOldNameDir.size());
    memcpy((char*)pdu->iMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;

}

void Book::enterDir(const QModelIndex &index)
{
    //QVariant是Qt中的一个通用容器类，它可以存储各种类型的数据。
    QString strDirName = index.data().toString();
    if(strDirName==".."&&TcpClient::getinstance().getCurPath()==TcpClient::getinstance().getRootPath())
    {
        QMessageBox::warning(this,"进入","已到根目录，不能查看上一级目录");
        return;
    }
    m_strEnterDir = strDirName;
    QString strCurPath = TcpClient::getinstance().getCurPath();
    PDU* pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_ENTER_DIR_RESQUEST;
    strcpy((char*)pdu->iMsg,strCurPath.toStdString().c_str());
    strcpy(pdu->caFileData,strDirName.toStdString().c_str());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::returnPreDir()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    if(strCurPath==TcpClient::getinstance().getRootPath()){
        QMessageBox::warning(this,"返回","已到根目录，不能查看上一级目录");
        return;
    }
    int index = strCurPath.lastIndexOf('/');
    strCurPath.remove(index,strCurPath.size()-index);
    TcpClient::getinstance().setCurPath(strCurPath);
    clearEnterDir();
    flushFile();

}

void Book::uploadFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    m_strUploadFilePath = QFileDialog::getOpenFileName();
    if(m_strUploadFilePath.isEmpty()){
        QMessageBox::warning(this,"上传文件","不能选择空文件");
        return;
    }
    int index = m_strUploadFilePath.lastIndexOf('/');
    QString fileName = m_strUploadFilePath.right(m_strUploadFilePath.size()-index-1);
    //获取自己电脑的文件信息用于上传到服务器上面
    QFile file(m_strUploadFilePath);
    qint64 fileSize = file.size();//获得文件大小
    PDU* pdu = mkPDU(strCurPath.size()+1);
    pdu->uiMsgType = ENUM_MSG_TYPE_UPLOAD_FILE_REQUEST;
    memcpy((char*)pdu->iMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    sprintf(pdu->caFileData,"%s %lld",fileName.toStdString().c_str(),fileSize);
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
    //1000毫秒结束触发定时器
    m_pTimer->start(1000);
}

void Book::uploadFileData()
{
    //结束定时
    m_pTimer->stop();
    QFile file(m_strUploadFilePath);
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"上传文件","上传失败");
        return;
    }
    char *pBuffer = new char[4096];
    qint64 ret = 0;
    while(1){
        ret = file.read(pBuffer,4096);
        if(ret>0&&ret<=4096){
            TcpClient::getinstance().getTcpSocket().write(pBuffer,ret);
        } else if(0==ret){
            break;
        }
        else {
            QMessageBox::warning(this,"上传文件","上传文件失败");
            break;
        }
    }
    file.close();
    delete []pBuffer;
    pBuffer = NULL;

}

void Book::deleteFile()
{
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"删除文件","请选择要删除的文件");
        return;
    }
    QString deletedFileName = pItem->text();
    PDU* pdu = mkPDU(strCurPath.size()+32);
    pdu->uiMsgType = ENUM_MSG_TYPE_DELETE_FILE_REQUEST;
    memcpy((char*)pdu->iMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    memcpy((char*)pdu->caFileData,deletedFileName.toStdString().c_str(),deletedFileName.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
}

void Book::downloadFile()
{

    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"下载文件","请选择要下载的文件");
        return;
    }
    m_strDwonloadFilePath = QFileDialog::getSaveFileName();
    if(m_strDwonloadFilePath.isEmpty()){
        QMessageBox::warning(this,"下载文件","请选择要保存的位置");
        return;
    }
    QString strCurPath = TcpClient::getinstance().getCurPath();
    QString downloadFileName = pItem->text();
    PDU* pdu = mkPDU(strCurPath.size()+32);
    pdu->uiMsgType = ENUM_MSG_TYPE_DOWNLOAD_FILE_REQUEST;
    memcpy((char*)pdu->iMsg,strCurPath.toStdString().c_str(),strCurPath.size());
    memcpy((char*)pdu->caFileData,downloadFileName.toStdString().c_str(),downloadFileName.size());
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;


}

void Book::shareFile()
{
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"分享文件","请选择要分享的文件");
        return;
    }
    m_strShareFileName = pItem->text();
    Friend* pFriend = OpeWidget::getinstance().getfriend();
    QListWidget* pFriendList = pFriend->getFriendList();
    ShareFile::getinstance().updateFriend(pFriendList);
    if(ShareFile::getinstance().isHidden()){
        ShareFile::getinstance().show();
    }
}

void Book::moveFile()
{
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"移动文件","请选择要移动的文件");
        return;
    }
    m_pSelectDirPB->setEnabled(true);
    m_strMoveFileName = pItem->text();
    QString strCurPath = TcpClient::getinstance().getCurPath();
    m_strMoveFilePath = strCurPath +'/'+ m_strMoveFileName;


}

void Book::selectDstDir()
{
    QListWidgetItem *pItem = m_pBookList->currentItem();
    if(pItem == NULL){
        QMessageBox::information(this,"移动文件","请选择要移动的目标文件夹");
        return;
    }
    QString strDestDir = pItem->text();
    QString strCurPath = TcpClient::getinstance().getCurPath();
    m_strDstPath = strCurPath +'/'+ strDestDir;
    int srcLen = m_strMoveFilePath.size();
    int destLen = m_strDstPath.size();
    PDU* pdu = mkPDU(srcLen+destLen+2);
    pdu->uiMsgType = ENUM_MSG_TYPE_MOVE_FILE_REQUEST;
    sprintf(pdu->caFileData,"%d %d %s",srcLen,destLen,m_strMoveFileName.toStdString().c_str());
    memcpy((char*)(pdu->iMsg),m_strMoveFilePath.toStdString().c_str(),srcLen);
    memcpy((char*)(pdu->iMsg)+srcLen+1,m_strDstPath.toStdString().c_str(),destLen);
    TcpClient::getinstance().getTcpSocket().write((char*)pdu,pdu->uiPDULen);
    free(pdu);
    pdu=NULL;
    m_pSelectDirPB->setEnabled(false);

}
