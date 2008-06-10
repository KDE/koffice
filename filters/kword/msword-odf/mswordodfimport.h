/* This file is part of the KOffice project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 David Faure <faure@kde.org>
   Copyright (C) 2008 Benjamin Cail <cricketc@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef MSWORDODFIMPORT_H
#define MSWORDODFIMPORT_H

#include <KoFilter.h>
#include <KoXmlWriter.h>
//Added by qt3to4:
#include <QByteArray>

class MSWordOdfImport : public KoFilter
{
    Q_OBJECT
public:
    MSWordOdfImport( QObject* parent, const QStringList& );
    virtual ~MSWordOdfImport();

    virtual KoFilter::ConversionStatus convert( const QByteArray& from, const QByteArray& to );

private:
    class Private;
    Private* d;
};

#endif // MSWORDODFIMPORT_H
