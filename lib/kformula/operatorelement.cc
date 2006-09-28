/* This file is part of the KDE project
   Copyright (C) 2006 Alfredo Beaumont Sainz <alfredo.beaumont@gmail.com>

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

#include <algorithm>

#include <qpainter.h>

#include <klocale.h>

#include "elementtype.h"
#include "sequenceelement.h"
#include "textelement.h"
#include "fontstyle.h"
#include "operatordictionary.h"
#include "operatorelement.h"
#include "identifierelement.h"
#include "numberelement.h"
#include "kformulacommand.h"
#include "kformulacontainer.h"
#include "kformuladocument.h"
#include "formulaelement.h"
#include "creationstrategy.h"

KFORMULA_NAMESPACE_BEGIN

OperatorElement::OperatorElement( BasicElement* parent ) : TokenElement( parent ),
                                                           m_form( NoForm ),
                                                           m_lspaceType( ThickMathSpace ),
                                                           m_rspaceType( ThickMathSpace ),
                                                           m_maxSizeType( InfinitySize ),
                                                           m_minSizeType( RelativeSize ),
                                                           m_minSize( 1 ),
                                                           m_fence( false ),
                                                           m_separator( false ),
                                                           m_stretchy( false ),
                                                           m_symmetric( true ),
                                                           m_largeOp( false ),
                                                           m_movableLimits( false ),
                                                           m_accent( false ),
                                                           m_customForm( false ),
                                                           m_customFence( false ),
                                                           m_customSeparator( false ),
                                                           m_customLSpace( false ),
                                                           m_customRSpace( false ),
                                                           m_customStretchy( false ),
                                                           m_customSymmetric( false ),
                                                           m_customMaxSize( false ),
                                                           m_customMinSize( false ),
                                                           m_customLargeOp( false ),
                                                           m_customMovableLimits( false ),
                                                           m_customAccent( false )
{
}

void OperatorElement::setForm( FormType type )
{
    if ( ! m_customForm ) { // Set by an attribute has higher priority
        m_form = type;
    }
    
    if ( ! isTextOnly() ) { // Only text content can be dictionary keys
        return;
    }
    QString text;
    for ( uint i = 0; i < countChildren(); i++ ) {
        text.append( getChild( i )->getCharacter() );
    }
    QString form;
    switch ( m_form ) {
    case PrefixForm:
        form = "prefix";
        break;
    case InfixForm:
        form = "infix";
        break;
    case PostfixForm:
        form = "postfix";
        break;
    default:
        // Should not happen
        kdWarning( DEBUGID ) << "Invalid `form' attribute value\n";
        return;
    }
    DictionaryKey key = { text.utf8(), form.ascii() };
    const OperatorDictionary* begin = operators;
    const OperatorDictionary* end = operators + OperatorDictionary::size();
    const OperatorDictionary* pos = std::lower_bound( begin, end, key );
    if ( pos != end && pos->key == key ) { // Entry found !
        if ( ! m_customFence ) {
            m_fence = pos->fence;
        }
        if ( ! m_customSeparator ) {
            m_separator = pos->separator;
        }
        if ( ! m_customLSpace ) {
            m_lspace = getSize( pos->lspace, &m_lspaceType );
            if ( m_lspaceType == NoSize ) {
                m_lspaceType = getSpace( pos->lspace );
            }
        }
        if ( ! m_customRSpace ) {
            m_rspace = getSize( pos->rspace, &m_rspaceType );
            if ( m_rspaceType == NoSize ) {
                m_rspaceType = getSpace( pos->rspace );
            }
        }
        if ( ! m_customStretchy ) {
            m_stretchy = pos->stretchy;
        }
        if ( ! m_customSymmetric ) {
            m_symmetric = pos->symmetric;
        }
        if ( ! m_customMaxSize ) {
            if ( qstrcmp( pos->maxsize, "infinity" ) == 0 ) {
                m_maxSizeType = InfinitySize;
            }
            else {
                m_maxSize = getSize( pos->maxsize, &m_maxSizeType );
                if ( m_maxSizeType == NoSize ) {
                    m_maxSizeType = getSpace( pos->maxsize );
                }
            }
        }
        if ( ! m_customMinSize ) {
            m_minSize = getSize( pos->minsize, &m_minSizeType );
            if ( m_minSizeType == NoSize ) {
                m_minSizeType = getSpace( pos->minsize );
            }
        }
        if ( ! m_customLargeOp ) {
            m_largeOp = pos->largeop;
        }
        if ( ! m_customMovableLimits ) {
            m_movableLimits = pos->movablelimits;
        }
        if ( ! m_customAccent ) {
            m_accent = pos->accent;
        }
    }
}

/*
 * Token elements' content has to be of homogeneous type. Every token element
 * must (TODO: check this) appear inside a non-token sequence, and thus, if
 * the command asks for a different content, a new element has to be created in
 * parent sequence.
 */
