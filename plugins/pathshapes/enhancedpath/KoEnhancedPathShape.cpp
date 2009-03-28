/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoEnhancedPathShape.h"
#include "KoEnhancedPathCommand.h"
#include "KoEnhancedPathParameter.h"
#include "KoEnhancedPathHandle.h"
#include "KoEnhancedPathFormula.h"

#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoShapeSavingContext.h>
#include <KoUnit.h>
#include <KoOdfWorkaround.h>

KoEnhancedPathShape::KoEnhancedPathShape( const QRectF &viewBox )
: m_viewBox( viewBox ), m_viewBoxOffset( 0.0, 0.0 )
{
}

KoEnhancedPathShape::~KoEnhancedPathShape()
{
    reset();
}

void KoEnhancedPathShape::reset()
{
    qDeleteAll( m_commands );
    m_commands.clear();
    qDeleteAll( m_enhancedHandles );
    m_enhancedHandles.clear();
    m_handles.clear();
    qDeleteAll( m_formulae );
    m_formulae.clear();
    qDeleteAll( m_parameters );
    m_parameters.clear();
    m_modifiers.clear();
    m_viewMatrix.reset();
    m_viewBoxOffset = QPointF();
    clear();
}

void KoEnhancedPathShape::moveHandleAction( int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers )
{
    Q_UNUSED( modifiers );
    KoEnhancedPathHandle *handle = m_enhancedHandles[ handleId ];
    if( handle )
    {
        handle->changePosition( shapeToViewbox( point ) );
        evaluateHandles();
    }
}

void KoEnhancedPathShape::updatePath( const QSizeF & )
{
    clear();

    foreach( KoEnhancedPathCommand *cmd, m_commands )
        cmd->execute();

    normalize();
}

void KoEnhancedPathShape::setSize( const QSizeF &newSize )
{
    QMatrix matrix( resizeMatrix( newSize ) );

    KoParameterShape::setSize( newSize );

    qreal scaleX = matrix.m11();
    qreal scaleY = matrix.m22();
    m_viewBoxOffset.rx() *= scaleX;
    m_viewBoxOffset.ry() *= scaleY;
    m_viewMatrix.scale( scaleX, scaleY );
}


QPointF KoEnhancedPathShape::normalize()
{
    QPointF offset = KoPathShape::normalize();
    m_viewBoxOffset -= offset;

    return offset;
}

void KoEnhancedPathShape::evaluateHandles()
{
    if( m_handles.size() != m_enhancedHandles.size() )
    {
        m_handles.clear();
        uint handleCount = m_enhancedHandles.size();
        for( uint i = 0; i < handleCount; ++i )
            m_handles.append( viewboxToShape( m_enhancedHandles[i]->position() ) );
    }
    else
    {
        uint handleCount = m_enhancedHandles.size();
        for( uint i = 0; i < handleCount; ++i )
            m_handles[i] = viewboxToShape( m_enhancedHandles[i]->position() );
    }
}

qreal KoEnhancedPathShape::evaluateReference( const QString &reference )
{
    if( reference.isEmpty() )
        return 0.0;

    QChar c = reference[0];

    qreal res = 0.0;

    switch( c.toAscii() )
    {
        // referenced modifier
        case '$':
        {
            bool success = false;
            int modifierIndex = reference.mid( 1 ).toInt( &success );
            res = m_modifiers[modifierIndex];
        }
        break;
        // referenced formula
        case '?':
        {
            QString fname = reference.mid( 1 );
            FormulaStore::const_iterator formulaIt = m_formulae.constFind( fname );
            if( formulaIt != m_formulae.constEnd() )
            {
                KoEnhancedPathFormula * formula = formulaIt.value();
                if( formula )
                    res = formula->evaluate();
            }
        }
        break;
        // maybe an identifier ?
        default:
            KoEnhancedPathNamedParameter p( reference, this );
            res = p.evaluate();
        break;
    }

    return res;
}

void KoEnhancedPathShape::modifyReference( const QString &reference, qreal value )
{
    if( reference.isEmpty() )
        return;

    QChar c = reference[0];

    if( c.toAscii() == '$' )
    {
        bool success = false;
        int modifierIndex = reference.mid( 1 ).toInt( &success );
        if( modifierIndex >= 0 && modifierIndex < m_modifiers.count() )
            m_modifiers[modifierIndex] = value;
    }
}

KoEnhancedPathParameter * KoEnhancedPathShape::parameter( const QString & text )
{
    Q_ASSERT( ! text.isEmpty() );

    ParameterStore::const_iterator parameterIt = m_parameters.constFind( text );
    if( parameterIt != m_parameters.constEnd() )
        return parameterIt.value();
    else
    {
        KoEnhancedPathParameter *parameter = 0;
        QChar c = text[0];
        if( c.toAscii() == '$' || c.toAscii() == '?' )
            parameter = new KoEnhancedPathReferenceParameter( text, this );
        else
        {
            if( c.isDigit() )
            {
                bool success = false;
                qreal constant = text.toDouble( &success );
                if( success )
                    parameter = new KoEnhancedPathConstantParameter( constant, this );
            }
            else
            {
                Identifier identifier = KoEnhancedPathNamedParameter::identifierFromString( text );
                if( identifier != IdentifierUnknown )
                    parameter = new KoEnhancedPathNamedParameter( identifier, this );
            }
        }

        if( parameter )
            m_parameters[text] = parameter;

        return parameter;
    }
}

