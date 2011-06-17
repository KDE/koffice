/* This file is part of the KDE project
   Copyright 2009 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_RIGHTTOLEFT_PAINTING_STRATEGY
#define KSPREAD_RIGHTTOLEFT_PAINTING_STRATEGY

#include <KoShapeManagerPaintingStrategy.h>

class KCanvasBase;


/**
 */
class RightToLeftPaintingStrategy : public KoShapeManagerPaintingStrategy
{
public:
    RightToLeftPaintingStrategy(KoShapeManager *shapeManager, KCanvasBase *canvas);
    virtual ~RightToLeftPaintingStrategy();

    /**
     * Paint the shape
     *
     * @param shape the shape to paint
     * @param painter the painter to paint to.
     * @param converter to convert between document and view coordinates.
     * @param forPrint if true, make sure only actual content is drawn and no decorations.
     */
    virtual void paint(KShape *shape, QPainter &painter, const KoViewConverter &converter, bool forPrint);

    /**
     * Adapt the rect the shape occupies
     *
     * @param rect rect which will be updated to give the rect the shape occupies.
     */
    virtual void adapt(KShape *shape, QRectF &rect);

private:
    class Private;
    Private *const d;
};

#endif // KSPREAD_RIGHTTOLEFT_PAINTING_STRATEGY
