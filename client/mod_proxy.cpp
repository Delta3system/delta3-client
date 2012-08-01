#include <QRegExp>
#include <QDebug>
#include <QTcpSocket>
#include "mod_proxy.h"

namespace delta3
{
Mod_Proxy::Mod_Proxy(qint16 adminId, Client *client) :
    ModAbstract(MOD_PROXY, adminId, client)
{
    _socket = new QTcpSocket;

    connect(_socket, SIGNAL(readyRead()), SLOT(protocolMessage()));
    connect(_socket, SIGNAL(connected()), SLOT(slotConnected()));
}

void Mod_Proxy::incomeMessage(const QByteArray &data)
{
    _data = data;
    QString _host = getHost(_data);
    if (_socket->state()!=QTcpSocket::UnconnectedState)
        _socket->disconnectFromHost();

    _socket->connectToHost(_host, 80);

    qDebug() << Q_FUNC_INFO;
    qDebug() << "Host: " << _host;
    qDebug() << "Request: " << _data;
}

void Mod_Proxy::close()
{
    _socket->close();
}

void Mod_Proxy::protocolMessage()
{
    //QByteArray _data = QVariant(QString(_socket->readAll() + "\n")).toByteArray();
    QByteArray _data = _socket->readAll();
    qDebug() << Q_FUNC_INFO << "\n" << _data;

    emit messageReadyRead(MOD_PROXY, _adminId, _data);

    //_socket->disconnectFromHost();
}

void Mod_Proxy::slotConnected()
{
    _socket->write(_data);

    qDebug() << Q_FUNC_INFO;
}

QString Mod_Proxy::getHost(QByteArray _data)
{
    QString _host,
            _str = _data.data();
    QRegExp rx("host: ([a-zA-Z0-9\\.\\-]+)");

    if (rx.indexIn(_str.toLower()) != 1)
        _host = rx.cap(1);

    return _host;
}


}
