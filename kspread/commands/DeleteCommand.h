/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_DELETE_COMMAND
#define KSPREAD_DELETE_COMMAND

#include "DataManipulators.h"

#include <QSet>

namespace KSpread
{
class ColumnFormat;
class RowFormat;

class DeleteCommand : public AbstractDataManipulator
{
public:
    /**
     * Constructor.
     */
    DeleteCommand();

    /**
     * Destructor.
     */
    virtual ~DeleteCommand();

protected:
    /**
     * Processes \p element , a Region::Point or a Region::Range .
     * Invoked by mainProcessing() .
     */
    virtual bool process( Element* element );

    virtual bool mainProcessing();

    // dummy
    virtual Value newValue( Element*, int, int, bool*, Format::Type* ) { return Value(); }

protected:
    QSet<ColumnFormat*> m_columnFormats;
    QSet<RowFormat*>   m_rowFormats;
};

} // namespace KSpread

#endif // KSPREAD_DELETE_COMMAND
