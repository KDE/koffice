/* This file is part of the KDE project
   Copyright 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 1999- 2003 The KSpread Team
                        www.koffice.org/kspread

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <float.h>

#include "kspread_cell.h"
#include "kspread_sheet.h"
#include "kspread_doc.h"
#include "kspread_style.h"
#include "kspread_style_manager.h"
#include "kspread_util.h"

#include <KoGenStyles.h>

#include <KoXmlWriter.h>
#include <KoXmlNS.h>
#include <kdebug.h>
#include <qdom.h>
#include <qbuffer.h>

#include "kspread_condition.h"

using namespace KSpread;

Conditional::Conditional():
  val1( 0.0 ), val2( 0.0 ), strVal1( 0 ), strVal2( 0 ),
  colorcond( 0 ), fontcond( 0 ), styleName( 0 ),
  style( 0 ), cond( None )
{
}

Conditional::~Conditional()
{
  delete strVal1;
  delete strVal2;
  delete colorcond;
  delete fontcond;
  delete styleName;
}

Conditional::Conditional( const Conditional& c )
{
  operator=( c );
}

Conditional& Conditional::operator=( const Conditional& d )
{
  strVal1 = d.strVal1 ? new QString( *d.strVal1 ) : 0;
  strVal2 = d.strVal2 ? new QString( *d.strVal2 ) : 0;
  styleName = d.styleName ? new QString( *d.styleName ) : 0;
  fontcond = d.fontcond ? new QFont( *d.fontcond ) : 0;
  colorcond = d.colorcond ? new QColor( *d.colorcond ) : 0;
  val1  = d.val1;
  val2  = d.val2;
  style = d.style;
  cond  = d.cond;

  return *this;
}

bool Conditional::operator==(const Conditional& other) const
{
  if ( cond == other.cond &&
       val1 == other.val1 &&
       val2 == other.val2 &&
       *strVal1 == *other.strVal1 &&
       *strVal2 == *other.strVal2 &&
       *colorcond == *other.colorcond &&
       *fontcond == *other.fontcond &&
       *styleName == *other.styleName &&
       *style == *other.style )
  {
    return true;
  }
  return false;
}

Conditions::Conditions( const Cell * ownerCell )
  : m_cell( ownerCell ), m_matchedStyle( 0 )
{
  Q_ASSERT( ownerCell != NULL );
}

Conditions::~Conditions()
{
  m_condList.clear();
}

void Conditions::checkMatches()
{
  Conditional condition;

  if ( currentCondition( condition ) )
    m_matchedStyle = condition.style;
  else
    m_matchedStyle = 0;
}

bool Conditions::currentCondition( Conditional & condition )
{
  /* for now, the first condition that is true is the one that will be used */

  QValueList<Conditional>::const_iterator it;
  double value   = m_cell->value().asFloat();
  QString strVal = m_cell->value().asString();


  for ( it = m_condList.begin(); it != m_condList.end(); ++it )
  {
    condition = *it;

//     if ( (*it).styleName )
//         kdDebug()<<"*it :"<<  *( ( *it ).styleName ) <<endl;
//
//     kdDebug()<<"*it style :"<<(  *it ).style <<endl;


    if ( condition.strVal1 && m_cell->value().isNumber() )
      continue;

    switch ( condition.cond )
    {
      case Conditional::Equal:
      if ( condition.strVal1 )
      {
        if ( strVal == *condition.strVal1 )
          return true;
      }
      else
      if ( value - condition.val1 < DBL_EPSILON &&
           value - condition.val1 > (0.0 - DBL_EPSILON) )
      {
        return true;
      }
      break;

      case Conditional::Superior:
      if ( condition.strVal1 )
      {
        if ( strVal > *condition.strVal1 )
          return true;
      }
      else
      if ( value > condition.val1 )
      {
        return true;
      }
      break;

      case Conditional::Inferior:
      if ( condition.strVal1 )
      {
        if ( strVal < *condition.strVal1 )
          return true;
      }
      else
      if ( value < condition.val1 )
      {
        return true;
      }
      break;

      case Conditional::SuperiorEqual :
      if ( condition.strVal1 )
      {
        if ( strVal >= *condition.strVal1 )
          return true;
      }
      else
      if ( value >= condition.val1 )
      {
        return true;
      }
      break;

      case Conditional::InferiorEqual :
      if ( condition.strVal1 )
      {
        if ( strVal <= *condition.strVal1 )
          return true;
      }
      else
      if ( value <= condition.val1 )
      {
        return true;
      }
      break;

      case Conditional::Between :
      if ( condition.strVal1 && condition.strVal2 )
      {
        if ( strVal > *condition.strVal1 && strVal < *condition.strVal2 )
          return true;
      }
      else
      if ( ( value > QMIN(condition.val1, condition.val2 ) )
           && ( value < QMAX(condition.val1, condition.val2 ) ) )
      {
        return true;
      }
      break;

      case Conditional::Different :
      if ( condition.strVal1 && condition.strVal2 )
      {
        if ( strVal < *condition.strVal1 || strVal > *condition.strVal2 )
          return true;
      }
      else
      if ( ( value < QMIN(condition.val1, condition.val2 ) )
           || ( value > QMAX(condition.val1, condition.val2) ) )
      {
        return true;
      }
      break;
      case Conditional::DifferentTo :
      if ( condition.strVal1 )
      {
        if ( strVal != *condition.strVal1 )
          return true;
      }
      else
      if ( value != condition.val1 )
      {
        return true;
      }
      break;

     default:
      break;
    }
  }
  return false;
}

