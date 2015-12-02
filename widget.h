#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTcpSocket>

#ifdef _WIN32
#define __PACKED__
#pragma pack(push,1)
#elif __APPLE__
#define __PACKED__ __attribute__((packed))
#endif

namespace Ui {
class Widget;
}


typedef struct __PACKED__ {
    unsigned short msgid;
    unsigned short msgtype;
    int msgtime;
    unsigned short linkid;
    unsigned int userdata;
    unsigned short len;
} MSGHEAD;

typedef struct __PACKED__ {
    MSGHEAD head;
    char data[8186];
} MSGBUF;

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

    void recv();
    bool readHead();
    bool readData();

private:
    Ui::Widget *ui;
    QTcpSocket *client;
    QByteArray tmpData;
    QTextCodec *codec;

    MSGHEAD currentHead;
    bool nextHead;

    bool continued;
    void reverse(void *data, int len) {
        register unsigned char *pb = (unsigned char *)data;
        register unsigned char *pe = pb + len - 1;
        register unsigned char b;
        while( pb < pe ){
            b = *pb;
            *pb = *pe;
            *pe = b;
            pb++; pe--;
        }
    }

    #define REVERSE(a) reverse(&(a), sizeof(a));
};

#endif // WIDGET_H
