/* This file is part of the KDE project
   Copyright (C)  2002 Montel Laurent <lmontel@mandrakesoft.com>

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

#include "kwdoc.h"
#include "configfootnotedia.h"
#include "configfootnotedia.moc"

#include <klocale.h>

#include <qvbox.h>
#include <koParagDia.h>
#include <kwvariable.h>
#include "kwcommand.h"

KWConfigFootNoteDia::KWConfigFootNoteDia( QWidget *parent, const char *name, KWDocument *_doc )
    : KDialogBase(Tabbed, QString::null, Ok | Cancel , Ok, parent, name, true)
{
    setCaption( i18n("Configure Endnote/Footnote") );
    m_doc = _doc;

    setupTab1();
    setupTab2();
}

void KWConfigFootNoteDia::setupTab1()
{
    QVBox * page = addVBoxPage( i18n( "Footnotes" ) );
    m_footNoteConfig = new KoCounterStyleWidget( false, true, page );
    //m_footNoteConfig->numTypeChanged( KoParagCounter::NUM_LIST );
    m_footNoteConfig->setCounter (static_cast<KWVariableSettings*>(m_doc->getVariableCollection()->variableSetting())->footNoteCounter() );
}

void KWConfigFootNoteDia::setupTab2()
{
    QVBox * page = addVBoxPage( i18n( "Endnotes" ) );
    m_endNoteConfig = new KoCounterStyleWidget( false, true/*onlyStyleLetter*/, page );
    //m_endNoteConfig->numTypeChanged( KoParagCounter::NUM_LIST );
    m_endNoteConfig->setCounter (static_cast<KWVariableSettings*>(m_doc->getVariableCollection()->variableSetting())->endNoteCounter() );
}


void KWConfigFootNoteDia::slotOk()
{
    KMacroCommand * macro = new KMacroCommand(i18n("Change End-/Footnote Variable Settings"));
    bool createMacro = false;
    KWChangeFootEndNoteSettingsCommand *cmd = 0L;
    KoParagCounter counter =static_cast<KWVariableSettings*>(m_doc->getVariableCollection()->variableSetting())->footNoteCounter();
    if (counter != m_footNoteConfig->counter() )
    {
        cmd= new KWChangeFootEndNoteSettingsCommand( i18n("Change End-/FootNote Variable Settings") , counter, m_footNoteConfig->counter() ,true ,m_doc);
        macro->addCommand(cmd );
        createMacro = true;
    }
    counter = static_cast<KWVariableSettings*>(m_doc->getVariableCollection()->variableSetting())->endNoteCounter();
    if (counter != m_endNoteConfig->counter() )
    {
        cmd= new KWChangeFootEndNoteSettingsCommand( i18n("Change End-/Footnote Variable Settings") , counter, m_endNoteConfig->counter() ,false ,m_doc);
        macro->addCommand(cmd );
        createMacro = true ;
    }
    if ( createMacro )
    {
        macro->execute();
        m_doc->addCommand( macro );
    }
    else
        delete macro;
    KDialogBase::slotOk();
}