KCommand* OperatorElement::buildCommand( Container* container, Request* request )
{
    FormulaCursor* cursor = container->activeCursor();
    if ( cursor->isReadOnly() ) {
        formula()->tell( i18n( "write protection" ) );
        return 0;
    }

    if ( *request == req_addOperator ) {
        KFCReplace* command = new KFCReplace( i18n("Add Operator"), container );
        OperatorRequest* opr = static_cast<OperatorRequest*>( request );
        TextElement* element = creationStrategy->createTextElement( opr->ch(), true );
        command->addElement( element );
        return command;
    }

    if ( countChildren() == 0 || cursor->getPos() == countChildren() ) {
        // We are in the last position, so it's easy, call the parent to 
        // create a new child
        SequenceElement* parent = static_cast<SequenceElement*>( getParent() );
        if ( parent ) {
            uint pos = parent->childPos( this );
            cursor->setTo( parent, pos + 1);
            return parent->buildCommand( container, request );
        }
    }
    if ( cursor->getPos() == 0 ) {
        SequenceElement* parent = static_cast<SequenceElement*>( getParent() );
        if ( parent ) {
            uint pos = parent->childPos( this );
            cursor->setTo( parent, pos );
            return parent->buildCommand( container, request );
        }
    }

    // We are in the middle of a token, so:
    // a) Cut from mark to the end
    // b) Create a new token and add an element from key pressed
    // c) Create a new token and add elements cut previously
    // d) Move cursor to parent so that it command execution works fine

    switch( *request ) {
    case req_addTextChar: {
        KFCSplitToken* command = new KFCSplitToken( i18n("Add Text"), container );
        TextCharRequest* tr = static_cast<TextCharRequest*>( request );
        IdentifierElement* id = creationStrategy->createIdentifierElement();
        TextElement* text = creationStrategy->createTextElement( tr->ch() );
        command->addCursor( cursor );
        command->addToken( id );
        command->addContent( id, text );
        SequenceElement* parent = static_cast< SequenceElement* >( getParent() );
        if ( parent ) {
            cursor->setTo( parent, parent->childPos( this ) + 1 );
        }
        return command;
    }

    case req_addText: {
        KFCSplitToken* command = new KFCSplitToken( i18n("Add Text"), container );
        TextRequest* tr = static_cast<TextRequest*>( request );
        IdentifierElement* id = creationStrategy->createIdentifierElement();
        command->addCursor( cursor );
        command->addToken( id );
        for ( uint i = 0; i < tr->text().length(); i++ ) {
            TextElement* text = creationStrategy->createTextElement( tr->text()[i] );
            command->addContent( id, text );
        }
        SequenceElement* parent = static_cast< SequenceElement* >( getParent() );
        if ( parent ) {
            cursor->setTo( parent, parent->childPos( this ) + 1 );
        }
        return command;
    }

    case req_addNumber: {
        KFCSplitToken* command = new KFCSplitToken( i18n("Add Number"), container );
        NumberRequest* nr = static_cast<NumberRequest*>( request );
        NumberElement* num = creationStrategy->createNumberElement();
        TextElement* text = creationStrategy->createTextElement( nr->ch() );
        command->addCursor( cursor );
        command->addToken( num );
        command->addContent( num, text );
        SequenceElement* parent = static_cast< SequenceElement* >( getParent() );
        if ( parent ) {
            cursor->setTo( parent, parent->childPos( this ) + 1 );
        }
        return command;
    }
    case req_addEmptyBox:
    case req_addNameSequence:
    case req_addBracket:
    case req_addSpace:
    case req_addFraction:
    case req_addRoot:
    case req_addSymbol:
    case req_addOneByTwoMatrix:
    case req_addMatrix: {
        uint pos = static_cast<SequenceElement*>(getParent())->childPos( this );
        cursor->setTo( getParent(), pos + 1);
        return getParent()->buildCommand( container, request );
    }
    default:
        return SequenceElement::buildCommand( container, request );
    }
    return 0;
}


