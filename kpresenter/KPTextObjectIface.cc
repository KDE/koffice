/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "KPTextObjectIface.h"
#include "kptextobject.h"
#include <KoTextViewIface.h>
#include "kpresenter_doc.h"
#include "kpresenter_view.h"
#include <kotextobject.h>
#include <kapplication.h>
#include <dcopclient.h>
#include "kprcanvas.h"
#include <kdebug.h>
#include <kcommand.h>

KPTextObjectIface::KPTextObjectIface( KPTextObject *_textobject )
    : KPresenterObjectIface(_textobject),KPresenterObject2DIface(_textobject)
{
   m_textobject = _textobject;
}

DCOPRef KPTextObjectIface::startEditing()
{
    KPresenterDoc *doc=m_textobject->kPresenterDocument();
    KPresenterView *view=doc->getKPresenterView();
    view->getCanvas()->createEditing( m_textobject);
    return DCOPRef( kapp->dcopClient()->appId(),
		    view->getCanvas()->currentTextObjectView()->dcopObject()->objId() );
}

bool KPTextObjectIface::hasSelection() const
{
    return m_textobject->textObject()->hasSelection();
}

QString KPTextObjectIface::selectedText() const
{
    return m_textobject->textObject()->selectedText();
}

void KPTextObjectIface::selectAll( bool select )
{
    m_textobject->textObject()->selectAll(select);
}

void KPTextObjectIface::recalcPageNum( )
{
    //FIXME
    //m_textobject->recalcPageNum(m_textobject->kPresenterDocument());

}

void KPTextObjectIface::setBoldText( bool b )
{
   KCommand *cmd=m_textobject->textObject()->setBoldCommand( b );
   delete cmd;
}

void KPTextObjectIface::setItalicText( bool b )
{
    KCommand *cmd=m_textobject->textObject()->setItalicCommand(b);
    delete cmd;
}

void KPTextObjectIface::setUnderlineText( bool b )
{
    KCommand *cmd=m_textobject->textObject()->setUnderlineCommand(b);
    delete cmd;
}

void KPTextObjectIface::setDoubleUnderlineText( bool b )
{
    KCommand *cmd=m_textobject->textObject()->setDoubleUnderlineCommand(b);
    delete cmd;
}

void KPTextObjectIface::setStrikeOutText( bool b )
{
    KCommand *cmd=m_textobject->textObject()->setStrikeOutCommand(b);
    delete cmd;
}

void KPTextObjectIface::setTextColor( const QColor &col )
{
    KCommand *cmd=m_textobject->textObject()->setTextColorCommand(col);
    delete cmd;
}

void KPTextObjectIface::setTextPointSize( int s )
{
    KCommand *cmd=m_textobject->textObject()->setPointSizeCommand( s );
    delete cmd;
}

void KPTextObjectIface::setTextSubScript( bool b )
{
    KCommand *cmd=m_textobject->textObject()->setTextSubScriptCommand( b );
    delete cmd;
}

void KPTextObjectIface::setTextSuperScript( bool b )
{
    KCommand *cmd=m_textobject->textObject()->setTextSuperScriptCommand( b );
    delete cmd;
}

void KPTextObjectIface::setTextDefaultFormat()
{
    KCommand *cmd=m_textobject->textObject()->setDefaultFormatCommand();
    delete cmd;
}

void KPTextObjectIface::setTextBackgroundColor(const QColor & col)
{
    KCommand *cmd=m_textobject->textObject()->setTextBackgroundColorCommand(col);
    delete cmd;
}

QColor KPTextObjectIface::textColor() const
{
    return m_textobject->textObject()->textColor();
}

QFont KPTextObjectIface::textFont() const
{
    return m_textobject->textObject()->textFont();
}

QString KPTextObjectIface::textFontFamily()const
{
    return m_textobject->textObject()->textFontFamily();
}

QColor KPTextObjectIface::textBackgroundColor() const
{
    return m_textobject->textObject()->textBackgroundColor();
}

void KPTextObjectIface::setTextFamilyFont(const QString &font)
{
    KCommand *cmd=m_textobject->textObject()->setFamilyCommand(font);
    delete cmd;
}

void KPTextObjectIface::changeCaseOfText( const QString & caseType)
{
    KCommand *cmd = 0L;
    if( caseType.lower() == "uppercase" )
    {
        cmd=m_textobject->textObject()->setChangeCaseOfTextCommand( KoChangeCaseDia::UpperCase );
    }
    else if( caseType.lower() =="lowercase" )
    {
        cmd=m_textobject->textObject()->setChangeCaseOfTextCommand( KoChangeCaseDia::LowerCase );
    }
    else if( caseType.lower() =="titlecase" )
    {
        cmd=m_textobject->textObject()->setChangeCaseOfTextCommand( KoChangeCaseDia::TitleCase );
    }
    else if( caseType.lower() =="togglecase" )
    {
        cmd=m_textobject->textObject()->setChangeCaseOfTextCommand( KoChangeCaseDia::ToggleCase );
    }
    else
        kdDebug()<<"Error in void KWordTextFrameSetIface::changeCaseOfText( const QString & caseType) parameter\n";
    delete cmd;
}
