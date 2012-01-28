/* This file is part of the KDE project
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef SCPAGEEFFECTSETCOMMAND_H
#define SCPAGEEFFECTSETCOMMAND_H

#include "showcase_export.h"

#include <QUndoCommand>

class KoPAPage;
class SCPageEffect;

class SHOWCASE_EXPORT SCPageEffectSetCommand : public QUndoCommand
{
public:
    SCPageEffectSetCommand(KoPAPage * page, SCPageEffect * pageEffect);
    virtual ~SCPageEffectSetCommand();

    /// redo the command
    void redo();
    /// revert the actions done in redo
    void undo();

private:
    KoPAPage * m_page;
    SCPageEffect * m_newPageEffect;
    SCPageEffect * m_oldPageEffect;
    bool m_deleteNewPageEffect;
};

#endif /* SCPAGEEFFECTSETCOMMAND_H */
