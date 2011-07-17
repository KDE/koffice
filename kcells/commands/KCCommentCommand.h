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

#ifndef KCELLS_COMMENT_COMMAND
#define KCELLS_COMMENT_COMMAND

#include "KCAbstractRegionCommand.h"


/**
 * \class KCCommentCommand
 * \ingroup Commands
 * \brief Adds/Removes comments to/of a cell region.
 */
class KCCommentCommand : public KCAbstractRegionCommand
{
public:
    KCCommentCommand(QUndoCommand* parent = 0);
    void setComment(const QString& comment);

protected:
    virtual bool process(Element* element);
    virtual bool mainProcessing();

private:
    QString m_comment;
    QList< QPair<QRectF, QString> > m_undoData;
};

#endif // KCELLS_COMMENT_COMMAND
