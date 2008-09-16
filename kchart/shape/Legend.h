/* This file is part of the KDE project

   Copyright 2007-2008 Johannes Simon <johannes.simon@gmail.com>

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

#ifndef KCHART_LEGEND_H
#define KCHART_LEGEND_H

// Local
#include "ChartShape.h"

// KOffice
#include <KoShape.h>

namespace KChart {

class CHARTSHAPELIB_EXPORT Legend : public KoShape
{
public:
    Legend( ChartShape *parent );
    ~Legend();

    QString title() const;
    bool showFrame() const;
    QPen framePen() const;
    QColor frameColor() const;
    QBrush backgroundBrush() const;
    QColor backgroundColor() const;
    QFont font() const;
    double fontSize() const;
    QFont titleFont() const;
    double titleFontSize() const;
    LegendExpansion expansion() const;
    Qt::Alignment alignment() const;
    LegendPosition legendPosition() const;

    void setTitle( const QString &title );
    void setShowFrame( bool show );
    void setFramePen( const QPen &pen );
    void setFrameColor( const QColor &color );
    void setBackgroundBrush( const QBrush &brush );
    void setBackgroundColor( const QColor &color );
    void setFont( const QFont &font );
    void setFontSize( double size );
    void setTitleFont( const QFont &font );
    void setTitleFontSize( double size );
    void setExpansion( LegendExpansion expansion );
    void setAlignment( Qt::Alignment alignment );
    void setLegendPosition( LegendPosition position );
    void setSize( const QSizeF &size );
    
    void paint( QPainter &painter, const KoViewConverter &converter );
    void paintPixmap( QPainter &painter, const KoViewConverter &converter );
    
    bool loadOdf( const KoXmlElement &legendElement, KoShapeLoadingContext &context );
    void saveOdf( KoShapeSavingContext &context ) const;
    
    KDChart::Legend *kdLegend() const;
    
    void update();

private:
    class Private;
    Private *const d;
};

}

#endif // KCHART_LEGEND_H