QValueList<Conditional> Conditions::conditionList() const
{
  return m_condList;
}

void Conditions::setConditionList( const QValueList<Conditional> & list )
{
  m_condList.clear();

  QValueList<Conditional>::const_iterator it;
  for ( it = list.begin(); it != list.end(); ++it )
  {
    Conditional d = *it;
    m_condList.append( Conditional( d ) );
  }
}

void Conditions::saveOasisConditions( KoGenStyle &currentCellStyle )
{
    //todo fix me with kspread old format!!!
    if ( m_condList.isEmpty() )
        return;
    QValueList<Conditional>::const_iterator it;
    int i = 0;
    for ( it = m_condList.begin(); it != m_condList.end(); ++it, ++i )
    {
        Conditional condition = *it;
        //<style:map style:condition="cell-content()=45" style:apply-style-name="Default" style:base-cell-address="Sheet1.E10"/>
        QMap<QString, QString> map;
        map.insert( "style:condition", saveOasisConditionValue( condition ) );
        map.insert( "style:apply-style-name",  *( condition.styleName ) );
        //map.insert( ""style:base-cell-address", "..." );//todo
        currentCellStyle.addStyleMap( map );
    }
}

QString Conditions::saveOasisConditionValue( Conditional &condition)
{
    //we can also compare text value.
    //todo adapt it.
    QString value;
    switch( condition.cond )
    {
      case Conditional::None:
        break;
      case Conditional::Equal:
        value="cell-content()=";
        if ( condition.strVal1 )
            value+=*condition.strVal1;
        else
            value+=QString::number( condition.val1 );
        break;
      case Conditional::Superior:
        value="cell-content()>";
        if ( condition.strVal1 )
            value+=*condition.strVal1;
        else
            value+=QString::number( condition.val1 );
        break;
      case Conditional::Inferior:
        value="cell-content()<";
        if ( condition.strVal1 )
            value+=*condition.strVal1;
        else
            value+=QString::number( condition.val1 );
        break;
      case Conditional::SuperiorEqual:
        value="cell-content()>=";
        if ( condition.strVal1 )
            value+=*condition.strVal1;
        else
            value+=QString::number( condition.val1 );
        break;
      case Conditional::InferiorEqual:
        value="cell-content()<=";
        if ( condition.strVal1 )
            value+=*condition.strVal1;
        else
            value+=QString::number( condition.val1 );
        break;
      case Conditional::Between:
        value="cell-content-is-between(";
        if ( condition.strVal1 )
        {
            value+=*condition.strVal1;
            value+=",";
            if ( condition.strVal2 )
                value+=*condition.strVal2;
        }
        else
        {
            value+=QString::number( condition.val1 );
            value+=",";
            value+=QString::number( condition.val2 );
        }
        value+=")";
        break;
      case Conditional::DifferentTo:
        value="cell-content()!="; //FIXME not good here !
        if ( condition.strVal1 )
            value+=*condition.strVal1;
        else
            value+=QString::number( condition.val1 );
        break;
      case Conditional::Different:
        value="cell-content-is-not-between(";
        if ( condition.strVal1 )
        {
            value+=*condition.strVal1;
            value+=",";
            if ( condition.strVal2 )
                value+=*condition.strVal2;
        }
        else
        {
            value+=QString::number( condition.val1 );
            value+=",";
            value+=QString::number( condition.val2 );
        }
        value+=")";
        break;
    }
    return value;
}