void KoEnhancedPathShape::addFormula( const QString &name, const QString &formula )
{
    if( name.isEmpty() || formula.isEmpty() )
        return;

    m_formulae[name] = new KoEnhancedPathFormula( formula, this );
}

void KoEnhancedPathShape::addHandle( const QMap<QString,QVariant> &handle )
{
    if( handle.isEmpty() )
        return;

    if( ! handle.contains( "draw:handle-position" ) )
        return;
    QVariant position = handle.value("draw:handle-position");

    QStringList tokens = position.toString().simplified().split( ' ' );
    if( tokens.count() < 2 )
        return;

    KoEnhancedPathHandle *newHandle = new KoEnhancedPathHandle( this );
    newHandle->setPosition( parameter( tokens[0] ), parameter( tokens[1] ) );

    // check if we have a polar handle
    if( handle.contains( "draw:handle-polar" ) )
    {
        QVariant polar = handle.value( "draw:handle-polar" );
        QStringList tokens = polar.toString().simplified().split( ' ' );
        if( tokens.count() == 2 )
        {
            newHandle->setPolarCenter( parameter( tokens[0] ), parameter( tokens[1] ) );

            QVariant minRadius = handle.value( "draw:handle-radius-range-minimum" );
            QVariant maxRadius = handle.value( "draw:handle-radius-range-maximum" );
            if( minRadius.isValid() && maxRadius.isValid() )
                newHandle->setRadiusRange( parameter( minRadius.toString() ), parameter( maxRadius.toString() ) );
        }
    }
    else
    {
        QVariant minX = handle.value( "draw:handle-range-x-minimum" );
        QVariant maxX = handle.value( "draw:handle-range-x-maximum" );
        if( minX.isValid() && maxX.isValid() )
            newHandle->setRangeX( parameter( minX.toString() ), parameter( maxX.toString() ) );

        QVariant minY = handle.value( "draw:handle-range-y-minimum" );
        QVariant maxY = handle.value( "draw:handle-range-y-maximum" );
        if( minY.isValid() && maxY.isValid() )
            newHandle->setRangeY( parameter( minY.toString() ), parameter( maxY.toString() ) );
    }

    m_enhancedHandles.append( newHandle );

    evaluateHandles();
}

void KoEnhancedPathShape::addModifiers( const QString &modifiers )
{
    if( modifiers.isEmpty() )
        return;

    QStringList tokens = modifiers.simplified().split( ' ' );
    int tokenCount = tokens.count();
    for( int i = 0; i < tokenCount; ++i )
       m_modifiers.append( tokens[i].toDouble() );
}

void KoEnhancedPathShape::addCommand( const QString &command )
{
    addCommand( command, true );
}

void KoEnhancedPathShape::addCommand( const QString &command, bool triggerUpdate )
{
    if( command.isEmpty() )
        return;

    QString commandStr = command.simplified();
    if( commandStr.isEmpty() )
        return;

    // the first character is the command
    KoEnhancedPathCommand * cmd = new KoEnhancedPathCommand( commandStr[0], this );

    // strip command char
    commandStr = commandStr.mid( 1 );

    // now parse the command parameters
    if( commandStr.length() > 0 )
    {
        QStringList tokens = commandStr.simplified().split( ' ' );
        int tokenCount = tokens.count();
        for( int i = 0; i < tokenCount; ++i )
            cmd->addParameter( parameter( tokens[i] ) );
    }
    m_commands.append( cmd );

    if( triggerUpdate )
        updatePath( size() );
}

const QRectF & KoEnhancedPathShape::viewBox() const
{
    return m_viewBox;
}

QPointF KoEnhancedPathShape::shapeToViewbox( const QPointF & point ) const
{
    return m_viewMatrix.inverted().map( point-m_viewBoxOffset );
}

QPointF KoEnhancedPathShape::viewboxToShape( const QPointF & point ) const
{
    return m_viewMatrix.map( point ) + m_viewBoxOffset;
}

qreal KoEnhancedPathShape::shapeToViewbox( qreal value ) const
{
    return m_viewMatrix.inverted().map( QPointF( value, value ) ).x();
}

qreal KoEnhancedPathShape::viewboxToShape( qreal value ) const
{
    return m_viewMatrix.map( QPointF( value, value ) ).x();
}

