/* This file is part of the KDE project
   Copyright (C) 2001 Eva Brucherseifer <eva@kde.org>
   Copyright (C) 2005 Bram Schoenmakers <bramschoenmakers@kde.nl>
   based on kspread csv export filter by David Faure

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

#ifndef HTMLEXPORT_TEST_H
#define HTMLEXPORT_TEST_H

#include <KoFilter.h>
#include <QByteArray>
#include <QVariantList>

class ExportDialog;
class KoDocument;

class KCSheet;

class HTMLExport : public KoFilter
{
    Q_OBJECT
public:
    HTMLExport(QObject* parent, const QVariantList&);
    virtual ~HTMLExport();

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);
private:
    /** Writes the top of the page in HTML to @par str */
    void openPage(KCSheet *sheet, KoDocument *document, QString &str);

    /** Closes a page in HTML */
    void closePage(QString &);

    /**
      Converts @par sheet to HTML and writes to @par str.
     */
    void convertSheet(KCSheet *sheet, QString &str, int, int);

    /** Writes a bar and a link to the top to @par str. */
    void createSheetSeparator(QString &);

    /** Writes the table of contents */
    void writeTOC(const QStringList &, const QString &, QString &);

    /**
      Returns a filename based on the @par base filename and the options
      defined in the dialog.
    */
    QString fileName(const QString &base, const QString &, bool);

    /**
      Detects which rows and columns of the given @par sheet are used and
      writes the number of them to @par row and @par column.
     */
    void detectFilledCells(KCSheet *sheet, int &rows, int &columns);
private:
    ExportDialog *m_dialog;

    typedef QMap<QString, int> Rows;
    Rows m_rowmap;
    typedef QMap<QString, int> Columns;
    Columns m_columnmap;
};

#endif

