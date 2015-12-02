#include "widget.h"
#include "ui_widget.h"

#include <QDebug>
#include <QDateTime>
#include <QByteArray>
#include <QTextCodec>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    continued(true),nextHead(true)
{
    ui->setupUi(this);
    codec = QTextCodec::codecForName("GBK");
    client = new QTcpSocket(this);

    connect(client, &QTcpSocket::stateChanged, [this](){
        qDebug() << client->state();
        if( client->state() == QAbstractSocket::ConnectedState ){
            MSGBUF msg;
            msg.head.len = 11;
            msg.head.linkid = 0;
            msg.head.msgid = 296;
            msg.head.msgtime = QDateTime::currentMSecsSinceEpoch();
            msg.head.msgtype = 100;
            msg.head.userdata = 0;
            strcpy(msg.data, "20151023*1*");
            qDebug() << "sent:" << client->write( (char*)(&msg), msg.head.len+sizeof(MSGHEAD));
            qDebug() << "sent: type:" << msg.head.msgtype << " len:" << msg.head.len << " data:" << msg.data;
        }
    });

    connect(client, &QTcpSocket::readyRead, [this](){
        recv();

        static int counter = 0;

        if( continued && currentHead.msgtype != 0 ){
            MSGBUF outmsg;
            outmsg.head.len = 4;
            outmsg.head.linkid = 0;
            outmsg.head.msgid = 296;
            outmsg.head.msgtime = QDateTime::currentMSecsSinceEpoch();
            outmsg.head.msgtype = 210;
            outmsg.head.userdata = 0;
            QString _data = QString::number(counter);
            _data += "*1*";
            strcpy(outmsg.data, _data.toStdString().c_str());

            qDebug() << "send:" << client->write( (char*)(&outmsg), outmsg.head.len+sizeof(MSGHEAD)) << _data;
            counter++;
        }
    });

    client->connectToHost("169.169.167.122", 2008);

}

Widget::~Widget()
{
    delete ui;
}

void Widget::recv()
{
    currentHead.msgtype = 0;
    tmpData += client->readAll();

    while(true){
        if(nextHead){
            if(!readHead()){
                break;
            }
        }
        else {
            if( !readData() ){
                break;
            }
        }

    }
}

bool Widget::readHead()
{
    if( tmpData.size() >= sizeof(MSGHEAD) ){
        QByteArray headerData = tmpData.left(sizeof(MSGHEAD));
        MSGHEAD* head = reinterpret_cast<MSGHEAD*>(headerData.data());
        memset(&currentHead,0,sizeof(MSGHEAD));
        memcpy(&currentHead,head,sizeof(MSGHEAD));
        tmpData.remove(0,sizeof(MSGHEAD));
        if( currentHead.msgtype != 8800 && currentHead.msgtype != 8889)
            qDebug() << "recv: len:" << currentHead.len << " type:" << currentHead.msgtype;
        nextHead = false;
        if( currentHead.msgtype == 8888)
            continued = false;
        return true;
    }
    nextHead = true;
    return false;
}

bool Widget::readData()
{
    if( tmpData.size() >= currentHead.len ){
        QByteArray data = tmpData.left(currentHead.len);
        MSGBUF msg;
        msg.head = currentHead;
        memcpy(msg.data,data.data(),currentHead.len);
        msg.data[currentHead.len] = 0;
        tmpData.remove(0,currentHead.len);
        nextHead = true;
//        if( currentHead.msgtype != 8800 && currentHead.msgtype != 8889)
//            qDebug() << codec->toUnicode(msg.data);
        return true;
    }
    else
        nextHead = false;
    return false;
}
