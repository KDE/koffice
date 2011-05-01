/* This file is part of the KDE project
   Copyright 2005,2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_CONDITION_COMMAND
#define KSPREAD_CONDITION_COMMAND

#include <QLinkedList>

#include "AbstractRegionCommand.h"
#include "KCCondition.h"


/**
 * \class CondtionCommand
 * \ingroup Commands
 * \brief Adds/Removes condtional formatting to/of a cell region.
 */
class CondtionCommand : public AbstractRegionCommand
{
public:
    CondtionCommand();
    void setConditionList(const QLinkedList<KCConditional>& list);

protected:
    virtual bool process(Element* element);
    virtual bool mainProcessing();

private:
    KCConditions m_conditions;
    QList< QPair<QRectF, KCConditions> > m_undoData;
};

#endif // KSPREAD_CONDITION_COMMAND
