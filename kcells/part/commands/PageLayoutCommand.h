/* This file is part of the KDE project
   Copyright 2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2004 Laurent Montel <montel@kde.org>

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

#ifndef KCELLS_PAGE_LAYOUT_COMMAND
#define KCELLS_PAGE_LAYOUT_COMMAND

#include "KCPrintSettings.h"

#include <QUndoCommand>

class KCSheet;

/**
 * \ingroup Commands
 * Alters the print settings.
 */
class PageLayoutCommand : public QUndoCommand
{
public:
    explicit PageLayoutCommand(KCSheet* sheet, const KCPrintSettings& settings, QUndoCommand* parent = 0);

    virtual void redo();
    virtual void undo();

private:
    KCSheet* m_sheet;
    KCPrintSettings m_settings;
};

#endif // KCELLS_PAGE_LAYOUT_COMMAND
