// $Header$

/*
   This file is part of the KDE project
   Copyright (C) 2001, 2002 Nicolas GOUTTE <nicog@snafu.de>

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

#ifndef EXPORTBASIC_H
#define EXPORTBASIC_H

#include <KWEFBaseWorker.h>
#include "ExportFilter.h"

class HtmlBasicWorker : public HtmlWorker
{
public:
    HtmlBasicWorker(void) { }
    virtual ~HtmlBasicWorker(void) { }
public:
protected:
    virtual QString getStartOfListOpeningTag(const CounterData::Style typeList, bool& ordered);
    virtual void openParagraph(const QString& strTag, const LayoutData& layout);
    virtual void closeParagraph(const QString& strTag, const LayoutData& layout);
    virtual void openSpan(const FormatData& format);
    virtual void closeSpan(const FormatData& format);
    virtual bool writeDocType(void);
private:
    void openFormatData(const FormatData& format);
    void closeFormatData(const FormatData& format);
    QString textFormatToCss(const TextFormatting& formatData) const;
};

#endif /* EXPORTBASIC_H */
