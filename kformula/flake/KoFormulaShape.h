/* This file is part of the KDE project
   Copyright (C)  2006 Martin Pfeiffer <hubipete@gmx.net>

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
   Boston, MA 02110-1301, USA.
*/

#ifndef KOFORMULASHAPE_H
#define KOFORMULASHAPE_H

#include <KoShape.h>
#define KoFormulaShapeId "FormulaShapeID"

class BasicElement;
class FormulaRenderer;

/**
 * @short The flake shape for a formula
 *
 * This class is basically the container for the formula and has also methods to paint,
 * load and save the formula. The formulaRenderer instance that is part of this class
 * serves to paint and layout the formula. The acutal formula data means the tree of
 * elements is accessible through the root element which is constructed and destroyed
 * by this class that holds a pointer to it. Everything that goes into the area of
 * editing the formula data is implemented in KoFormulaTool respectivly FormulaCursor.
 *
 * @author Martin Pfeiffer <hubipete@gmx.net>
 */
class KoFormulaShape : public KoShape {
public:
    /// The basic constructor
    KoFormulaShape();

    /// The basic destructor
    ~KoFormulaShape();

    /// inherited from KoShape
    void paint( QPainter &painter, const KoViewConverter &converter );
    
    void updateLayout();
    
    /// @return The element at the point @p p
    BasicElement* elementAt( const QPointF& p );

    /// Resize the shape.
    void resize( const QSizeF &size );
    
    /// @return Get the size of the shape in pt.
    QSizeF size() const;

    /// @return Get the bounding box of the shape.
    QRectF boundingRect() const;

    /// @return The formula's root element
    BasicElement* formulaElement() const;

    /**
     * Load a shape from odf - reimplemented from KoShape
     * @param context the KoShapeLoadingContext used for loading
     * @param element element which represents the shape in odf
     * @return false if loading failed
     */ 
    bool loadOdf( const KoXmlElement& element, KoShapeLoadingContext& context );

    /**
     * @brief store the shape data as ODF XML. - reimplemented from KoShape
     * This is the method that will be called when saving a shape as a described in
     * OpenDocument 9.2 Drawing Shapes.
     * @see saveOdfAttributes
     */ 
    void saveOdf( KoShapeSavingContext& context ) const;

private:
    /// The element at the highest level in the formula tree, contains all other elements
    BasicElement* m_formulaElement;

    /// The renderer that takes care of painting the shape's formula
    FormulaRenderer* m_formulaRenderer;
};

#endif // KOFORMULASHAPE_H
