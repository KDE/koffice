/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000 theKompany.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef KIVIO_FILL_STYLE_H
#define KIVIO_FILL_STYLE_H

#include <qbrush.h>
#include <qcolor.h>
#include <qdom.h>
#include <kdebug.h>

class KivioGradient;


class KivioFillStyle
{
public:
    /*
     * kcsNone - No fill
     * kcsSolid - Solid fill
     * kcsGradient - Gradient fill
     * kcsPixmap - Pixmap fill
     */
    typedef enum {
        kcsNone = 0,
        kcsSolid,
        kcsGradient,
        kcsPixmap
    } KivioColorStyle;

protected:
    KivioColorStyle m_colorStyle;       // The color style to use when filling
    QColor m_color;                     // The color to use when solid filling
    QBrush::BrushStyle m_brushStyle;    // The brush pattern to use when solid filling (maybe gradient too?)
    KivioGradient *m_pGradient;         // The gradient to use when filling gradient style
    
public:
    KivioFillStyle();
    KivioFillStyle( const KivioFillStyle & );
    virtual ~KivioFillStyle();
    
    
    void copyInto( KivioFillStyle *pTarget ) const;
    
    bool loadXML( const QDomElement & );
    QDomElement saveXML( QDomDocument & );
    
    
    inline KivioColorStyle colorStyle() const { return m_colorStyle; }
    inline void setKivioColorStyle( KivioColorStyle k ) { m_colorStyle=k; }
    
    
    inline QColor color() const { return m_color; }
    inline void setColor( QColor c ) { m_color=c; }
    
    
    inline QBrush::BrushStyle brushStyle() const { return m_brushStyle; }
    inline void setBrushStyle( QBrush::BrushStyle b ) { m_brushStyle=b; }
    
    inline KivioGradient *gradient() const { return m_pGradient; }
};

#endif


