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

#ifndef KOENHANCEDPATHPARAMETER_H
#define KOENHANCEDPATHPARAMETER_H

#include <QString>

class KoEnhancedPathShape;

/// the different possible identifiers, taken from the odf spec
enum Identifier {
    IdentifierUnknown,   ///< unknown identifier
    IdentifierPi,        ///< value of pi.
    IdentifierLeft,      ///< left of svg:viewBox or draw:coordinate-origin-x
    IdentifierTop,       ///< top of svg:viewBox or draw:coordinate-origin-y
    IdentifierRight,     ///< right of svg:viewBox or draw:coordinate-origin-x + draw:coordinate-width
    IdentifierBottom,    ///< bottom of svg:viewBox or draw:coordinate-origin-y + draw:coordinate-height
    IdentifierXstretch,  ///< The value of draw:path-stretchpoint-x is used.
    IdentifierYstretch,  ///< The value of draw:path-stretchpoint-y is used.
    IdentifierHasStroke, ///< If the shape has a line style, a value of 1 is used.
    IdentifierHasFill,   ///< If the shape has a fill style, a value of 1 is used.
    IdentifierWidth,     ///< The width of the svg:viewBox is used.
    IdentifierHeight,    ///< The height of the svg:viewBox is used.
    IdentifierLogwidth,  ///< The width of the svg:viewBox in 1/100th mm is used.
    IdentifierLogheight  ///< The height of the svg:viewBox in 1/100th mm is used.
};

/// The bstract parameter class
class KoEnhancedPathParameter
{
public:
    explicit KoEnhancedPathParameter( KoEnhancedPathShape * parent );
    virtual ~KoEnhancedPathParameter();
    /// evaluates the parameter using the given path
    virtual qreal evaluate() = 0;
    /// modifies the parameter if possible, using the new value
    virtual void modify( qreal value );
    /// returns string representation of the parameter
    virtual QString toString() const = 0;
protected:
    KoEnhancedPathShape * parent();
private:
    KoEnhancedPathShape * m_parent;
};

/// A constant parameter, a fixed value (i.e. 5, 11.3, -7 )
class KoEnhancedPathConstantParameter : public KoEnhancedPathParameter
{
public:
    /// Constructs the constant parameter with the given value
    KoEnhancedPathConstantParameter( qreal value, KoEnhancedPathShape * parent );
    qreal evaluate();
    virtual QString toString() const;
private:
    qreal m_value; ///< the constant value
};

/// A named parameter, one that refers to a variable of the path
class KoEnhancedPathNamedParameter : public KoEnhancedPathParameter
{
public:
    /// Constructs named parameter from given identifier
    KoEnhancedPathNamedParameter( Identifier identifier, KoEnhancedPathShape * parent );
    /// Constructs named parameter from given identifier string
    KoEnhancedPathNamedParameter( const QString &identifier, KoEnhancedPathShape * parent );
    qreal evaluate();
    /// Returns identfier type from given string
    static Identifier identifierFromString( const QString &text );
    virtual QString toString() const;
private:
    Identifier m_identifier; ///< the identifier type
};

/// A referencing parameter, one that refrences another formula or a modifier
class KoEnhancedPathReferenceParameter : public KoEnhancedPathParameter
{
public:
    /// Constructs reference paramater from the fiven reference string
    explicit KoEnhancedPathReferenceParameter( const QString &reference, KoEnhancedPathShape * parent );
    qreal evaluate();
    virtual void modify( qreal value );
    virtual QString toString() const;
private:
    QString m_reference; ///< the reference, formula or modifier
};

#endif // KOENHANCEDPATHPARAMETER_H
