/* This file is part of the KDE project
   Copyright (C) 2003,2004 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2005 Tomas Mecir <mecirt@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; only
   version 2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

// Local
#include "Functions.h"

#include <QDomElement>
#include <QDomNode>
#include <QFile>
#include <QHash>

#include <KComponentData>
#include <kdebug.h>
#include <KGlobal>
#include <klocale.h>
#include <kstandarddirs.h>

#include "part/Factory.h" // FIXME detach from part
#include "Formula.h"
#include "FunctionModuleRegistry.h"
#include "ValueCalc.h"

namespace KSpread
{

class Function::Private
{
public:
  QString name;
  FunctionPtr ptr;
  int paramMin, paramMax;
  bool acceptArray;
  bool ne;   // need FunctionExtra* when called ?
};

class FunctionRepository::Private
{
public:
  QHash<QString, Function*> functions;
  QHash<QString, FunctionDescription*> descriptions;
  QStringList groups;
    bool initialized;
};

} // namespace KSpread


using namespace KSpread;

Function::Function( const QString& name, FunctionPtr ptr )
    : d( new Private )
{
  d->name = name;
  d->ptr = ptr;
  d->acceptArray = false;
  d->paramMin = 1;
  d->paramMax = 1;
  d->ne = false;
}

Function::~Function()
{
  delete d;
}

QString Function::name() const
{
  return d->name;
}

void Function::setParamCount (int min, int max)
{
  d->paramMin = min;
  d->paramMax = (max == 0) ? min : max;
}

bool Function::paramCountOkay (int count)
{
  // less than needed
  if (count < d->paramMin) return false;
  // no upper limit
  if (d->paramMax == -1) return true;
  // more than needed
  if (count > d->paramMax) return false;
  // okay otherwise
  return true;
}

void Function::setAcceptArray (bool accept) {
  d->acceptArray = accept;
}

bool Function::needsExtra () {
  return d->ne;
}
void Function::setNeedsExtra (bool extra) {
  d->ne = extra;
}

Value Function::exec (valVector args, ValueCalc *calc, FuncExtra *extra)
{
  // check number of parameters
  if (!paramCountOkay (args.count()))
    return Value::errorVALUE();

  if( extra )
    extra->function = this;

  // do we need to perform array expansion ?
  bool mustExpandArray = false;
  if (!d->acceptArray)
    for (int i = 0; i < args.count(); ++i) {
      if (args[i].isArray())
        mustExpandArray = true;
    }

  if( !d->ptr ) return Value::errorVALUE();

  // perform the actual array expansion if need be

  if (mustExpandArray) {
    // compute number of rows/cols of the result
    int rows = 0;
    int cols = 0;
    for (int i = 0; i < args.count(); ++i) {
      int x = (args[i].type() == Value::Array) ? args[i].rows() : 1;
      if (x > rows) rows = x;
      x = (args[i].type() == Value::Array) ? args[i].columns() : 1;
      if (x > cols) cols = x;
    }
    // allocate the resulting array
    Value res( Value::Array );
    // perform the actual computation for each element of the array
    for (int row = 0; row < rows; ++row)
      for (int col = 0; col < cols; ++col) {
        // fill in the parameter vector
        valVector vals (args.count());
        for (int i = 0; i < args.count(); ++i) {
          int r = args[i].rows();
          int c = args[i].columns();
          vals[i] = args[i].isArray() ?
              args[i].element (col % c, row % r): args[i];
        }

        // execute the function on each element
        res.setElement (col, row, exec (vals, calc, extra));
      }
    return res;
  }
  else
    // call the function
    return (*d->ptr) (args, calc, extra);
}


class FunctionRepositorySingleton
{
public:
    FunctionRepository instance;
};
K_GLOBAL_STATIC(FunctionRepositorySingleton, s_singleton)


FunctionRepository* FunctionRepository::self()
{
  if (!s_singleton->instance.d->initialized) {
      s_singleton->instance.d->initialized = true;
    kDebug() <<"Creating function repository ...";

    // register all existing functions
    FunctionModuleRegistry::instance()->registerFunctions();

    kDebug() << s_singleton->instance.d->functions.count() <<" functions registered.";

#ifndef NDEBUG
        // Verify, that every function has a description.
        QStringList missingDescriptions;
        typedef QHash<QString, Function*> Functions;
        Functions::ConstIterator end = s_singleton->instance.d->functions.constEnd();
        for ( Functions::ConstIterator it = s_singleton->instance.d->functions.constBegin(); it != end; ++it )
        {
            if ( !s_singleton->instance.d->descriptions.contains( it.key() ) )
                missingDescriptions << it.key();
        }
        if ( missingDescriptions.count() > 0 )
        {
            kDebug() <<"No function descriptions found for:";
            foreach( const QString& missingDescription, missingDescriptions )
                kDebug() <<"\t" << missingDescription;
        }
#endif
        kDebug() << s_singleton->instance.d->descriptions.count() <<" descriptions loaded.";
        kDebug() <<"Function repository ready.";
    }
    return &s_singleton->instance;
}

FunctionRepository::FunctionRepository()
    : d( new Private )
{
    d->initialized = false;
}

FunctionRepository::~FunctionRepository()
{
  qDeleteAll( d->functions );
  qDeleteAll( d->descriptions );
  delete d;
}

void FunctionRepository::add( Function* function )
{
  if( !function ) return;
  d->functions.insert( function->name().toUpper(), function );
}

void FunctionRepository::add( FunctionDescription *desc )
{
  if( !desc ) return;
  if( !d->functions.contains( desc->name() ) ) return;
  d->descriptions.insert (desc->name(), desc);
}

void FunctionRepository::remove(const QString& groupName)
{
    if (!d->groups.contains(groupName)) {
        return;
    }
    d->groups.removeAll(groupName);
    QStringList functionNames;
    foreach (FunctionDescription* description, d->descriptions) {
        if (description->group() == groupName) {
            functionNames.append(description->name());
        }
    }
    foreach (QString functionName, functionNames) {
        d->functions.remove(functionName);
        d->descriptions.remove(functionName);
    }
}

Function *FunctionRepository::function (const QString& name)
{
  return d->functions.value( name.toUpper() );
}

FunctionDescription *FunctionRepository::functionInfo (const QString& name)
{
  return d->descriptions.value( name.toUpper() );
}

// returns names of function in certain group
QStringList FunctionRepository::functionNames( const QString& group )
{
  QStringList lst;

  foreach ( FunctionDescription* description, d->descriptions )
  {
    if (group.isNull() || (description->group() == group))
      lst.append (description->name());
  }

  lst.sort();
  return lst;
}

const QStringList& FunctionRepository::groups () const
{
  return d->groups;
}

void FunctionRepository::addGroup(const QString& groupname)
{
  d->groups.append( groupname );
  d->groups.sort();
}

void FunctionRepository::loadFunctionDescriptions(const QString& filename)
{
  QFile file (filename);
  if (!file.open (QIODevice::ReadOnly))
    return;

  QDomDocument doc;
  doc.setContent( &file );
  file.close();

  QString group = "";

  QDomNode n = doc.documentElement().firstChild();
  for (; !n.isNull(); n = n.nextSibling())
  {
    if (!n.isElement())
      continue;
    QDomElement e = n.toElement();
    if (e.tagName() == "Group")
    {
      group = i18n (e.namedItem ("GroupName").toElement().text().toUtf8());
      addGroup(group);

      QDomNode n2 = e.firstChild();
      for (; !n2.isNull(); n2 = n2.nextSibling())
      {
        if (!n2.isElement())
          continue;
        QDomElement e2 = n2.toElement();
        if (e2.tagName() == "Function")
        {
          FunctionDescription* desc = new FunctionDescription( e2 );
          desc->setGroup (group);
          if ( d->functions.contains( desc->name() ) )
            d->descriptions.insert (desc->name(), desc);
          else
          {
            kDebug() <<"Description for unknown function" << desc->name() <<" found.";
            delete desc;
          }
        }
      }
      group = "";
    }
  }
}

// ------------------------------------------------------------

static ParameterType toType( const QString& type )
{
  if ( type == "Boolean" )
    return KSpread_Boolean;
  if ( type == "Int" )
    return KSpread_Int;
  if ( type == "String" )
    return KSpread_String;
  if ( type == "Any" )
    return KSpread_Any;

  return KSpread_Float;
}

static QString toString (ParameterType type, bool range = false)
{
  if ( !range )
  {
    switch(type) {
      case KSpread_String:
        return i18n("Text");
      case KSpread_Int:
        return i18n("Whole number (like 1, 132, 2344)");
      case KSpread_Boolean:
        return i18n("A truth value (TRUE or FALSE)" );
      case KSpread_Float:
        return i18n("A floating point value (like 1.3, 0.343, 253 )" );
      case KSpread_Any:
        return i18n("Any kind of value");
    }
  }
  else
  {
    switch(type) {
      case KSpread_String:
        return i18n("A range of strings");
      case KSpread_Int:
        return i18n("A range of whole numbers (like 1, 132, 2344)");
      case KSpread_Boolean:
        return i18n("A range of truth values (TRUE or FALSE)" );
      case KSpread_Float:
        return i18n("A range of floating point values (like 1.3, 0.343, 253 )" );
      case KSpread_Any:
        return i18n("A range of any kind of values");
    }
  }

  return QString();
}

FunctionParameter::FunctionParameter()
{
  m_type = KSpread_Float;
  m_range = false;
}

FunctionParameter::FunctionParameter (const FunctionParameter& param)
{
  m_help = param.m_help;
  m_type = param.m_type;
  m_range = param.m_range;
}

FunctionParameter::FunctionParameter (const QDomElement& element)
{
  m_type  = KSpread_Float;
  m_range = false;

  QDomNode n = element.firstChild();
  for( ; !n.isNull(); n = n.nextSibling() )
    if ( n.isElement() )
    {
      QDomElement e = n.toElement();
      if ( e.tagName() == "Comment" )
        m_help = i18n( e.text().toUtf8() );
      else if ( e.tagName() == "Type" )
      {
        m_type = toType( e.text() );
        if ( e.hasAttribute( "range" ))
        {
          if (e.attribute("range").toLower() == "true")
            m_range = true;
        }
      }
    }
}

FunctionDescription::FunctionDescription()
{
  m_type = KSpread_Float;
}

FunctionDescription::FunctionDescription (const QDomElement& element)
{
  QDomNode n = element.firstChild();
  for( ; !n.isNull(); n = n.nextSibling() )
  {
    if (!n.isElement())
      continue;
    QDomElement e = n.toElement();
    if ( e.tagName() == "Name" )
      m_name = e.text();
    else if ( e.tagName() == "Type" )
      m_type = toType( e.text() );
    else if ( e.tagName() == "Parameter" )
      m_params.append (FunctionParameter (e));
    else if ( e.tagName() == "Help" )
    {
      QDomNode n2 = e.firstChild();
      for( ; !n2.isNull(); n2 = n2.nextSibling() )
      {
        if (!n2.isElement())
          continue;
        QDomElement e2 = n2.toElement();
        if ( e2.tagName() == "Text" )
          m_help.append ( i18n( e2.text().toUtf8() ) );
        else if ( e2.tagName() == "Syntax" )
          m_syntax.append( i18n( e2.text().toUtf8() ) );
        else if ( e2.tagName() == "Example" )
          m_examples.append( i18n( e2.text().toUtf8() ) );
        else if ( e2.tagName() == "Related" )
          m_related.append( i18n( e2.text().toUtf8() ) );
      }
    }
  }
}

FunctionDescription::FunctionDescription( const FunctionDescription& desc )
{
  m_examples = desc.m_examples;
  m_related = desc.m_related;
  m_syntax = desc.m_syntax;
  m_help = desc.m_help;
  m_name = desc.m_name;
  m_type = desc.m_type;
}

QString FunctionDescription::toQML() const
{
  QString text( "<qt><h1>" );
  text += name();
  text += "</h1>";

  if( !m_help.isEmpty() )
  {
    text += "<p>";
    QStringList::ConstIterator it = m_help.begin();
    for( ; it != m_help.end(); ++it )
    {
      text += *it;
      text += "<p>";
    }
    text += "</p>";
  }

  text += i18n ("<p><b>Return type:</b> %1</p>", toString( type() ));

  if ( !m_syntax.isEmpty() )
  {
    text += "<h2>" + i18n("Syntax") + "</h2><ul>";
    QStringList::ConstIterator it = m_syntax.begin();
    for( ; it != m_syntax.end(); ++it )
    {
      text += "<li>";
      text += *it;
    }
    text += "</ul>";
  }

  if ( !m_params.isEmpty() )
  {
    text += "<h2>" + i18n("Parameters") + "</h2><ul>";
    QList<FunctionParameter>::ConstIterator it = m_params.begin();
    for( ; it != m_params.end(); ++it )
    {
      text += i18n ("<li><b>Comment:</b> %1", (*it).helpText());
      text += i18n ("<br><b>Type:</b> %1", toString( (*it).type(), (*it).hasRange() ));
    }
    text += "</ul>";
  }

  if ( !m_examples.isEmpty() )
  {
    text += "<h2>" + i18n("Examples") + "</h2><ul>";
    QStringList::ConstIterator it = m_examples.begin();
    for( ; it != m_examples.end(); ++it )
    {
      text += "<li>";
      text += *it;
    }
    text += "</ul>";
  }

  if ( !m_related.isEmpty() )
  {
    text += "<h2>" + i18n("Related Functions") + "</h2><ul>";
    QStringList::ConstIterator it = m_related.begin();
    for( ; it != m_related.end(); ++it )
    {
      text += "<li>";
      text += "<a href=\"" + *it + "\">";
      text += *it;
      text += "</a>";
    }
    text += "</ul>";
  }

  text += "</qt>";
  return text;
}
