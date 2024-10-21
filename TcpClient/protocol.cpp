#include "protocol.h"


PDU* mkPDU(uint uiMsgLen)
{
    uint uiPDULen = sizeof(PDU)+uiMsgLen;
    //malloc c语言中的函数
    PDU* pdu = (PDU*)malloc(uiPDULen);
    if(NULL == pdu){
        exit(EXIT_FAILURE); // 错误退出程序
    }
    //pdu 是一个指针，指向要处理的内存区域的起始地址。
    //  0 是要用来设置内存区域的值，这里是将内存区域填充为0。
    //  uiPDULen 是要设置的字节数，即要对pdu指向的内存区域设置前多少个字节为0。
    memset(pdu,0,uiPDULen);
    //数据初始化 *pdu 表示指针 pdu 所指向的对象，而 pdu 表示指向动态分配内存块的指针本身
    //pdu->uiPDULen 表示解引用 pdu 指针并访问其所指向的 PDU 结构体中的 uiPDULen 成员变量
    pdu->uiPDULen = uiPDULen;
    pdu->uiMsgLen = uiMsgLen;
    return pdu;
}
