/* This file is part of the KDE project
   Copyright (C) 2002, Dirk Sch�nberger <dirk.schoenberger@sz-online.de>

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

#ifndef AIELEMENT_H
#define AIELEMENT_H

// #include <Q3ValueList>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3CString>
#include <Q3Shared>
class QString;
class Q3CString;

/**
  *@author 
  */

class AIElement {
public:
    enum Type {
	Invalid,
//	List,
	String,
	Int,
  UInt,
	Double,
	CString,
  // Custom Types
  Operator,
  Reference,
  ElementArray,
  Block,
  ByteArray,
  Byte
    };

    AIElement();
    ~AIElement();
    explicit AIElement( const AIElement& );
    explicit AIElement( const QString&, Type type = String );
    explicit AIElement( const Q3CString& );
    explicit AIElement( const char* );
//    AIElement( const QValueList<AIElement>& );
    explicit AIElement( const Q3ValueVector<AIElement>&, Type type = ElementArray);
    explicit AIElement( int );
    explicit AIElement( uint );
    explicit AIElement( double );
    explicit AIElement( const QByteArray& );
    explicit AIElement( uchar );

    AIElement& operator= ( const AIElement& );
    bool operator==( const AIElement& ) const;
    bool operator!=( const AIElement& ) const;

    Type type() const;
    const char* typeName() const;

    bool canCast( Type ) const;
    bool cast( Type );

    bool isValid() const;

    void clear();

    const QString toString() const;
    const Q3CString toCString() const;
    int toInt( bool * ok=0 ) const;
    uint toUInt( bool * ok=0 ) const;
    double toDouble( bool * ok=0 ) const;
//    const QValueList<AIElement> toList() const;
    const Q3ValueVector<AIElement> toElementArray() const;
    const Q3ValueVector<AIElement> toBlock() const;

    // Custom types
    const QString toReference() const;
    const QString toOperator() const;
    const QByteArray toByteArray() const;
    uchar toByte( bool * ok=0 ) const;

//    QValueListConstIterator<AIElement> listBegin() const;
//    QValueListConstIterator<AIElement> listEnd() const;
    QString& asString();
    Q3CString& asCString();
    int& asInt();
    uint& asUInt();
    double& asDouble();
//    QValueList<AIElement>& asList();
    Q3ValueVector<AIElement>& asElementArray();
    Q3ValueVector<AIElement>& asBlock();

    // Custom types
    QString& asReference();
    QString& asToken();
    QByteArray& asByteArray();
    uchar& asByte();

    static const char* typeToName( Type typ );
    static Type nameToType( const char* name );

private:
    void detach();

    class Private : public Q3Shared
    {
    public:
        Private();
        Private( Private* );
        ~Private();

        void clear();

        Type typ;
        union
        {
	    uint u;
	    int i;
	    double d;
      uchar b;
	    void *ptr;
        } value;
    };

    Private* d;
};

inline AIElement::Type AIElement::type() const
{
    return d->typ;
}

inline bool AIElement::isValid() const
{
    return (d->typ != Invalid);
}

#endif
