/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <david@mandrakesoft.com>
                 2002 Laurent Montel <lmontel@mandrakesoft.com>

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


#include "kprbgspellcheck.h"
#include <koBgSpellCheck.h>
#include <kdebug.h>
#include <kotextobject.h>
#include <klocale.h>
#include "kpresenter_doc.h"
#include "kptextobject.h"

KPrBgSpellCheck::KPrBgSpellCheck(KPresenterDoc *_doc)
    : KoBgSpellCheck()
{
    m_doc=_doc;
    m_currentObj=0L;
    objectForSpell(m_currentObj);
}

KPrBgSpellCheck::~KPrBgSpellCheck()
{
}

void KPrBgSpellCheck::objectForSpell(KPTextObject *curr)
{
    m_currentObj=curr;
    if( m_currentObj && m_currentObj->textObject())
        objectForSpellChecking(m_currentObj->textObject());
    else
        objectForSpellChecking(0L);
}

void KPrBgSpellCheck::slotRepaintChanged(KoTextObject *obj)
{
    if( m_currentObj->textObject()==obj)
        m_doc->repaint(m_currentObj);
}

void KPrBgSpellCheck::objectForSpellChecking(KoTextObject *obj)
{
    currObj=obj;
    m_bgSpell.currentTextObj=obj;
}

void KPrBgSpellCheck::nextTextFrameSet( KoTextObject *obj )
{
    if( m_currentObj->textObject()==obj)
    {
        m_currentObj=m_doc->nextTextFrameSet(m_currentObj);
        if(!m_currentObj)
            objectForSpellChecking(0L);
        else
            objectForSpellChecking(m_currentObj->textObject());
    }
    else
    {
        m_currentObj=0L;
        objectForSpellChecking(0L);
    }
}
