/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef CSVFILTER_H
#define CSVFILTER_H

#include <qstring.h>
#include <qtextstream.h>
#include <qfile.h>
#include <qobject.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <koFilter.h>
#include <koTarStore.h>

#include <xmltree.h>

class CSVFilter : public KoFilter {

    Q_OBJECT

public:
    CSVFilter(KoFilter *parent, QString name);
    virtual ~CSVFilter() {}
    /** filtering :) */
    virtual const bool I_filter(const QCString &file, const QCString &from,
				QDomDocument &document, const QCString &to,
				const QString &config=QString::null);
};
#endif // CSVFILTER_H
