/* This file is part of the KDE project
 * Copyright (C) 2011 Ganesh Paramasivam <ganesh@crystalfab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KFORMAT_CHANGE_INFORMATION_H
#define KFORMAT_CHANGE_INFORMATION_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the KOdfText API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//


#include <QTextCharFormat>
#include <QTextBlockFormat>

class KFormatChangeInformation
{
public:
    typedef enum {
        eTextStyleChange = 0,
        eParagraphStyleChange,
        eListItemNumberingChange
    } FormatChangeType;

    KFormatChangeInformation::FormatChangeType formatType();

protected:
    KFormatChangeInformation(KFormatChangeInformation::FormatChangeType formatChangeType);

private:
    KFormatChangeInformation::FormatChangeType formatChangeType;
};

class KTextStyleChangeInformation : public KFormatChangeInformation
{
public:
    KTextStyleChangeInformation(KFormatChangeInformation::FormatChangeType formatChangeType = KFormatChangeInformation::eTextStyleChange);
    void setPreviousCharFormat(QTextCharFormat &oldFormat);
    QTextCharFormat& previousCharFormat();
private:
    QTextCharFormat previousTextCharFormat;
};

class KParagraphStyleChangeInformation : public KTextStyleChangeInformation
{
public:
    KParagraphStyleChangeInformation();
    void setPreviousBlockFormat(QTextBlockFormat &oldFormat);
    QTextBlockFormat& previousBlockFormat();
private:
    QTextBlockFormat previousTextBlockFormat;
};

class KListItemNumChangeInformation : public KFormatChangeInformation
{
public:
    typedef enum {
        eNumberingRestarted = 0,
        eRestartRemoved
    } ListItemNumChangeType;
    KListItemNumChangeInformation(KListItemNumChangeInformation::ListItemNumChangeType eSubType);
    void setPreviousStartNumber(int oldRestartNumber);
    KListItemNumChangeInformation::ListItemNumChangeType listItemNumChangeType();
    int previousStartNumber();
private:
    int oldStartNumber;
    KListItemNumChangeInformation::ListItemNumChangeType eSubType;
};
#endif