QDomElement Conditions::saveConditions( QDomDocument & doc ) const
{
  QDomElement conditions = doc.createElement("condition");
  QValueList<Conditional>::const_iterator it;
  QDomElement child;
  int num = 0;
  QString name;

  for ( it = m_condList.begin(); it != m_condList.end(); ++it )
  {
    Conditional condition = *it;

    /* the name of the element will be "condition<n>"
     * This is unimportant now but in older versions three conditions were
     * hardcoded with names "first" "second" and "third"
     */
    name.setNum( num );
    name.prepend( "condition" );

    child = doc.createElement( name );
    child.setAttribute( "cond", (int) condition.cond );

    // TODO: saving in KSpread 1.1 | KSpread 1.2 format
    if ( condition.strVal1 )
    {
      child.setAttribute( "strval1", *condition.strVal1 );
      if ( condition.strVal2 )
        child.setAttribute( "strval2", *condition.strVal2 );
    }
    else
    {
      child.setAttribute( "val1", condition.val1 );
      child.setAttribute( "val2", condition.val2 );
    }
    if ( condition.styleName )
    {
      child.setAttribute( "style", *condition.styleName );
    }
    else
    {
      child.setAttribute( "color", condition.colorcond->name() );
      child.appendChild( util_createElement( "font", *condition.fontcond, doc ) );
    }

    conditions.appendChild( child );

    ++num;
  }

  if ( num == 0 )
  {
    /* there weren't any real conditions -- return a null dom element */
    return QDomElement();
  }
  else
  {
    return conditions;
  }
}

void Conditions::loadOasisConditions( const QDomElement & element )
{
    kdDebug(36003) << "Loading conditional styles" << endl;
    QDomNode node( element );
    StyleManager * manager = m_cell->sheet()->doc()->styleManager();

    while ( !node.isNull() )
    {
        QDomElement elementItem = node.toElement();
        if ( elementItem.tagName()== "map" && elementItem.namespaceURI() == KoXmlNS::style  )
        {
            bool ok = true;
            kdDebug(36003) << "\tcondition: "<< elementItem.attributeNS( KoXmlNS::style, "condition", QString::null )<<endl;
            Conditional newCondition;
            loadOasisConditionValue( elementItem.attributeNS( KoXmlNS::style, "condition", QString::null ), newCondition );
            if ( elementItem.hasAttributeNS( KoXmlNS::style, "apply-style-name" ) )
            {
                kdDebug(36003)<<"\tstyle: "<<elementItem.attributeNS( KoXmlNS::style, "apply-style-name", QString::null )<<endl;
                newCondition.styleName = new QString( elementItem.attributeNS( KoXmlNS::style, "apply-style-name", QString::null ) );
                newCondition.style = manager->style( *newCondition.styleName );
                if ( !newCondition.style )
                    ok = false;
                else
                    ok = true;
            }

            if ( ok )
                m_condList.append( newCondition );
            else
                kdDebug(36003) << "Error loading condition " << elementItem.nodeName()<< endl;
        }
        node = node.nextSibling();
    }
}

void Conditions::loadOasisConditionValue( const QString &styleCondition, Conditional &newCondition )
{
    QString val( styleCondition );
    if ( val.contains( "cell-content()" ) )
    {
        val = val.remove( "cell-content()" );
        loadOasisCondition( val,newCondition );
    }
    //GetFunction ::= cell-content-is-between(Value, Value) | cell-content-is-not-between(Value, Value)
    //for the moment we support just int/double value, not text/date/time :(
    if ( val.contains( "cell-content-is-between(" ) )
    {
        val = val.remove( "cell-content-is-between(" );
        val = val.remove( ")" );
        QStringList listVal = QStringList::split( "," , val );
        loadOasisValidationValue( listVal, newCondition );
        newCondition.cond = Conditional::Between;
    }
    if ( val.contains( "cell-content-is-not-between(" ) )
    {
        val = val.remove( "cell-content-is-not-between(" );
        val = val.remove( ")" );
        QStringList listVal = QStringList::split( ",", val );
        loadOasisValidationValue( listVal,newCondition );
        newCondition.cond = Conditional::Different;
    }

}


void Conditions::loadOasisCondition( QString &valExpression, Conditional &newCondition )
{
    QString value;
    if (valExpression.find( "<=" )==0 )
    {
        value = valExpression.remove( 0,2 );
        newCondition.cond = Conditional::InferiorEqual;
    }
    else if (valExpression.find( ">=" )==0 )
    {
        value = valExpression.remove( 0,2 );
        newCondition.cond = Conditional::SuperiorEqual;
    }
    else if (valExpression.find( "!=" )==0 )
    {
        //add Differentto attribute
        value = valExpression.remove( 0,2 );
        newCondition.cond = Conditional::DifferentTo;
    }
    else if ( valExpression.find( "<" )==0 )
    {
        value = valExpression.remove( 0,1 );
        newCondition.cond = Conditional::Inferior;
    }
    else if(valExpression.find( ">" )==0 )
    {
        value = valExpression.remove( 0,1 );
        newCondition.cond = Conditional::Superior;
    }
    else if (valExpression.find( "=" )==0 )
    {
        value = valExpression.remove( 0,1 );
        newCondition.cond = Conditional::Equal;
    }
    else
        kdDebug()<<" I don't know how to parse it :"<<valExpression<<endl;
    kdDebug(36003) << "\tvalue: " << value << endl;
    bool ok = false;
    newCondition.val1 = value.toDouble(&ok);
    if ( !ok )
    {
        newCondition.val1 = value.toInt(&ok);
        if ( !ok )
        {
            newCondition.strVal1 = new QString( value );
            kdDebug()<<" Try to parse this value :"<<value<<endl;
        }
    }
}


