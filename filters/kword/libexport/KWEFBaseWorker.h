// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001 Nicolas GOUTTE <nicog@snafu.de>

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

#ifndef KWEF_BASEWORKER_H
#define KWEF_BASEWORKER_H

#include <qstring.h>

#include <KWEFStructures.h>

class KWEFBaseWorker
{
    public:
        KWEFBaseWorker(void) {}
        virtual ~KWEFBaseWorker(void) {}
    public: // leader/worker functions
        bool doOpenFile(const QString& filenameOut, const QString& to);
        bool doCloseFile(void); // Close file in normal conditions
        bool doAbortFile(void); // Close file after errors
        bool doOpenDocument(void);
        bool doCloseDocument(void);
        bool doFullParagraph(QString& paraText, ValueListFormatData& paraFormatDataList);
};

#endif /* KWEF_BASEWORKER_H */
