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


#include "kwbgspellcheck.h"
#include <koBgSpellCheck.h>
#include <kdebug.h>
#include <kotextobject.h>
#include <klocale.h>
#include "kwdoc.h"
#include "kwtextframeset.h"

KWBgSpellCheck::KWBgSpellCheck(KWDocument *_doc)
    : KoBgSpellCheck()
{
    m_doc=_doc;
    m_currentFrame=0L;
    objectForSpell(m_currentFrame);
}

KWBgSpellCheck::~KWBgSpellCheck()
{
}

void KWBgSpellCheck::objectForSpell(KWTextFrameSet *curr)
{
    m_currentFrame=curr;
    if( m_currentFrame && m_currentFrame->textObject())
        m_bgSpell.currentTextObj=m_currentFrame->textObject();
    else
        m_bgSpell.currentTextObj=0L;
}

void KWBgSpellCheck::slotRepaintChanged(KoTextObject *obj)
{
    if( m_currentFrame->textObject()==obj)
        m_doc->slotRepaintChanged(m_currentFrame);
}

KoTextObject *KWBgSpellCheck::nextTextObject( KoTextObject *  /*obj*/ )
{
    m_currentFrame=m_doc->nextTextFrameSet(m_currentFrame);
    if(!m_currentFrame)
        return 0L;
    else
        return m_currentFrame->textObject();
}

void KWBgSpellCheck::configurateSpellChecker()
{
    m_doc->configureSpellChecker();
}

