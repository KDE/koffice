/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KWSTYLEMANAGER_H
#define KWSTYLEMANAGER_H

#include <koStylist.h>

class KWDocument;
class KoStyle;
class KoUnit;
/******************************************************************/
/* Class: KWStyleManager                                          */
/******************************************************************/
class KWStyleManager : public KoStyleManager
{
    Q_OBJECT

public:
    KWStyleManager( QWidget *_parent, KoUnit::Unit unit, KWDocument *_doc, const QPtrList<KoStyle> & style );

    virtual KoStyle* addStyleTemplate(KoStyle *style);
    virtual void applyStyleChange( StyleChangeDefMap changed  );
    virtual void removeStyleTemplate( KoStyle *style );
    virtual void updateAllStyleLists();
    virtual void updateStyleListOrder( const QStringList & list);
protected:
    KWDocument *m_doc;
};

#endif
