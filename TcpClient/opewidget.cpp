#include "opewidget.h"

OpeWidget::OpeWidget(QWidget *parent) : QWidget(parent)
{
  m_pListW = new QListWidget(this);
  m_pListW->addItem("好友");
  m_pListW->addItem("图书");
  m_pFriend = new Friend();
  m_pbook = new Book();
  m_pSW = new QStackedWidget();
  m_pSW ->addWidget(m_pFriend);
  m_pSW->addWidget(m_pbook);
  QHBoxLayout *pMain = new QHBoxLayout();
  pMain->addWidget(m_pListW);
  pMain->addWidget(m_pSW);
  setLayout(pMain);
  // 当 m_pListW 对象（可能是一个列表窗口）的当前选择的行改变时，会触发与 m_pSW 控件关联的 setCurrentIndex() 槽函数，从而根据选择的行来设置 m_pSW 控件中显示的页面索引，实现页面之间的切换。
  connect(m_pListW,SIGNAL(currentRowChanged(int)),
          m_pSW,SLOT(setCurrentIndex(int)));
}

OpeWidget &OpeWidget::getinstance()
{
    static OpeWidget OpeW;
    //返回引用可以直接返回对象 而如果是指针需要解引用*OpeW
    return OpeW;
}

Friend *OpeWidget::getfriend()
{
    return m_pFriend;
}

Book *OpeWidget::getBook()
{
    return m_pbook;
}
