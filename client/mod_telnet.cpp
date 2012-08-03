#include "mod_telnet.h"
#include <QDebug>

namespace delta3
{
ModTelnet::ModTelnet(qint16 adminId, Client *client)
        : ModAbstract(MOD_TELNET, adminId, client)
    {
        _protocol = new QProcess(this);
    #ifdef Q_WS_X11                             // linux desktop
        //constructing command prompt
        QProcess proc;
        proc.start("whoami");
        proc.waitForFinished();

        prompt = QString::fromUtf8(proc.readAll());
        prompt.chop(1);
        prompt.append("@");

        proc.start("hostname");
        proc.waitForFinished();
        prompt.append(proc.readAll());

        prompt.chop(1);
        prompt.append(':');

        proc.start("pwd");
        proc.waitForFinished();
        prompt.append(proc.readAll());
        prompt.chop(1);

        prompt.append("$ ");
        _protocol->start("/bin/sh");
    #elif defined(Q_WS_MAC)                     // darwin
        _protocol->start("/bin/bash");
    #elif defined(Q_WS_QWS)                     // embedded linux
        _protocol->start("bash");
    #elif defined(Q_OS_MSDOS)
        _protocol->start("cmd");
    #elif defined(Q_WS_WIN)
        _protocol->start("cmd");
    #else
        qDebug() << "This OS is not supported";
    #endif
        connect(_protocol, SIGNAL(readyReadStandardOutput()),
                this, SLOT(protocolMessage()));
        emit messageReadyRead(MOD_TELNET, _adminId, _prompt.toUtf8());
}

    ModTelnet::~ModTelnet()
    {
        _protocol->disconnect(this);
        _protocol->close();
    }

    void ModTelnet::incomeMessage(const QByteArray &data)
    {
        QString cmd = QString::fromUtf8(data);
        cmd.append('\n');
        qDebug() << cmd ;
        if(_protocol->Running)
            _protocol->write(cmd.toLocal8Bit());
    }

    void ModTelnet::protocolMessage()
    {
        #if defined(Q_OS_WIN)
        QString output = QString::fromLocal8Bit(
                    _protocol->readAllStandardOutput());
        #else
        QString output = QString::fromLocal8Bit(
                    _protocol->readAllStandardOutput() + "\n" +
                    _prompt.toLocal8Bit());
        #endif


        qDebug() << "proto3 output:\n" << output.toUtf8();
        emit messageReadyRead(MOD_TELNET, _adminId, output.toUtf8());
    }
}
