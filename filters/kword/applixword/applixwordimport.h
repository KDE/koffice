/* This file is part of the KDE project
   Copyright (C) 2000 Enno Bartels <ebartels@nwn.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef APPLIXWORDIMPORT_H
#define APPLIXWORDIMPORT_H

#include <QString>
#include <QVariantList>
#include <QFile>
#include <QObject>
#include <QTextStream>
#include <KoFilter.h>
#include <KoStore.h>
#include <kofficeversion.h>

class KoOdfWriteStore;
class QByteArray;

class APPLIXWORDImport : public KoFilter
{

    Q_OBJECT

public:
    APPLIXWORDImport(QObject *parent, const QVariantList&);
    virtual ~APPLIXWORDImport() {}

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);

private:
    QChar   specCharfind(QChar , QChar);
    QString readTagLine(QTextStream &);
    void    replaceSpecial(QString &);
    QString nextLine(QTextStream &);
    int     readHeader(QTextStream &stream);
    bool createMeta(KoOdfWriteStore &store);

private:
    int m_stepsize;
    int m_instep;
    int m_progress;
    QString m_nextPendingLine;

};
#endif // APPLIXWORDIMPORT_H