bool OperatorElement::readAttributesFromMathMLDom( const QDomElement &element )
{
    if ( ! BasicElement::readAttributesFromMathMLDom( element ) ) {
        return false;
    }

    QString formStr = element.attribute( "form" ).stripWhiteSpace().lower();
    if ( ! formStr.isNull() ) {
        m_customForm = true;
        if ( formStr == "prefix" ) {
            m_form = PrefixForm;
        }
        else if ( formStr == "infix" ) {
            m_form = InfixForm;
        }
        else if ( formStr == "postfix" ) {
            m_form = PostfixForm;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `form': " << formStr << endl;
            m_customForm = false;
        }
    }
    QString fenceStr = element.attribute( "fence" ).stripWhiteSpace().lower();
    if ( ! fenceStr.isNull() ) {
        m_customFence = true;
        if ( fenceStr == "true" ) {
            m_fence = true;
        }
        else if ( fenceStr == "false" ) {
            m_fence = false;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `fence': " << fenceStr << endl;
            m_customFence = false;
        }
    }
    QString separatorStr = element.attribute( "separator" ).stripWhiteSpace().lower();
    if ( ! separatorStr.isNull() ) {
        m_customSeparator = true;
        if ( separatorStr == "true" ) {
            m_separator = true;
        }
        else if ( separatorStr == "false" ) {
            m_separator = false;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `separator': " << separatorStr << endl;
            m_customSeparator = false;
        }
    }
    QString lspaceStr = element.attribute( "lspace" ).stripWhiteSpace().lower();
    if ( ! lspaceStr.isNull() ) {
        m_customLSpace = true;
        m_lspace = getSize( lspaceStr, &m_lspaceType );
        if ( m_lspaceType == NoSize ) {
            m_lspaceType = getSpace( lspaceStr );
        }
    }
    QString rspaceStr = element.attribute( "rspace" ).stripWhiteSpace().lower();
    if ( ! rspaceStr.isNull() ) {
        m_customRSpace = true;
        m_rspace = getSize( rspaceStr, &m_rspaceType );
        if ( m_rspaceType == NoSize ) {
            m_rspaceType = getSpace( rspaceStr );
        }
    }
    QString stretchyStr = element.attribute( "stretchy" ).stripWhiteSpace().lower();
    if ( ! stretchyStr.isNull() ) {
        m_customStretchy = true;
        if ( stretchyStr == "true" ) {
            m_stretchy = true;
        }
        else if ( stretchyStr == "false" ) {
            m_stretchy = false;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `stretchy': " << stretchyStr << endl;
            m_customStretchy = false;
        }
    }
    QString symmetricStr = element.attribute( "symmetric" ).stripWhiteSpace().lower();
    if ( ! symmetricStr.isNull() ) {
        m_customSymmetric = true;
        if ( symmetricStr == "true" ) {
            m_symmetric = true;
        }
        else if ( symmetricStr == "false" ) {
            m_symmetric = false;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `symmetric': " << symmetricStr << endl;
            m_customSymmetric = false;
        }
    }
    QString maxsizeStr = element.attribute( "maxsize" ).stripWhiteSpace().lower();
    if ( ! maxsizeStr.isNull() ) {
        m_customMaxSize = true;
        if ( maxsizeStr == "infinity" ) {
            m_maxSizeType = InfinitySize;
        }
        else {
            m_maxSize = getSize( maxsizeStr, &m_maxSizeType );
            if ( m_maxSizeType == NoSize ) {
                m_maxSizeType = getSpace( maxsizeStr );
            }
        }
    }
    QString minsizeStr = element.attribute( "minsize" ).stripWhiteSpace().lower();
    if ( ! minsizeStr.isNull() ) {
        m_customMinSize = true;
        m_minSize = getSize( minsizeStr, &m_minSizeType );
        if ( m_minSizeType == NoSize ) {
            m_minSizeType = getSpace( minsizeStr );
        }
    }
    QString largeopStr = element.attribute( "largeop" ).stripWhiteSpace().lower();
    if ( ! largeopStr.isNull() ) {
        m_customLargeOp = true;
        if ( largeopStr == "true" ) {
            m_largeOp = true;
        }
        else if ( largeopStr == "false" ) {
            m_largeOp = false;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `largeop': " << largeopStr << endl;
            m_customLargeOp = false;
        }
    }
    QString movablelimitsStr = element.attribute( "movablelimits" ).stripWhiteSpace().lower();
    if ( ! movablelimitsStr.isNull() ) {
        m_customMovableLimits = true;
        if ( movablelimitsStr == "true" ) {
            m_movableLimits = true;
        }
        else if ( movablelimitsStr == "false" ) {
            m_movableLimits = false;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `movablelimits': " << movablelimitsStr << endl;
            m_customMovableLimits = false;
        }
    }
    QString accentStr = element.attribute( "accent" ).stripWhiteSpace().lower();
    if ( ! accentStr.isNull() ) {
        m_customAccent = true;
        if ( accentStr == "true" ) {
            m_accent = true;
        }
        else if ( accentStr == "false" ) {
            m_accent = false;
        }
        else {
            kdWarning( DEBUGID ) << "Invalid value for attribute `accent': " << accentStr << endl;
            m_customAccent = false;
        }
    }
    return true;
}

