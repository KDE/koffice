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

#include "KFormatChangeInformation_p.h"

KFormatChangeInformation::KFormatChangeInformation(KFormatChangeInformation::FormatChangeType formatChangeType)
{
    this->formatChangeType = formatChangeType;
}

KFormatChangeInformation::FormatChangeType KFormatChangeInformation::formatType()
{
    return formatChangeType;
}

KTextStyleChangeInformation::KTextStyleChangeInformation(KFormatChangeInformation::FormatChangeType formatChangeType):
                              KFormatChangeInformation(formatChangeType)
{
}

void KTextStyleChangeInformation::setPreviousCharFormat(QTextCharFormat &previousFormat)
{
    this->previousTextCharFormat = previousFormat;
}

QTextCharFormat& KTextStyleChangeInformation::previousCharFormat()
{
    return this->previousTextCharFormat;
}

KParagraphStyleChangeInformation::KParagraphStyleChangeInformation():
                                   KTextStyleChangeInformation(KFormatChangeInformation::eParagraphStyleChange)
{
}

void KParagraphStyleChangeInformation::setPreviousBlockFormat(QTextBlockFormat &previousFormat)
{
    this->previousTextBlockFormat = previousFormat;
}

QTextBlockFormat& KParagraphStyleChangeInformation::previousBlockFormat()
{
    return this->previousTextBlockFormat;
}

KListItemNumChangeInformation::KListItemNumChangeInformation(KListItemNumChangeInformation::ListItemNumChangeType type):
                                                               KFormatChangeInformation(KFormatChangeInformation::eListItemNumberingChange),
                                                               eSubType(type)
{
}

void KListItemNumChangeInformation::setPreviousStartNumber(int oldStartNumber)
{
    this->oldStartNumber = oldStartNumber;
}

KListItemNumChangeInformation::ListItemNumChangeType KListItemNumChangeInformation::listItemNumChangeType()
{
    return eSubType;
}

int KListItemNumChangeInformation::previousStartNumber()
{
    return oldStartNumber;
}

