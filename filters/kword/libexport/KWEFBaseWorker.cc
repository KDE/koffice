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

#include <qdom.h>

#include <kdebug.h>

#include "KWEFBaseWorker.h"

//
// At first, define all methods that do something real!
//

bool KWEFBaseWorker::doAbortFile(void)
{
    // Mostly, aborting is the same than closing the file!
    return doCloseFile();
}

//
// Secondly, define all methods returning false
//

#define DO_FALSE_DEFINITION(string) \
    bool KWEFBaseWorker::##string \
    {\
        kdWarning() << "KWEFBaseWorker::" << #string << " was called (Worker not correctly defined?)" << endl; \
        return false;\
    }

DO_FALSE_DEFINITION(doOpenFile(const QString& , const QString& ))
DO_FALSE_DEFINITION(doCloseFile(void))
DO_FALSE_DEFINITION(doOpenDocument(void))
DO_FALSE_DEFINITION(doCloseDocument(void))
DO_FALSE_DEFINITION(doFullParagraph(QString&, LayoutData&, ValueListFormatData&))

//
// Thirdly, define all methods returning false
//

#define DO_TRUE_DEFINITION(string) \
    bool KWEFBaseWorker::##string \
    {\
        return true;\
    }

DO_TRUE_DEFINITION(doOpenTextFrameSet(void))
DO_TRUE_DEFINITION(doCloseTextFrameSet(void))
DO_TRUE_DEFINITION(doFullDocumentInfo(QDomDocument& info))
DO_TRUE_DEFINITION(doFullPaperFormat(const int format, const double width, const double height, const int orientation))
DO_TRUE_DEFINITION(doOpenHead(void))
DO_TRUE_DEFINITION(doCloseHead(void))
DO_TRUE_DEFINITION(doOpenBody(void))
DO_TRUE_DEFINITION(doCloseBody(void))
