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

#ifndef kprbgspellcheck_h
#define kprbgspellcheck_h

class KPresenterDoc;
class KPTextObject;

#include <koBgSpellCheck.h>

class KPrBgSpellCheck : public KoBgSpellCheck
{
public:
    KPrBgSpellCheck(KPresenterDoc *_doc);
    virtual ~KPrBgSpellCheck();

    void objectForSpell(KPTextObject *curr);
    //repaint object when we spell check
    virtual void slotRepaintChanged(KoTextObject *obj);

    KPTextObject *currentCheckSpellingFrame() const { return m_currentObj; }

    KoTextObject *nextTextObject( KoTextObject *obj );

    //spell checker is not configurate.
    virtual void configurateSpellChecker();

private:
    KPresenterDoc *m_doc;
    KPTextObject *m_currentObj;
};
#endif
