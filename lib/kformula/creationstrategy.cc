/* This file is part of the KDE project
   Copyright (C) 2003 Ulrich Kuettler <ulrich.kuettler@gmx.de>

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

#include "bracketelement.h"
#include "creationstrategy.h"
#include "elementtype.h"
#include "fractionelement.h"
#include "indexelement.h"
#include "matrixelement.h"
#include "rootelement.h"
#include "sequenceelement.h"
#include "spaceelement.h"
#include "symbolelement.h"
#include "textelement.h"

KFORMULA_NAMESPACE_BEGIN

BasicElement* OrdinaryCreationStrategy::createElement( QString type )
{
    if      ( type == "TEXT" )         return new TextElement();
    else if ( type == "EMPTY" )        return new EmptyElement();
    else if ( type == "SPACE" )        return new SpaceElement();
    else if ( type == "ROOT" )         return new RootElement();
    else if ( type == "BRACKET" )      return new BracketElement();
    else if ( type == "MATRIX" )       return new MatrixElement();
    else if ( type == "INDEX" )        return new IndexElement();
    else if ( type == "FRACTION" )     return new FractionElement();
    else if ( type == "SYMBOL" )       return new SymbolElement();
    else if ( type == "NAMESEQUENCE" ) return new NameSequence();
    else if ( type == "OVERLINE" )     return new OverlineElement();
    else if ( type == "UNDERLINE" )    return new UnderlineElement();
    else if ( type == "MULTILINE" )    return new MultilineElement();
    else if ( type == "SEQUENCE" ) {
        kdWarning() << "malformed data: sequence inside sequence." << endl;
        return 0;
    }
    return 0;
}


TextElement* OrdinaryCreationStrategy::createTextElement( const QChar& ch, bool symbol )
{
    return new TextElement( ch, symbol );
}

EmptyElement* OrdinaryCreationStrategy::createEmptyElement()
{
    return new EmptyElement;
}

NameSequence* OrdinaryCreationStrategy::createNameSequence()
{
    return new NameSequence;
}

BracketElement* OrdinaryCreationStrategy::createBracketElement( SymbolType lhs, SymbolType rhs )
{
    return new BracketElement( lhs, rhs );
}

OverlineElement* OrdinaryCreationStrategy::createOverlineElement()
{
    return new OverlineElement;
}

UnderlineElement* OrdinaryCreationStrategy::createUnderlineElement()
{
    return new UnderlineElement;
}

MultilineElement* OrdinaryCreationStrategy::createMultilineElement()
{
    return new MultilineElement;
}

SpaceElement* OrdinaryCreationStrategy::createSpaceElement( SpaceWidth width )
{
    return new SpaceElement( width );
}

FractionElement* OrdinaryCreationStrategy::createFractionElement()
{
    return new FractionElement;
}

RootElement* OrdinaryCreationStrategy::createRootElement()
{
    return new RootElement;
}

SymbolElement* OrdinaryCreationStrategy::createSymbolElement( SymbolType type )
{
    return new SymbolElement( type );
}

MatrixElement* OrdinaryCreationStrategy::createMatrixElement( uint rows, uint columns )
{
    return new MatrixElement( rows, columns );
}

IndexElement* OrdinaryCreationStrategy::createIndexElement()
{
    return new IndexElement;
}


KFORMULA_NAMESPACE_END
