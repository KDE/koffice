/* This file is part of the KDE project
   Copyright (C) 2002 Laurent MONTEL <lmontel@mandrakesoft.com>

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

#include "KWordFormulaFrameSetEditIface.h"

#include "kwframe.h"
#include "kwformulaframe.h"
#include "kwdoc.h"
#include <kapplication.h>
#include <dcopclient.h>
#include <kformulacontainer.h>
#include <kformuladocument.h>
#include <kformulaview.h>

KWordFormulaFrameSetEditIface::KWordFormulaFrameSetEditIface( KWFormulaFrameSetEdit *_framesetedit )
    :DCOPObject()
{
   m_framesetedit = _framesetedit;
}

void KWordFormulaFrameSetEditIface::copy()
{
    m_framesetedit->copy();
}

void KWordFormulaFrameSetEditIface::cut()
{
    m_framesetedit->cut();
}

void KWordFormulaFrameSetEditIface::paste()
{
    m_framesetedit->paste();
}

void KWordFormulaFrameSetEditIface::selectAll()
{
    m_framesetedit->selectAll();
}

void KWordFormulaFrameSetEditIface::addThinSpace()
{
   m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addThinSpace();
}

void KWordFormulaFrameSetEditIface::addMediumSpace()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addMediumSpace();
}

void KWordFormulaFrameSetEditIface::addThickSpace()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addThickSpace();
}

void KWordFormulaFrameSetEditIface::addQuadSpace()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addQuadSpace();
}

void KWordFormulaFrameSetEditIface::addBracket( int left, int right )
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()
        ->wrapper()->addBracket( static_cast<KFormula::SymbolType>( left ),
                                 static_cast<KFormula::SymbolType>( right ) );
}

void KWordFormulaFrameSetEditIface::addParenthesis()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addParenthesis();
}

void KWordFormulaFrameSetEditIface::addSquareBracket()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addSquareBracket();
}

void KWordFormulaFrameSetEditIface::addCurlyBracket()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addCurlyBracket();
}

void KWordFormulaFrameSetEditIface::addLineBracket()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addLineBracket();
}

void KWordFormulaFrameSetEditIface::addFraction()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addFraction();
}

void KWordFormulaFrameSetEditIface::addRoot()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addRoot();
}

void KWordFormulaFrameSetEditIface::addIntegral()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addIntegral();
}

void KWordFormulaFrameSetEditIface::addProduct()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addProduct();
}

void KWordFormulaFrameSetEditIface::addSum()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addSum();
}

void KWordFormulaFrameSetEditIface::addMatrix()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addMatrix();
}

void KWordFormulaFrameSetEditIface::addMatrix( uint rows, uint columns )
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addMatrix( rows, columns );
}

void KWordFormulaFrameSetEditIface::addOneByTwoMatrix()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addOneByTwoMatrix();
}

void KWordFormulaFrameSetEditIface::addNameSequence()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addNameSequence();
}

void KWordFormulaFrameSetEditIface::addLowerLeftIndex()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addLowerLeftIndex();
}

void KWordFormulaFrameSetEditIface::addUpperLeftIndex()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addUpperLeftIndex();
}

void KWordFormulaFrameSetEditIface::addLowerRightIndex()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addLowerRightIndex();
}

void KWordFormulaFrameSetEditIface::addUpperRightIndex()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addUpperRightIndex();
}

void KWordFormulaFrameSetEditIface::addGenericLowerIndex()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addGenericLowerIndex();
}

void KWordFormulaFrameSetEditIface::addGenericUpperIndex()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addGenericUpperIndex();
}

void KWordFormulaFrameSetEditIface::removeEnclosing()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->removeEnclosing();
}

void KWordFormulaFrameSetEditIface::makeGreek()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->makeGreek();
}

void KWordFormulaFrameSetEditIface::insertSymbol( const QString &name )
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->insertSymbol( name );
}

void KWordFormulaFrameSetEditIface::appendColumn()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->appendColumn();
}

void KWordFormulaFrameSetEditIface::insertColumn()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->insertColumn();
}

void KWordFormulaFrameSetEditIface::removeColumn()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->removeColumn();
}

void KWordFormulaFrameSetEditIface::appendRow()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->appendRow();
}

void KWordFormulaFrameSetEditIface::insertRow()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->insertRow();
}

void KWordFormulaFrameSetEditIface::removeRow()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->removeRow();
}

void KWordFormulaFrameSetEditIface::moveLeft( int flag )
{
    m_framesetedit->getFormulaView()->moveLeft( flag );
}

void KWordFormulaFrameSetEditIface::moveRight( int flag )
{
    m_framesetedit->getFormulaView()->moveRight( flag );
}

void KWordFormulaFrameSetEditIface::moveUp( int flag )
{
    m_framesetedit->getFormulaView()->moveUp( flag );
}

void KWordFormulaFrameSetEditIface::moveDown( int flag )
{
    m_framesetedit->getFormulaView()->moveDown( flag );
}

void KWordFormulaFrameSetEditIface::moveHome( int flag )
{
    m_framesetedit->getFormulaView()->moveHome( flag );
}

void KWordFormulaFrameSetEditIface::moveEnd( int flag )
{
    m_framesetedit->getFormulaView()->moveEnd( flag );
}

bool KWordFormulaFrameSetEditIface::isHome() const
{
    return m_framesetedit->getFormulaView()->isHome();
}

bool KWordFormulaFrameSetEditIface::isEnd() const
{
    return m_framesetedit->getFormulaView()->isEnd();
}

void KWordFormulaFrameSetEditIface::eraseSelection( int direction )
{
    m_framesetedit->getFormulaView()->eraseSelection( static_cast<KFormula::Direction>( direction ) );
}

void KWordFormulaFrameSetEditIface::addText( const QString &str )
{
    m_framesetedit->getFormulaView()->addText( str );
}

void KWordFormulaFrameSetEditIface::addNegThinSpace()
{
    m_framesetedit->formulaFrameSet()->kWordDocument()->getFormulaDocument()->wrapper()->addNegThinSpace();
}