void OperatorElement::writeMathMLAttributes( QDomElement& element ) const
{
    if ( m_customForm ) {
        switch ( m_form ) {
        case PrefixForm:
            element.setAttribute( "form", "prefix" );
            break;
        case InfixForm:
            element.setAttribute( "form", "infix" );
            break;
        case PostfixForm:
            element.setAttribute( "form", "postfix" );
        default:
            break;
        }
    }
    if ( m_customFence ) {
        element.setAttribute( "fence", m_fence ? "true" : "false" );
    }
    if ( m_customSeparator ) {
        element.setAttribute( "separator", m_separator ? "true" : "false" );
    }
    if ( m_customLSpace ) {
        writeSizeAttribute( element, "lspace", m_lspaceType, m_lspace );
    }
    if ( m_customRSpace ) {
        writeSizeAttribute( element, "rspace", m_rspaceType, m_rspace );
    }
    if ( m_customStretchy ) {
        element.setAttribute( "stretchy", m_stretchy ? "true" : "false" );
    }
    if ( m_customSymmetric ) {
        element.setAttribute( "symmetric", m_symmetric ? "true" : "false" );
    }
    if ( m_customMaxSize ) {
        writeSizeAttribute( element, "maxsize", m_maxSizeType, m_maxSize );
    }
    if ( m_customMinSize ) {
        writeSizeAttribute( element, "minsize", m_minSizeType, m_minSize );
    }
    if ( m_customLargeOp ) {
        element.setAttribute( "largeop", m_largeOp ? "true" : "false" );
    }
    if ( m_customMovableLimits ) {
        element.setAttribute( "movablelimits", m_movableLimits ? "true" : "false" );
    }
    if ( m_customAccent ) {
        element.setAttribute( "accent", m_accent ? "true" : "false" );
    }
}

void OperatorElement::writeSizeAttribute( QDomElement& element, const QString &attr, SizeType type, double length ) const
{
    switch ( type ) {
    case InfinitySize:
        element.setAttribute( attr, "infinity" );
        break;
    case AbsoluteSize:
        element.setAttribute( attr, QString( "%1pt" ).arg( length ) );
        break;
    case RelativeSize:
        element.setAttribute( attr, QString( "%1% " ).arg( length * 100.0 ) );
        break;
    case PixelSize:
        element.setAttribute( attr, QString( "%1px " ).arg( length ) );
        break;
    case NegativeVeryVeryThinMathSpace:
        element.setAttribute( attr, "negativeveryverythinmathspace" );
        break;
    case NegativeVeryThinMathSpace:
        element.setAttribute( attr, "negativeverythinmathspace" );
        break;
    case NegativeThinMathSpace:
        element.setAttribute( attr, "negativethinmathspace" );
        break;
    case NegativeMediumMathSpace:
        element.setAttribute( attr, "negativemediummathspace" );
        break;
    case NegativeThickMathSpace:
        element.setAttribute( attr, "negativethickmathspace" );
        break;
    case NegativeVeryThickMathSpace:
        element.setAttribute( attr, "negativeverythickmathspace" );
        break;
    case NegativeVeryVeryThickMathSpace:
        element.setAttribute( attr, "negativeveryverythickmathspace" );
        break;
    case VeryVeryThinMathSpace:
        element.setAttribute( attr, "veryverythinmathspace" );
        break;
    case VeryThinMathSpace:
        element.setAttribute( attr, "verythinmathspace" );
        break;
    case ThinMathSpace:
        element.setAttribute( attr, "thinmathspace" );
        break;
    case MediumMathSpace:
        element.setAttribute( attr, "mediummathspace" );
        break;
    case ThickMathSpace:
        element.setAttribute( attr, "thickmathspace" );
        break;
    case VeryThickMathSpace:
        element.setAttribute( attr, "verythickmathspace" );
        break;
    case VeryVeryThickMathSpace:
        element.setAttribute( attr, "veryverythickmathspace" );
        break;
    default:
        break;
    }
}


KFORMULA_NAMESPACE_END
