/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000 Michael Johnson <mikej@xnet.com>

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

#ifndef ASCIIIMPORT_H
#define ASCIIIMPORT_H

#include <qstring.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qobject.h>

#include <koFilter.h>
#include <koStore.h>

#define MAXLINES  1000
#define MAXCOLUMNS 15

 const double ptsperline = 15.0;
 const double ptsperchar = 6.0;
 const int spacespertab = 6;
 const double leftmargin = 56.0;

 const int shortline = 40;  // max length of a "short" line

 struct Tabs
    {
    int columns;
    int indent;
    QString field[MAXCOLUMNS];
    int width[MAXCOLUMNS];
    };

 struct Position
    {
    double top;
    double left;
    double bottom;
    double right;
    };

class ASCIIImport : public KoFilter {

    Q_OBJECT

public:
    ASCIIImport(KoFilter *parent, const char *name);
    ASCIIImport();
    virtual ~ASCIIImport() {}
    /** filtering :) */
    virtual bool filter(const QString &fileIn, const QString &fileOut,
                        const QString &from, const QString &to,
                        const QString &config=QString::null);
private: // not yet changed
    void WriteOutParagraph( QString name, QString type, QString text,
       int firstindent, int secondindent, QString &str);



    void WriteOutTableCell( int table_no, int row, int col, Position *pos,
                          QString &str);

    bool Table( QString *Line, int *linecount, int no_lines,
            int table_no, QString &tbl, QString &str );



    bool ListItem( QString *Line, int no_lines,
             QString &str );

    bool IsListItem( QString FirstLine, QChar mark );

private:
    void WriteOutIndents(const int firstindent,const int secondindent, QString &str);
    QString ASCIIImport::escapeXmlText(const QString& strIn) const;
    int Indent(const QString& line) const;
    int MultSpaces(const QString& text, const int index) const;
    bool IsWhiteSpace(const QChar& c) const;
    QString readLine(QTextStream& textstream, bool& lastCharWasCr);
};
#endif // ASCIIIMPORT_H
