/* This file is part of the KDE project
   Copyright (C) 2004 David Faure <faure@kde.org>

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

#include <koOasisStyles.h>
#include "kooasiscontext.h"
#include <kdebug.h>

KoOasisContext::KoOasisContext( KoOasisStyles& styles )
    : m_styles( styles )
{
}

void KoOasisContext::fillStyleStack( const QDomElement& object, const QString& attrName )
{
    // find all styles associated with an object and push them on the stack
    // OoImpressImport has more tests here, but I don't think they're relevant to OoWriterImport
    if ( object.hasAttribute( attrName ) )
        addStyles( m_styles.styles()[object.attribute( attrName )] );
}

void KoOasisContext::addStyles( const QDomElement* style )
{
    // this recursive function is necessary as parent styles can have parents themselves
    if ( style->hasAttribute( "style:parent-style-name" ) )
        addStyles( m_styles.styles()[style->attribute( "style:parent-style-name" )] );
    else if ( !m_styles.defaultStyle().isNull() ) // on top of all, the default style
        m_styleStack.push( m_styles.defaultStyle() );

    //kdDebug(32500) << "pushing style " << style->attribute( "style:name" ) << endl;
    m_styleStack.push( *style );
}

static QDomElement findListLevelStyle( const QDomElement& fullListStyle, int level )
{
    for ( QDomNode n = fullListStyle.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
       const QDomElement listLevelItem = n.toElement();
       if ( listLevelItem.attribute( "text:level" ).toInt() == level )
           return listLevelItem;
    }
    return QDomElement();
}

bool KoOasisContext::pushListLevelStyle( const QString& listStyleName, int level )
{
    QDomElement* fullListStyle = m_styles.listStyles()[listStyleName];
    if ( !fullListStyle ) {
        kdWarning(32500) << "List style " << listStyleName << " not found!" << endl;
        return false;
    }
    else
        return pushListLevelStyle( listStyleName, *fullListStyle, level );
}

bool KoOasisContext::pushOutlineListLevelStyle( int level )
{
    QDomElement outlineStyle = m_styles.officeStyle().namedItem( "text:outline-style" ).toElement();
    Q_ASSERT( !outlineStyle.isNull() );
    return pushListLevelStyle( "<outline-style>", outlineStyle, level );
}

bool KoOasisContext::pushListLevelStyle( const QString& listStyleName, // for debug only
                                         const QDomElement& fullListStyle, int level )
{
    // Find applicable list-level-style for level
    int i = level;
    QDomElement listLevelStyle;
    while ( i > 0 && listLevelStyle.isNull() ) {
        listLevelStyle = findListLevelStyle( fullListStyle, i );
        --i;
    }
    if ( listLevelStyle.isNull() ) {
        kdWarning(32500) << "List level style for level " << level << " in list style " << listStyleName << " not found!" << endl;
        return false;
    }
    kdDebug(32500) << "Pushing list-level-style from list-style " << listStyleName << " level " << level << endl;
    m_listStyleStack.push( listLevelStyle );
    return true;
}