void KoEnhancedPathShape::saveOdf( KoShapeSavingContext & context ) const
{
    if( isParametricShape() )
    {
        context.xmlWriter().startElement("draw:custom-shape");
        saveOdfAttributes( context, OdfAllAttributes );

        context.xmlWriter().startElement("draw:enhanced-geometry");
        context.xmlWriter().addAttribute("svg:viewBox", QString("%1 %2 %3 %4").arg( m_viewBox.x() ).arg( m_viewBox.y() ).arg( m_viewBox.width() ).arg( m_viewBox.height() ) );

        QString modifiers;
        foreach( qreal modifier, m_modifiers )
            modifiers += QString::number( modifier ) + ' ';
        context.xmlWriter().addAttribute("draw:modifiers", modifiers.trimmed() );

        QString path;
        foreach( KoEnhancedPathCommand * c, m_commands )
            path += c->toString() + ' ';
        context.xmlWriter().addAttribute("draw:enhanced-path", path.trimmed() );

        FormulaStore::const_iterator i = m_formulae.constBegin();
        for( ; i != m_formulae.constEnd(); ++i )
        {
            context.xmlWriter().startElement("draw:equation");
            context.xmlWriter().addAttribute("draw:name", i.key() );
            context.xmlWriter().addAttribute("draw:formula", i.value()->toString() );
            context.xmlWriter().endElement(); // draw:equation
        }

        foreach( KoEnhancedPathHandle * handle, m_enhancedHandles )
            handle->saveOdf( context );

        context.xmlWriter().endElement(); // draw:enhanced-geometry
        saveOdfCommonChildElements( context );
        context.xmlWriter().endElement(); // draw:custom-shape
    }
    else
        KoPathShape::saveOdf( context );
}

bool KoEnhancedPathShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    reset();

    KoXmlElement child;
    forEachElement( child, element )
    {
        if( child.localName() == "enhanced-geometry" && child.namespaceURI() == KoXmlNS::draw )
        {
            // load the viewbox
            QRectF viewBox = loadOdfViewbox( child );
            if( ! viewBox.isEmpty() )
                m_viewBox = viewBox;

            // load the modifiers
            QString modifiers = child.attributeNS( KoXmlNS::draw, "modifiers", "" );
            if( ! modifiers.isEmpty() )
            {
                addModifiers( modifiers );
            }

            KoXmlElement grandChild;
            forEachElement( grandChild, child )
            {
                if( grandChild.namespaceURI() != KoXmlNS::draw )
                    continue;
                if( grandChild.localName() == "equation" )
                {
                    QString name = grandChild.attributeNS( KoXmlNS::draw, "name" );
                    QString formula = grandChild.attributeNS( KoXmlNS::draw, "formula" );
                    addFormula( name, formula );
                }
                else if( grandChild.localName() == "handle" )
                {
                    KoEnhancedPathHandle * handle = new KoEnhancedPathHandle( this );
                    if( handle->loadOdf( grandChild ) )
                    {
                        m_enhancedHandles.append( handle );
                        evaluateHandles();
                    }
                    else
                        delete handle;
                }

            }
            // load the enhanced path data
            QString path = child.attributeNS( KoXmlNS::draw, "enhanced-path", "" );
#ifndef NWORKAROUND_ODF_BUGS
            KoOdfWorkaround::fixEnhancedPath(path, child, context);
#endif
            if ( !path.isEmpty() ) {
                parsePathData( path );
            }
        }
    }

    QPointF pos;
    pos.setX( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "x", QString() ) ) );
    pos.setY( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "y", QString() ) ) );
    setPosition( pos );
    normalize();

    QSizeF size;
    size.setWidth( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "width", QString() ) ) );
    size.setHeight( KoUnit::parseValue( element.attributeNS( KoXmlNS::svg, "height", QString() ) ) );

    setSize( size );

    loadOdfAttributes( element, context, OdfMandatories | OdfTransformation | OdfAdditionalAttributes | OdfCommonChildElements );

    return true;
}

void KoEnhancedPathShape::parsePathData( const QString & data )
{
    if( data.isEmpty() )
        return;

    QString d = data;
    d = d.replace( ',', ' ' );
    d = d.simplified();

    const QByteArray buffer = d.toLatin1();
    const char *ptr = buffer.constData();
    const char *end = buffer.constData() + buffer.length();

    char lastChar = ' ';

    QString cmdString;

    for( ; ptr < end; ptr++ )
    {
        switch( *ptr )
        {
            case 'M':
            case 'L':
            case 'C':
            case 'Z':
            case 'N':
            case 'F':
            case 'S':
            case 'T':
            case 'U':
            case 'A':
            case 'B':
            case 'W':
            case 'V':
            case 'X':
            case 'Y':
            case 'Q':
                if( lastChar == ' ' || QChar(lastChar).isNumber() )
                {
                    if( ! cmdString.isEmpty() )
                        addCommand( cmdString, false );
                    cmdString = *ptr;
                }
                else
                {
                    cmdString += *ptr;
                }
                break;
            default:
                cmdString += *ptr;
        }

        lastChar = *ptr;
    }
    if( ! cmdString.isEmpty() )
        addCommand( cmdString, false );

    updatePath( size() );
}
