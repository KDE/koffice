// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2002 Laurent Montel <lmontel@mandrakesoft.com>

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

#include "kpresenter_doc.h"
#include "kprstylemanager.h"
#include "kprstylemanager.moc"
#include <koUnit.h>
#include <kdebug.h>
#include <koStylist.h>
#include <kostyle.h>

KPrStyleManager::KPrStyleManager( QWidget *_parent, KoUnit::Unit unit,KPresenterDoc *_doc,
                                  const QPtrList<KoStyle> & style, const QString & activeStyleName)
    : KoStyleManager(_parent,unit,style, activeStyleName)
{
    m_doc = _doc;
}

KoStyle* KPrStyleManager::addStyleTemplate(KoStyle *style)
{
    m_doc->setModified( true );
    return m_doc->styleCollection()->addStyleTemplate(style);
}

void KPrStyleManager::applyStyleChange( KoStyleChangeDefMap changed )
{
    m_doc->applyStyleChange( changed );
}

void KPrStyleManager::removeStyleTemplate( KoStyle *style )
{
    m_doc->setModified( true );
    m_doc->styleCollection()->removeStyleTemplate(style);
}

void KPrStyleManager::updateAllStyleLists()
{
    m_doc->updateAllStyleLists();
}

void KPrStyleManager::updateStyleListOrder( const QStringList & list)
{
    m_doc->updateStyleListOrder( list );
}
