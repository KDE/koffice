/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KC_COLUMN_STYLE_COMMAND
#define KC_COLUMN_STYLE_COMMAND

#include "KCAbstractRegionCommand.h"

class KCColumnFormat;

/**
 * \ingroup Commands
 * \brief Sets column styles.
 */
class KCColumnStyleCommand : public KCAbstractRegionCommand
{
public:
    KCColumnStyleCommand(QUndoCommand *parent = 0);
    virtual ~KCColumnStyleCommand();

    void setWidth(double width);
    void setHidden(bool hidden);
    void setPageBreak(bool pageBreak);
    void setTemplate(const KCColumnFormat &columnFormat);

protected:
    virtual bool mainProcessing();

private:
    double m_width;
    bool m_hidden;
    bool m_pageBreak;
    QMap<int, KCColumnFormat *> m_columnFormats;
};

#endif // KC_COLUMN_STYLE_COMMAND
