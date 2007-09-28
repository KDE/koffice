/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoGenStyles.h"
#include <KoXmlWriter.h>
#include <float.h>
#include <kdebug.h>

class KoGenStyles::Private
{
};

KoGenStyles::KoGenStyles()
    : d( 0 )
{
}

KoGenStyles::~KoGenStyles()
{
    delete d;
}

QString KoGenStyles::lookup( const KoGenStyle& style, const QString& name, int flags )
{
    StyleMap::iterator it = m_styleMap.find( style );
    if ( it == m_styleMap.end() ) {
        // Not found, try if this style is in fact equal to its parent (the find above
        // wouldn't have found it, due to m_parentName being set).
        if ( !style.parentName().isEmpty() ) {
            KoGenStyle testStyle( style );
            const KoGenStyle* parentStyle = this->style( style.parentName() ); // ## linear search
            if( !parentStyle ) {
                kDebug(30003) <<"KoGenStyles::lookup(" << name <<"): parent style '" << style.parentName() <<"' not found in collection";
            } else {
                if ( testStyle.m_familyName != parentStyle->m_familyName )
                {
                    kWarning(30003) << "KoGenStyles::lookup(" << name << ", family=" << testStyle.m_familyName << ") parent style '" << style.parentName() << "' has a different family: " << parentStyle->m_familyName;
                }

                testStyle.m_parentName = parentStyle->m_parentName;
                // Exclude the type from the comparison. It's ok for an auto style
                // to have a user style as parent; they can still be identical
                testStyle.m_type = parentStyle->m_type;
                // Also it's ok to not have the display name of the parent style
                // in the auto style
                QMap<QString, QString>::const_iterator it = parentStyle->m_attributes.find( "style:display-name" );
                if ( it != parentStyle->m_attributes.end() )
                    testStyle.addAttribute( "style:display-name", *it );

                if ( *parentStyle == testStyle )
                    return style.parentName();
            }
        }

        QString styleName( name );
        if ( styleName.isEmpty() ) {
            styleName = 'A'; // for "auto".
            flags &= ~DontForceNumbering; // i.e. force numbering
        }
        styleName = makeUniqueName( styleName, flags );
        if ( style.autoStyleInStylesDotXml() )
            m_autoStylesInStylesDotXml.insert( styleName );
        else
            m_styleNames.insert( styleName );
        it = m_styleMap.insert( style, styleName );
        NamedStyle s;
        s.style = &it.key();
        s.name = styleName;
        m_styleArray.append( s );
    }
    return it.value();
}

QString KoGenStyles::makeUniqueName( const QString& base, int flags ) const
{
    // If this name is not used yet, and numbering isn't forced, then the given name is ok.
    if ( ( flags & DontForceNumbering )
         && ! m_autoStylesInStylesDotXml.contains( base )
         && ! m_styleNames.contains( base ) )
        return base;
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number( num++ );
    } while ( m_autoStylesInStylesDotXml.contains( name )
              || m_styleNames.contains( name ) );
    return name;
}

QList<KoGenStyles::NamedStyle> KoGenStyles::styles( int type, bool markedForStylesXml ) const
{
    QList<KoGenStyles::NamedStyle> lst;
    const NameMap& nameMap = markedForStylesXml ? m_autoStylesInStylesDotXml : m_styleNames;
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        // Look up if it's marked for styles.xml or not by looking up in the corresponding style map.
        if ( (*it).style->type() == type && nameMap.find((*it).name) != nameMap.end() ) {
            lst.append( *it );
        }
    }
    return lst;
}

const KoGenStyle* KoGenStyles::style( const QString& name ) const
{
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        if ( (*it).name == name )
            return (*it).style;
    }
    return 0;
}

KoGenStyle* KoGenStyles::styleForModification( const QString& name )
{
    return const_cast<KoGenStyle *>( style( name ) );
}

void KoGenStyles::markStyleForStylesXml( const QString& name )
{
    Q_ASSERT( m_styleNames.contains( name ) );
    m_styleNames.remove( name );
    m_autoStylesInStylesDotXml.insert( name );
    styleForModification( name )->setAutoStyleInStylesDotXml( true );
}

void KoGenStyles::dump()
{
    kDebug(30003) <<"Style array:";
    StyleArray::const_iterator it = m_styleArray.begin();
    const StyleArray::const_iterator end = m_styleArray.end();
    for ( ; it != end ; ++it ) {
        kDebug(30003) << (*it).name;
    }
    for ( NameMap::const_iterator it = m_styleNames.begin(); it != m_styleNames.end(); ++it ) {
        kDebug(30003) <<"style:" << *it;
    }
    for ( NameMap::const_iterator it = m_autoStylesInStylesDotXml.begin(); it != m_autoStylesInStylesDotXml.end(); ++it ) {
        kDebug(30003) <<"auto style for style.xml:" << *it;
        const KoGenStyle* s = style( *it );
        Q_ASSERT( s );
        Q_ASSERT( s->autoStyleInStylesDotXml() );
    }
}

void KoGenStyles::saveOdfAutomaticStyles( KoXmlWriter* xmlWriter, bool stylesDotXml )
{
    QList<KoGenStyles::NamedStyle> stylesList = styles( KoGenStyle::StyleGraphicAuto, stylesDotXml );
    QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "style:style", ( *it ).name , "style:graphic-properties" );
    }

    stylesList = styles( KoGenStyle::StyleDrawingPage, stylesDotXml );
    it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "style:style", ( *it ).name , "style:drawing-page-properties" );
    }

    stylesList = styles( KoGenStyle::StylePageLayout, stylesDotXml );
    it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "style:page-layout", (*it).name, "style:page-layout-properties" );
    }
}


void KoGenStyles::saveOdfDocumentStyles( KoXmlWriter* xmlWriter )
{
    QList<KoGenStyles::NamedStyle> stylesList = styles( KoGenStyle::StyleGradientLinear );
    QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "svg:linearGradient", ( *it ).name, 0, true, true /*add draw:name*/ );
    }

    stylesList = styles( KoGenStyle::StyleGradientRadial );
    it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "svg:radialGradient", ( *it ).name, 0, true, true /*add draw:name*/ );
    }

    stylesList = styles( KoGenStyle::StyleStrokeDash );
    it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "draw:stroke-dash", ( *it ).name, 0, true, true /*add draw:name*/ );
    }

    stylesList = styles( KoGenStyle::StyleFillImage );
    it = stylesList.begin();
    for ( ; it != stylesList.end() ; ++it ) {
        ( *it ).style->writeStyle( xmlWriter, *this, "draw:fill-image", ( *it ).name, 0, true, true /*add draw:name*/ );
    }
}
