/* This file is part of the KDE project
* Copyright (C) 2007 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
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

#include "TextInsertTextCommand.h"

#include <QTextBlock>

#include "kdebug.h"
#include <klocalizedstring.h>


TextInsertTextCommand::TextInsertTextCommand( TextTool *tool, QString text, QUndoCommand *parent )
: QUndoCommand( i18n("Insert Text"), parent ),
    m_text(text),
    m_id(TextTool::InsertText),
    m_tool(tool)
{
    m_tool->m_currentCommand = this;
    m_tool->m_currentCommandHasChildren = true;
    m_tool->m_caret.insertText( m_text );
    m_tool->m_currentCommand = 0;
    m_tool->m_currentCommandHasChildren = false;

    //Save tool text counter, position and char format for comparison in MergeWith()
    m_position = m_tool->m_caret.position();
    m_charFormat = m_tool->m_caret.charFormat();
}


TextInsertTextCommand::~TextInsertTextCommand()
{
}

/// redo the command
void TextInsertTextCommand::redo()
{
    m_tool->flagUndoRedo( false );
    QUndoCommand::redo();
    m_tool->flagUndoRedo( true );
}

/// revert the actions done in redo
void TextInsertTextCommand::undo()
{
    m_tool->flagUndoRedo( false );
    QUndoCommand::undo();
    m_tool->flagUndoRedo( true );
}

int TextInsertTextCommand::id() const
{
    return m_id;
}

bool TextInsertTextCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != m_id){ 	 // make sure other is also an InsertText command
    return false;
    }
    
    if ((m_position == (static_cast<const TextInsertTextCommand*>(other)->m_position - static_cast<const TextInsertTextCommand*>(other)->m_text.length())) &&
        (static_cast<const TextInsertTextCommand*>(other)->m_charFormat == m_charFormat)) {
        m_text += static_cast<const TextInsertTextCommand*>(other)->m_text;
        m_position += static_cast<const TextInsertTextCommand*>(other)->m_text.length();
	for (int i = 1; i <= other->childCount(); i++)
	  new TextTool::UndoTextCommand( m_tool->m_textShapeData->document(), m_tool, this);
        return true;
    }
    return false;
}