void Conditions::loadOasisValidationValue( const QStringList &listVal, Conditional &newCondition )
{
    bool ok = false;
    kdDebug()<<" listVal[0] :"<<listVal[0]<<" listVal[1] :"<<listVal[1]<<endl;

    newCondition.val1 = listVal[0].toDouble(&ok);
    if ( !ok )
    {
        newCondition.val1 = listVal[0].toInt(&ok);
        if ( !ok )
        {
            newCondition.strVal1 = new QString( listVal[0] );
            kdDebug()<<" Try to parse this value :"<<listVal[0]<<endl;
        }
    }
    ok=false;
    newCondition.val2 = listVal[1].toDouble(&ok);
    if ( !ok )
    {
        newCondition.val2 = listVal[1].toInt(&ok);
        if ( !ok )
        {
            newCondition.strVal2 = new QString( listVal[1] );
            kdDebug()<<" Try to parse this value :"<<listVal[1]<<endl;
        }
    }
}


void Conditions::loadConditions( const QDomElement & element )
{
  QDomNodeList nodeList = element.childNodes();
  Conditional newCondition;
  bool ok;
  StyleManager * manager = m_cell->sheet()->doc()->styleManager();

  for ( int i = 0; i < (int)(nodeList.length()); ++i )
  {
    newCondition.strVal1   = 0;
    newCondition.strVal2   = 0;
    newCondition.styleName = 0;
    newCondition.fontcond  = 0;
    newCondition.colorcond = 0;

    QDomElement conditionElement = nodeList.item( i ).toElement();

    ok = conditionElement.hasAttribute( "cond" );

    if ( ok )
      newCondition.cond = (Conditional::Type) conditionElement.attribute( "cond" ).toInt( &ok );
    else continue;

    if ( conditionElement.hasAttribute( "val1" ) )
    {
      newCondition.val1 = conditionElement.attribute( "val1" ).toDouble( &ok );

      if ( conditionElement.hasAttribute( "val2" ) )
        newCondition.val2 = conditionElement.attribute("val2").toDouble( &ok );
    }

    if ( conditionElement.hasAttribute( "strval1" ) )
    {
      newCondition.strVal1 = new QString( conditionElement.attribute( "strval1" ) );

      if ( conditionElement.hasAttribute( "strval2" ) )
        newCondition.strVal2 = new QString( conditionElement.attribute( "strval2" ) );
    }

    if ( conditionElement.hasAttribute( "color" ) )
      newCondition.colorcond = new QColor( conditionElement.attribute( "color" ) );

    QDomElement font = conditionElement.namedItem( "font" ).toElement();
    if ( !font.isNull() )
      newCondition.fontcond = new QFont( util_toFont( font ) );

    if ( conditionElement.hasAttribute( "style" ) )
    {
      newCondition.styleName = new QString( conditionElement.attribute( "style" ) );
      newCondition.style = manager->style( *newCondition.styleName );
      if ( !newCondition.style )
        ok = false;
    }

    if ( ok )
    {
      m_condList.append( newCondition );
    }
    else
    {
      kdDebug(36001) << "Error loading condition " << conditionElement.nodeName()<< endl;
    }
  }
}

bool Conditions::operator==( const Conditions& other ) const
{
  if ( !( *m_matchedStyle == *other.m_matchedStyle ) )
    return false;
  if ( m_condList.count() != other.m_condList.count() )
    return false;
  QValueList<Conditional>::ConstIterator end( m_condList.end() );
  for ( QValueList<Conditional>::ConstIterator it( m_condList.begin() ); it != end; ++it )
  {
    bool found = false;
    QValueList<Conditional>::ConstIterator otherEnd( other.m_condList.end() );
    for ( QValueList<Conditional>::ConstIterator otherIt( other.m_condList.begin() ); otherIt != otherEnd; ++otherIt )
    {
      if ( (*it) == (*otherIt) )
        found = true;
    }
    if ( !found )
      return false;
  }
  return true;
}
