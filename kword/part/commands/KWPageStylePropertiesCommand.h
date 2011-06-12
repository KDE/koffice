/* This file is part of the KDE project
 * Copyright (C) 2007-2010 Thomas Zander <zander@kde.org>
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

#ifndef KWPAGESTYLEPROPERTIESCOMMAND_H
#define KWPAGESTYLEPROPERTIESCOMMAND_H

#include "../kword_export.h"
#include "../KWPage.h"

#include <KOdfPageLayoutData.h>
#include <KoText.h>

#include <QUndoCommand>

class KWDocument;
class KWPage;

/**
 * The undo / redo command for changing the properties of a KWPageStyle
 * When altering the size of a page this command will also reposition all required
 * frames to account for the changes.
 */
class KWORD_TEST_EXPORT KWPageStylePropertiesCommand : public QUndoCommand
{
public:
    explicit KWPageStylePropertiesCommand(KWDocument *document, const KWPageStyle &styleBefore, const KWPageStyle &styleAfter, QUndoCommand *parent = 0);

    /// redo the command
    virtual void redo();
    /// revert the actions done in redo
    virtual void undo();

private:
    KWDocument *m_document;
    KWPageStyle m_style; // the user style we change
    KWPageStyle m_styleBefore; // one detached set of properties
    KWPageStyle m_styleAfter; // another detached set of properties
};

#endif
