/* This file is part of the KDE project
   Copyright (C) 2003 Norbert Andres, nandres@web.de

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


#include "kspread_style.h"
#include "kspread_style_manager.h"
#include <koOasisStyles.h>
#include <kdebug.h>
#include <klocale.h>
#include <qdom.h>
#include <qstringlist.h>

KSpreadStyleManager::KSpreadStyleManager()
  : m_defaultStyle( new KSpreadCustomStyle() )
{
  m_defaultStyle->setName( "Default" );
  m_defaultStyle->setType( KSpreadStyle::BUILTIN );
}

KSpreadStyleManager::~KSpreadStyleManager()
{
  delete m_defaultStyle;

  Styles::iterator iter = m_styles.begin();
  Styles::iterator end  = m_styles.end();

  while ( iter != end )
  {
    delete iter.data();

    ++iter;
  }
}

void KSpreadStyleManager::saveOasis( KoGenStyles &mainStyles )
{
    kdDebug() << "Saving default oasis style" << endl;
    m_defaultStyle->saveOasis( mainStyles );

    Styles::iterator iter = m_styles.begin();
    Styles::iterator end  = m_styles.end();

    while ( iter != end )
    {
        kdDebug() << "Saving style" << endl;
        KSpreadCustomStyle * styleData = iter.data();

        styleData->saveOasis( mainStyles );

        ++iter;
    }

}

void KSpreadStyleManager::loadOasisStyleTemplate(  KoOasisStyles& oasisStyles )
{
    uint nStyles = oasisStyles.userStyles().count();
    kdDebug()<<" number of template style to load : "<<nStyles<<endl;
    for (unsigned int item = 0; item < nStyles; item++) {
        QDomElement styleElem = oasisStyles.userStyles()[item];
        QString name;
        if ( styleElem.hasAttribute( "style:display-name" ) )
        {
            name = styleElem.attribute( "style:display-name" );
        }

        if ( name == "Default" )
        {
            m_defaultStyle->loadOasis( styleElem,name );
            m_defaultStyle->setType( KSpreadStyle::BUILTIN );
        }
        else if ( !name.isEmpty() )
        {
            KSpreadCustomStyle * style = 0;
            if ( styleElem.hasAttribute( "style:parent-style-name" ) && styleElem.attribute( "style:parent-style-name" ) == "Default" )
                style = new KSpreadCustomStyle( name, m_defaultStyle );
            else
                style = new KSpreadCustomStyle( name, 0 );

            //fixme test return;
            style->loadOasis( styleElem,name );
            style->setType( KSpreadStyle::CUSTOM );
            m_styles[name] = style;
            kdDebug() << "Style " << name << ": " << style << endl;

        }
    }
}

QDomElement KSpreadStyleManager::save( QDomDocument & doc )
{
  kdDebug() << "Saving styles" << endl;
  QDomElement styles = doc.createElement( "styles" );

  kdDebug() << "Saving default style" << endl;
  m_defaultStyle->save( doc, styles );

  Styles::iterator iter = m_styles.begin();
  Styles::iterator end  = m_styles.end();

  while ( iter != end )
  {
    kdDebug() << "Saving style" << endl;
    KSpreadCustomStyle * styleData = iter.data();

    styleData->save( doc, styles );

    ++iter;
  }

  kdDebug() << "End saving styles" << endl;
  return styles;
}

bool KSpreadStyleManager::loadXML( QDomElement const & styles )
{
  QDomElement e = styles.firstChild().toElement();
  while ( !e.isNull() )
  {
    QString name;
    if ( e.hasAttribute( "name" ) )
      name = e.attribute( "name" );

    if ( name == "Default" )
    {
      if ( !m_defaultStyle->loadXML( e, name ) )
        return false;
      m_defaultStyle->setType( KSpreadStyle::BUILTIN );
    }
    else if ( !name.isNull() )
    {
      KSpreadCustomStyle * style = 0;
      if ( e.hasAttribute( "parent" ) && e.attribute( "parent" ) == "Default" )
        style = new KSpreadCustomStyle( name, m_defaultStyle );
      else
        style = new KSpreadCustomStyle( name, 0 );

      if ( !style->loadXML( e, name ) )
      {
        delete style;
        return false;
      }

      if ( style->type() == KSpreadStyle::AUTO )
        style->setType( KSpreadStyle::CUSTOM );
      m_styles[name] = style;
      kdDebug() << "Style " << name << ": " << style << endl;
    }

    e = e.nextSibling().toElement();
  }

  Styles::iterator iter = m_styles.begin();
  Styles::iterator end  = m_styles.end();

  while ( iter != end )
  {
    KSpreadCustomStyle * styleData = iter.data();

    if ( !styleData->parent() && !styleData->parentName().isNull() )
      styleData->setParent( m_styles[ styleData->parentName() ] );

    ++iter;
  }

  m_defaultStyle->setName( "Default" );
  m_defaultStyle->setType( KSpreadStyle::BUILTIN );

  return true;
}

void KSpreadStyleManager::createBuiltinStyles()
{
  KSpreadCustomStyle * header1 = new KSpreadCustomStyle( i18n( "Header" ), m_defaultStyle );
  QFont f( header1->font() );
  f.setItalic( true );
  f.setPointSize( f.pointSize() + 2 );
  f.setBold( true );
  header1->changeFont( f );
  header1->setType( KSpreadStyle::BUILTIN );
  m_styles[ header1->name() ] = header1;

  KSpreadCustomStyle * header2 = new KSpreadCustomStyle( i18n( "Header1" ), header1 );
  QColor color( "#F0F0FF" );
  header2->changeBgColor( color );
  QPen pen( Qt::black, 1, Qt::SolidLine );
  header2->changeBottomBorderPen( pen );
  header2->setType( KSpreadStyle::BUILTIN );

  m_styles[ header2->name() ] = header2;
}

KSpreadCustomStyle * KSpreadStyleManager::style( QString const & name ) const
{
  Styles::const_iterator iter( m_styles.find( name ) );

  if ( iter != m_styles.end() )
    return iter.data();

  if ( name == i18n( "Default" ) || name == "Default" )
    return m_defaultStyle;

  return 0;
}

void KSpreadStyleManager::takeStyle( KSpreadCustomStyle * style )
{
  KSpreadCustomStyle * parent = style->parent();

  Styles::iterator iter = m_styles.begin();
  Styles::iterator end  = m_styles.end();

  while ( iter != end )
  {
    if ( iter.data()->parent() == style )
      iter.data()->setParent( parent );

    ++iter;
  }

  Styles::iterator i( m_styles.find( style->name() ) );

  if ( i != m_styles.end() )
  {
    kdDebug() << "Erasing style entry for " << style->name() << endl;
    m_styles.erase( i );
  }
}

bool KSpreadStyleManager::checkCircle( QString const & name, QString const & parent )
{
  KSpreadCustomStyle * s = style( parent );
  if ( !s || s->parent() == 0 )
    return true;
  if ( s->parentName() == name )
    return false;
  else
    return checkCircle( name, s->parentName() );
}

bool KSpreadStyleManager::validateStyleName( QString const & name, KSpreadCustomStyle * style )
{
  if ( m_defaultStyle->name() == name || name == i18n( "Default" ) )
    return false;

  Styles::const_iterator iter = m_styles.begin();
  Styles::const_iterator end  = m_styles.end();

  while ( iter != end )
  {
    if ( iter.key() == name && iter.data() != style )
      return false;

    ++iter;
  }

  return true;
}

void KSpreadStyleManager::changeName( QString const & oldName, QString const & newName )
{
  Styles::iterator iter = m_styles.begin();
  Styles::iterator end  = m_styles.end();

  while ( iter != end )
  {
    if ( iter.data()->parentName() == oldName )
      iter.data()->refreshParentName();

    ++iter;
  }

  iter = m_styles.find( oldName );
  if ( iter != end )
  {
    KSpreadCustomStyle * s = iter.data();
    m_styles.erase( iter );
    m_styles[newName] = s;
  }
}

QStringList KSpreadStyleManager::styleNames() const
{
  QStringList list;

  list.push_back( i18n( "Default" ) );

  Styles::const_iterator iter = m_styles.begin();
  Styles::const_iterator end  = m_styles.end();

  while ( iter != end )
  {
    list.push_back( iter.key() );

    ++iter;
  }

  return list;
}

