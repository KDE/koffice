/*
 * Copyright (C) 2007 Igor Stepin <igor_for_os@stepin.name>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KCOLLABORATE_NETWORKCLIENT_H
#define KCOLLABORATE_NETWORKCLIENT_H

#include <QTcpSocket>
#include <libcollaboration/network/messages/Generic.h>

namespace kcollaborate
{

class KCOLLABORATE_EXPORT NetworkClient : public QTcpSocket
{
        Q_OBJECT
    public:
        NetworkClient( QObject *parent = 0 );
        virtual ~NetworkClient();

        void sendMsg( const Message::Generic &msg );
    signals:
        void messageReceivedRaw( const QString &msg );
    private slots:
        void readyRead();
        void error( QAbstractSocket::SocketError socketError );
};

};

#endif
