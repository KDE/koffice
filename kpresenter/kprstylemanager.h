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

#ifndef kprstylemanager_h
#define kprstylemanager_h

#include <kdialogbase.h>
#include <qstringlist.h>
#include <koStylist.h>

class KPresenterDoc;
class KoStyle;
class KoUnit;

class KPrStyleManager : public KoStyleManager
{
    Q_OBJECT

public:
    KPrStyleManager( QWidget *_parent, KoUnit::Unit unit,KPresenterDoc *_doc,
                     const QPtrList<KoStyle> & style, const QString & activeStyleName );

    virtual KoStyle* addStyleTemplate(KoStyle *style);
    virtual void applyStyleChange( StyleChangeDefMap changed  );
    virtual void removeStyleTemplate( KoStyle *style );
    virtual void updateAllStyleLists();
    virtual void updateStyleListOrder( const QStringList & list);
protected:
    KPresenterDoc *m_doc;
};

#endif
