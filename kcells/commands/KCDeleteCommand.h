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

class KCColumnFormat;
class KCRowFormat;

/**
 * \ingroup Commands
 * \brief Deletes cell contents and column/row styles.
 */
class KCDeleteCommand : public KCAbstractDataManipulator
{
public:
    /**
     * Constructor.
     */
    KCDeleteCommand(QUndoCommand *parent = 0);

    /**
     * Destructor.
     */
    virtual ~KCDeleteCommand();

    enum Mode {
        Everything,     ///< Delete also column and row formats.
        OnlyCells       ///< Delete only cell contents, styles, etc.
    };
    void setMode(Mode mode);

protected:
    /**
     * Processes \p element , a KCRegion::Point or a KCRegion::Range .
     * Invoked by mainProcessing() .
     */
    virtual bool process(Element* element);

    virtual bool mainProcessing();

    // dummy
    virtual KCValue newValue(Element*, int, int, bool*, KCFormat::Type*) {
        return KCValue();
    }

protected:
    QSet<KCColumnFormat*> m_columnFormats;
    QSet<KCRowFormat*>   m_rowFormats;
    Mode m_mode;
};

#endif // KSPREAD_DELETE_COMMAND
