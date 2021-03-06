/* This file is part of the KDE project
   Copyright (C) 2002 Ariya Hidayat <ariya@kde.org>

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

#ifndef WP5_H
#define WP5_H

#include <QtCore/QFile>

#include <KWEFStructures.h>
#include <KWEFBaseWorker.h>

class QString;
class QDataStream;

class WPFiveWorker : public KWEFBaseWorker
{
public:
    explicit WPFiveWorker(void)  { }
    virtual ~WPFiveWorker(void) { }
    virtual bool doOpenFile(const QString& filenameOut, const QString& to);
    virtual bool doCloseFile(void);
    virtual bool doOpenDocument(void);
    virtual bool doCloseDocument(void);
    virtual bool doFullParagraph(const QString& paraText, const LayoutData& layout,
                                 const ValueListFormatData& paraFormatDataList);
private:
    QString filename;
    QFile outfile;
    QDataStream output;
    quint32 document_area_ptr;
};

#endif //WP5_H
