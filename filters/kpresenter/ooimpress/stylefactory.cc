/* This file is part of the KDE project
   Copyright (C) 2003 Percy Leonhardt

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

#include "stylefactory.h"

#include <koUnit.h>
#include <kdebug.h>

StyleFactory::StyleFactory()
{
    m_strokeDashStyles.setAutoDelete( true );
    m_gradientStyles.setAutoDelete( true );
    m_hatchStyles.setAutoDelete( true );
    m_markerStyles.setAutoDelete( true );
    m_fillImageStyles.setAutoDelete( true );
    m_pageStyles.setAutoDelete( true );
    m_textStyles.setAutoDelete( true );
    m_graphicStyles.setAutoDelete( true );
    m_paragraphStyles.setAutoDelete( true );
    m_pageMasterStyles.setAutoDelete( true );

    // create standard graphic style
    GraphicStyle * graphicStyle;
    graphicStyle = new GraphicStyle ( "standard", "solid", "0cm", "0x000000",
                                      "hidden", "0.3cm", "0.3cm", "0x808080",
                                      "0cm", "0cm", "0cm", "0cm", "0x000000",
                                      "false", "none", "Thorndale", "24pt",
                                      "normal", "none", "none", "normal",
                                      "100%", "start", "solid", "0x00b8ff",
                                      "false" );

    m_graphicStyles.append( graphicStyle );
}

StyleFactory::~StyleFactory()
{
}

void StyleFactory::addOfficeStyles( QDomDocument & doc, QDomElement & styles )
{
    StrokeDashStyle * sd;
    for ( sd = m_strokeDashStyles.first(); sd ; sd = m_strokeDashStyles.next() )
        sd->toXML( doc, styles );

    GradientStyle * g;
    for ( g = m_gradientStyles.first(); g ; g = m_gradientStyles.next() )
        g->toXML( doc, styles );

    MarkerStyle * m;
    for ( m = m_markerStyles.first(); m ; m = m_markerStyles.next() )
        m->toXML( doc, styles );

    HatchStyle * h;
    for ( h = m_hatchStyles.first(); h ; h = m_hatchStyles.next() )
        h->toXML( doc, styles );

    GraphicStyle * gr;
    gr = m_graphicStyles.first(); // skip the "standard" style
    gr->toXML( doc, styles );
}

void StyleFactory::addOfficeMaster( QDomDocument & doc, QDomElement & master )
{
    PageMasterStyle * p;
    for ( p = m_pageMasterStyles.first(); p ; p = m_pageMasterStyles.next() )
    {
        QDomElement masterPage = doc.createElement( "style:master-page" );
        masterPage.setAttribute( "style:name", p->style() );
        masterPage.setAttribute( "style:page-master-name", p->name() );
        masterPage.setAttribute( "draw:style-name", "dp1" );
        master.appendChild( masterPage );
    }
}

void StyleFactory::addOfficeAutomatic( QDomDocument & doc, QDomElement & automatic )
{
    PageMasterStyle * p;
    for ( p = m_pageMasterStyles.first(); p ; p = m_pageMasterStyles.next() )
    {
        p->toXML( doc, automatic );
    }
}

void StyleFactory::addAutomaticStyles( QDomDocument & doc, QDomElement & autoStyles )
{
    PageStyle * p;
    for ( p = m_pageStyles.first(); p ; p = m_pageStyles.next() )
        p->toXML( doc, autoStyles );

    TextStyle * t;
    for ( t = m_textStyles.first(); t ; t = m_textStyles.next() )
        t->toXML( doc, autoStyles );

    GraphicStyle * g;
    g = m_graphicStyles.first(); // skip the "standard" style
    for ( g = m_graphicStyles.next(); g ; g = m_graphicStyles.next() )
        g->toXML( doc, autoStyles );

    ParagraphStyle * pg;
    for ( pg = m_paragraphStyles.first(); pg ; pg = m_paragraphStyles.next() )
        pg->toXML( doc, autoStyles );
}

QString StyleFactory::createStrokeDashStyle( int style )
{
    StrokeDashStyle * newStrokeDashStyle, * sd;
    newStrokeDashStyle = new StrokeDashStyle( style );
    for ( sd = m_strokeDashStyles.first(); sd ; sd = m_strokeDashStyles.next() )
    {
        if ( sd->name() == newStrokeDashStyle->name() )
        {
            delete newStrokeDashStyle;
            return sd->name();
        }
    }

    m_strokeDashStyles.append( newStrokeDashStyle );
    return newStrokeDashStyle->name();
}

QString StyleFactory::createGradientStyle( QDomElement & gradient )
{
    GradientStyle * newGradientStyle, * g;
    newGradientStyle = new GradientStyle( gradient, m_gradientStyles.count() + 1 );
    for ( g = m_gradientStyles.first(); g ; g = m_gradientStyles.next() )
    {
        if ( g->name() == newGradientStyle->name() )
        {
            delete newGradientStyle;
            return g->name();
        }
    }

    m_gradientStyles.append( newGradientStyle );
    return newGradientStyle->name();
}

QString StyleFactory::createMarkerStyle( int style )
{
    MarkerStyle * newMarkerStyle, * m;
    newMarkerStyle = new MarkerStyle( style );
    for ( m = m_markerStyles.first(); m ; m = m_markerStyles.next() )
    {
        if ( m->name() == newMarkerStyle->name() )
        {
            delete newMarkerStyle;
            return m->name();
        }
    }

    m_markerStyles.append( newMarkerStyle );
    return newMarkerStyle->name();
}

QString StyleFactory::createHatchStyle( int style, QString & color )
{
    HatchStyle * newHatchStyle, * h;
    newHatchStyle = new HatchStyle( style, color );
    for ( h = m_hatchStyles.first(); h ; h = m_hatchStyles.next() )
    {
        if ( h->name() == newHatchStyle->name() )
        {
            delete newHatchStyle;
            return h->name();
        }
    }

    m_hatchStyles.append( newHatchStyle );
    return newHatchStyle->name();
}

QString StyleFactory::createPageStyle( QDomElement & e )
{
    PageStyle * newPageStyle, * p;
    newPageStyle = new PageStyle( e, m_pageStyles.count() + 1 );
    for ( p = m_pageStyles.first(); p ; p = m_pageStyles.next() )
    {
        if ( *p == *newPageStyle )
        {
            delete newPageStyle;
            return p->name();
        }
    }

    m_pageStyles.append( newPageStyle );
    return newPageStyle->name();
}

QString StyleFactory::createTextStyle( QDomElement & e )
{
    TextStyle * newTextStyle, * t;
    newTextStyle = new TextStyle( e, m_textStyles.count() + 1 );
    for ( t = m_textStyles.first(); t ; t = m_textStyles.next() )
    {
        if ( *t == *newTextStyle )
        {
            delete newTextStyle;
            return t->name();
        }
    }

    m_textStyles.append( newTextStyle );
    return newTextStyle->name();
}

QString StyleFactory::createGraphicStyle( QDomElement & e )
{
    GraphicStyle * newGraphicStyle, * g;
    newGraphicStyle = new GraphicStyle( this, e, m_graphicStyles.count() );
    for ( g = m_graphicStyles.first(); g ; g = m_graphicStyles.next() )
    {
        if ( *g == *newGraphicStyle )
        {
            delete newGraphicStyle;
            return g->name();
        }
    }

    m_graphicStyles.append( newGraphicStyle );
    return newGraphicStyle->name();
}

QString StyleFactory::createParagraphStyle( QDomElement & e )
{
    ParagraphStyle * newParagraphStyle, * p;
    newParagraphStyle = new ParagraphStyle( e, m_paragraphStyles.count() + 1 );
    for ( p = m_paragraphStyles.first(); p ; p = m_paragraphStyles.next() )
    {
        if ( *p == *newParagraphStyle )
        {
            delete newParagraphStyle;
            return p->name();
        }
    }

    m_paragraphStyles.append( newParagraphStyle );
    return newParagraphStyle->name();
}

QString StyleFactory::createPageMasterStyle( QDomElement & e )
{
    PageMasterStyle * newPMStyle, * p;
    newPMStyle = new PageMasterStyle( e, m_pageMasterStyles.count() );
    for ( p = m_pageMasterStyles.first(); p ; p = m_pageMasterStyles.next() )
    {
        if ( *p == *newPMStyle )
        {
            delete newPMStyle;
            return p->style();
        }
    }

    m_pageMasterStyles.append( newPMStyle );
    return newPMStyle->style();
}

QString StyleFactory::toCM( const QString & point )
{
    double pt = point.toFloat();
    double cm = KoUnit::toCM( pt );
    return QString( "%1cm" ).arg ( cm );
}

StrokeDashStyle::StrokeDashStyle( int style )
{
    switch ( style )
    {
    case 2:
        m_name = "Fine Dashed";
        m_style = "rect";
        m_dots1 = "1";
        m_dots1_length = "0.508cm";
        m_dots2 = "1";
        m_dots2_length = "0.508cm";
        m_distance = "0.508cm";
        break;
    case 3:
        m_name = "Fine Dotted";
        m_style = "rect";
        m_dots1 = "1";
        m_distance = "0.257cm";
        break;
    case 4:
        m_name = "Ultrafine 1 Dot 1 Dash";
        m_style = "rect";
        m_dots1 = "1";
        m_dots1_length = "0.051cm";
        m_dots2 = "1";
        m_dots2_length = "0.254cm";
        m_distance = "0.127cm";
        break;
    case 5:
        m_name = "2 Dots 1 Dash";
        m_style = "rect";
        m_dots1 = "2";
        m_dots2 = "1";
        m_dots2_length = "0.203cm";
        m_distance = "0.203cm";
        break;
    }
}

void StrokeDashStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement strokeDash = doc.createElement( "draw:stroke-dash" );
    strokeDash.setAttribute( "draw:name", m_name );
    if ( m_style != QString::null )
        strokeDash.setAttribute( "draw:style", m_style );
    if ( m_dots1 != QString::null )
        strokeDash.setAttribute( "draw:dots1", m_dots1 );
    if ( m_dots1_length != QString::null )
        strokeDash.setAttribute( "draw:dots1-length", m_dots1_length );
    if ( m_dots2 != QString::null )
        strokeDash.setAttribute( "draw:dots2", m_dots2 );
    if ( m_dots2_length != QString::null )
        strokeDash.setAttribute( "draw:dots2-length", m_dots2_length );
    if ( m_distance != QString::null )
        strokeDash.setAttribute( "draw:distance", m_distance );

    e.appendChild( strokeDash );
}

GradientStyle::GradientStyle( QDomElement & gradient, int index )
{
    m_name = QString( "Gradient %1" ).arg( index );
    m_start_intensity = "100%";
    m_end_intensity = "100%";
    m_border = "0%";

    if ( gradient.hasAttribute( "color1" ) )
        m_start_color = gradient.attribute( "color1" );
    if ( gradient.hasAttribute( "color2" ) )
        m_end_color = gradient.attribute( "color2" );
    if ( gradient.hasAttribute( "type" ) )
    {
        int type = gradient.attribute( "type" ).toInt();
        switch ( type )
        {
        case 1:
            m_style = "linear";
            m_angle = "0";
            break;
        case 2:
            m_style = "linear";
            m_angle = "900";
            break;
        case 3:
            m_style = "linear";
            m_angle = "450";
            break;
        case 4:
            m_style = "linear";
            m_angle = "135";
            break;
        case 5:
            m_style = "radial";
            m_angle = "0";
            break;
        case 6:
            m_style = "square";
            m_angle = "0";
            break;
        case 7:
            m_style = "axial";
            m_angle = "0";
            break;
        }
    }
    if ( gradient.hasAttribute( "unbalanced" ) )
    {
        if ( gradient.attribute( "unbalanced" ) == "0" )
        {
            m_cx = "50%";
            m_cy = "50%";
        }
        else
        {
            int cx = gradient.attribute( "xfactor" ).toInt();
            int cy = gradient.attribute( "yfactor" ).toInt();
            m_cx = QString( "%1%" ).arg( cx / 4 + 50 );
            m_cy = QString( "%1%" ).arg( cy / 4 + 50 );
        }
    }
}

void GradientStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement gradient = doc.createElement( "draw:gradient" );
    gradient.setAttribute( "draw:name", m_name );
    if ( m_style != QString::null )
        gradient.setAttribute( "draw:style", m_style );
    if ( m_start_color != QString::null )
        gradient.setAttribute( "draw:start-color", m_start_color );
    if ( m_end_color != QString::null )
        gradient.setAttribute( "draw:end-color", m_end_color );
    if ( m_start_intensity != QString::null )
        gradient.setAttribute( "draw:start-intensity", m_start_intensity );
    if ( m_end_intensity != QString::null )
        gradient.setAttribute( "draw:end-intensity", m_end_intensity );
    if ( m_angle != QString::null )
        gradient.setAttribute( "draw:angle", m_angle );
    if ( m_border != QString::null )
        gradient.setAttribute( "draw:border", m_border );
    if ( m_cx != QString::null )
        gradient.setAttribute( "draw:cx", m_cx );
    if ( m_cy != QString::null )
        gradient.setAttribute( "draw:cy", m_cy );

    e.appendChild( gradient );
}

MarkerStyle::MarkerStyle( int style )
{
    // Markers are not working because OOImpress depends on the sequence
    // of the attributes in the draw:marker tag. svg:ViewBox has to be in
    // front of svg:d in order to work.

    switch ( style )
    {
    case 1:
        m_name = "Arrow";
        m_viewBox = "0 0 20 30";
        m_d = "m10 0-10 30h20";
        break;
    case 2:
        m_name = "Square";
        m_viewBox = "0 0 10 10";
        m_d = "m0 0h10v10h-10z";
        break;
    case 3:
        m_name = "Circle";
        m_viewBox = "0 0 1131 1131";
        m_d = "m462 1118-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z";
        break;
    case 4:
        m_name = "Line Arrow";
        m_viewBox = "0 0 1122 2243";
        m_d = "m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z";
        break;
    case 5:
        m_name = "Dimension Lines";
        m_viewBox = "0 0 836 110";
        m_d = "m0 0h278 278 280v36 36 38h-278-278-280v-36-36z";
        break;
    case 6:
        m_name = "Double Arrow";
        m_viewBox = "0 0 1131 1918";
        m_d = "m737 1131h394l-564-1131-567 1131h398l-398 787h1131z";
        break;
    }
}

void MarkerStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement marker = doc.createElement( "draw:marker" );
    marker.setAttribute( "draw:name", m_name );
    if ( m_d != QString::null )
        marker.setAttribute( "svg:d", m_d );
    if ( m_viewBox != QString::null )
        marker.setAttribute( "svg:viewBox", m_viewBox );

    e.appendChild( marker );
}

HatchStyle::HatchStyle( int style, QString & color )
{
    m_color = color;

    switch ( style )
    {
    case 9:
        m_name = m_color + " 0 Degrees";
        m_style = "single";
        m_distance = "0.102cm";
        m_rotation = "0";
        break;
    case 10:
        m_name = m_color + " 90 Degrees";
        m_style = "single";
        m_distance = "0.102cm";
        m_rotation = "900";
        break;
    case 11:
        m_name = m_color + " Crossed 0 Degrees";
        m_style = "double";
        m_distance = "0.076cm";
        m_rotation = "900";
        break;
    case 12:
        m_name = m_color + " 45 Degrees";
        m_style = "single";
        m_distance = "0.102cm";
        m_rotation = "450";
        break;
    case 13:
        m_name = m_color + " -45 Degrees";
        m_style = "single";
        m_distance = "0.102cm";
        m_rotation = "3150";
        break;
    case 14:
        m_name = m_color + " Crossed 45 Degrees";
        m_style = "double";
        m_distance = "0.076cm";
        m_rotation = "450";
        break;
    }
}

void HatchStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement hatch = doc.createElement( "draw:hatch" );
    hatch.setAttribute( "draw:name", m_name );
    if ( m_style != QString::null )
        hatch.setAttribute( "draw:style", m_style );
    if ( m_color != QString::null )
        hatch.setAttribute( "draw:color", m_color );
    if ( m_distance != QString::null )
        hatch.setAttribute( "draw:distance", m_distance );
    if ( m_rotation != QString::null )
        hatch.setAttribute( "draw:rotation", m_rotation );

    e.appendChild( hatch );
}

FillImageStyle::FillImageStyle( QString & name )
{

}

void FillImageStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{

}

PageMasterStyle::PageMasterStyle( QDomElement & e, const uint index )
{
    QDomNode borders = e.namedItem( "PAPERBORDERS" );
    QDomElement b = borders.toElement();

    m_name = QString( "PM%1" ).arg( index );
    m_style = QString( "Default%1" ).arg( index );
    m_margin_top = StyleFactory::toCM( b.attribute( "ptTop" ) );
    m_margin_bottom = StyleFactory::toCM( b.attribute( "ptBottom" ) );
    m_margin_left = StyleFactory::toCM( b.attribute( "ptLeft" ) );
    m_margin_right = StyleFactory::toCM( b.attribute( "ptRight" ) );
    m_page_width = StyleFactory::toCM( e.attribute( "ptWidth" ) );
    m_page_height = StyleFactory::toCM( e.attribute( "ptHeight" ) );
    m_orientation = "landscape";
}

void PageMasterStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement style = doc.createElement( "style:page-master" );
    style.setAttribute( "style:name", "PM0" );

    QDomElement properties = doc.createElement( "style:properties" );
    properties.setAttribute( "fo:margin-top", m_margin_top );
    properties.setAttribute( "fo:margin-bottom", m_margin_bottom );
    properties.setAttribute( "fo:margin-left", m_margin_left );
    properties.setAttribute( "fo:margin-right", m_margin_right );
    properties.setAttribute( "fo:page-width", m_page_width );
    properties.setAttribute( "fo:page-height", m_page_height );
    properties.setAttribute( "fo:print-orientation", m_orientation );

    style.appendChild( properties );
    e.appendChild( style );
}

bool PageMasterStyle::operator==( const PageMasterStyle & pageMasterStyle ) const
{
    return ( m_margin_top == pageMasterStyle.m_margin_top &&
             m_margin_bottom == pageMasterStyle.m_margin_bottom &&
             m_margin_left == pageMasterStyle.m_margin_left &&
             m_margin_right == pageMasterStyle.m_margin_right &&
             m_page_width == pageMasterStyle.m_page_width &&
             m_page_height == pageMasterStyle.m_page_height &&
             m_orientation == pageMasterStyle.m_orientation );
}

PageStyle::PageStyle( QDomElement & e, const uint index )
{
    // some defaults that won't be overwritten because they
    // are not available in KPresenter
    m_bg_visible = "true";
    m_bg_objects_visible = "true";

    m_name = QString( "dp%1" ).arg( index );

    // check if this is an empty page tag
    if ( !e.hasChildNodes() )
        return;

    QDomElement backType = e.namedItem( "BACKTYPE" ).toElement();
    if ( backType.isNull() || backType.attribute( "value" ) == "0" )
    {
        // color
        QDomElement bcType = e.namedItem( "BYTYPE" ).toElement();
        if ( bcType.isNull() || bcType.attribute( "value" ) == "0" )
        {
            // plain
            QDomElement backColor = e.namedItem( "BACKCOLOR1" ).toElement();
            m_fill = "solid";
            m_fill_color = backColor.attribute( "color" );
        }
        else
        {
            // gradient
        }
    }
    else
    {
        // picture
    }
}

void PageStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement style = doc.createElement( "style:style" );
    style.setAttribute( "style:name", m_name );
    style.setAttribute( "style:family", "drawing-page" );

    QDomElement properties = doc.createElement( "style:properties" );
    properties.setAttribute( "presentation:background-visible", m_bg_visible );
    properties.setAttribute( "presentation:background-objects-visible",
                             m_bg_objects_visible );
    if ( m_fill != QString::null )
        properties.setAttribute( "draw:fill", m_fill );
    if ( m_fill_color != QString::null )
        properties.setAttribute( "draw:fill-color", m_fill_color );
    if ( m_fill_image_name != QString::null )
        properties.setAttribute( "draw:fill-image-name", m_fill_image_name );
    if ( m_fill_image_width != QString::null )
        properties.setAttribute( "draw:fill-image-width", m_fill_image_width );
    if ( m_fill_image_height != QString::null )
        properties.setAttribute( "draw:fill-image-height", m_fill_image_height );
    if ( m_fill_image_ref_point != QString::null )
        properties.setAttribute( "draw:fill-image-ref-point", m_fill_image_ref_point );
    if ( m_fill_gradient_name != QString::null )
        properties.setAttribute( "draw:fill-gradient-name", m_fill_gradient_name );
    if ( m_repeat != QString::null )
        properties.setAttribute( "style:repeat", m_repeat );

    style.appendChild( properties );
    e.appendChild( style );
}

bool PageStyle::operator==( const PageStyle & pageStyle ) const
{
    return ( m_bg_visible == pageStyle.m_bg_visible &&
             m_bg_objects_visible == pageStyle.m_bg_objects_visible &&
             m_fill == pageStyle.m_fill &&
             m_fill_color == pageStyle.m_fill_color &&
             m_fill_image_name == pageStyle.m_fill_image_name &&
             m_fill_image_width == pageStyle.m_fill_image_width &&
             m_fill_image_height == pageStyle.m_fill_image_height &&
             m_fill_image_ref_point == pageStyle.m_fill_image_ref_point &&
             m_fill_gradient_name == pageStyle.m_fill_gradient_name &&
             m_repeat == pageStyle.m_repeat );
}

TextStyle::TextStyle( QDomElement & e, const uint index )
{
    m_name = QString( "T%1" ).arg( index );
    if ( e.hasAttribute( "family" ) )
        m_font_family = e.attribute( "family" );
    if ( e.hasAttribute( "pointSize" ) )
        m_font_size = QString( "%1pt" ).arg( e.attribute( "pointSize" ) );
    if ( e.hasAttribute( "color" ) )
        m_color = e.attribute( "color" );
}

void TextStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement style = doc.createElement( "style:style" );
    style.setAttribute( "style:name", m_name );
    style.setAttribute( "style:family", "text" );

    QDomElement properties = doc.createElement( "style:properties" );
    if ( m_font_size != QString::null )
        properties.setAttribute( "fo:font-size", m_font_size );
    if ( m_font_family != QString::null )
        properties.setAttribute( "fo:font-family", m_font_family );
    if ( m_font_family_generic != QString::null )
        properties.setAttribute( "fo:font-family-generic", m_font_family_generic );
    if ( m_color != QString::null )
        properties.setAttribute( "fo:color", m_color );
    if ( m_font_pitch != QString::null )
        properties.setAttribute( "style:font-pitch", m_font_pitch );
    if ( m_font_style != QString::null )
        properties.setAttribute( "fo:font-style", m_font_style );
    if ( m_font_weight != QString::null )
        properties.setAttribute( "fo:font-weight", m_font_weight );
    if ( m_text_shadow != QString::null )
        properties.setAttribute( "fo:text-shadow", m_text_shadow );
    if ( m_text_underline != QString::null )
        properties.setAttribute( "style:text-underline", m_text_underline );
    if ( m_text_underline_color != QString::null )
        properties.setAttribute( "style:text-underline-color", m_text_underline_color );
    if ( m_text_crossing_out != QString::null )
        properties.setAttribute( "style:text-crossing-out", m_text_crossing_out );

    style.appendChild( properties );
    e.appendChild( style );
}

bool TextStyle::operator==( const TextStyle & textStyle ) const
{
    return ( m_font_size == textStyle.m_font_size &&
             m_font_family == textStyle.m_font_family &&
             m_font_family_generic == textStyle.m_font_family_generic &&
             m_color == textStyle.m_color &&
             m_font_pitch == textStyle.m_font_pitch &&
             m_font_style == textStyle.m_font_style &&
             m_font_weight == textStyle.m_font_weight &&
             m_text_shadow == textStyle.m_text_shadow &&
             m_text_underline == textStyle.m_text_underline &&
             m_text_underline_color == textStyle.m_text_underline_color &&
             m_text_crossing_out == textStyle.m_text_crossing_out );
}

GraphicStyle::GraphicStyle( StyleFactory * styleFactory, QDomElement & e, const uint index )
{
    QDomNode pen = e.namedItem( "PEN" );
    QDomNode brush = e.namedItem( "BRUSH" );
    QDomNode linebegin = e.namedItem( "LINEBEGIN" );
    QDomNode lineend = e.namedItem( "LINEEND" );
    QDomNode gradient = e.namedItem( "GRADIENT" );

    m_name = QString( "gr%1" ).arg( index );
    if ( !pen.isNull() )
    {
        QDomElement p = pen.toElement();
        m_stroke_width = StyleFactory::toCM( p.attribute( "width" ) );
        m_stroke_color = p.attribute( "color" );

        int style = p.attribute( "style" ).toInt();
        if ( style == 1 )
            m_stroke = "solid";
        else if ( style >= 2 && style <= 5 )
        {
            m_stroke = "dash";
            m_stroke_dash = styleFactory->createStrokeDashStyle( style );
        }
        else
            m_stroke = "none";
    }

    if ( !brush.isNull() )
    {
        QDomElement b = brush.toElement();
        m_fill_color = b.attribute( "color" );

        int style = b.attribute( "style" ).toInt();
        if ( style == 1 )
            m_fill = "solid";
        else if ( style >= 9 && style <= 14 )
        {
            m_fill = "hatch";
            m_fill_hatch_name = styleFactory->createHatchStyle( style, m_fill_color );
        }
    }
    else if ( !gradient.isNull() )
    {
        QDomElement g = gradient.toElement();
        m_fill = "gradient";
        m_fill_gradient_name = styleFactory->createGradientStyle( g );
    }
    else
        m_fill = "none";

    if ( !linebegin.isNull() )
    {
        QDomElement lb = linebegin.toElement();
        m_marker_start_width = "0.25cm";

        int style = lb.attribute( "value" ).toInt();
        m_marker_start = styleFactory->createMarkerStyle( style );
    }

    if ( !lineend.isNull() )
    {
        QDomElement le = lineend.toElement();
        m_marker_end_width = "0.25cm";

        int style = le.attribute( "value" ).toInt();
        m_marker_end = styleFactory->createMarkerStyle( style );
    }
}

GraphicStyle::GraphicStyle( const char * name,
                            const char * stroke, const char * stroke_color,
                            const char * stroke_width, const char * shadow,
                            const char * shadow_offset_x, const char * shadow_offset_y,
                            const char * shadow_color, const char * margin_left,
                            const char * margin_right, const char * margin_top,
                            const char * margin_bottom, const char * color,
                            const char * text_outline, const char * text_crossing_out,
                            const char * font_family, const char * font_size,
                            const char * font_style, const char * text_shadow,
                            const char * text_underline, const char * font_weight,
                            const char * line_height, const char * text_align,
                            const char * fill, const char * fill_color,
                            const char * enable_numbering )
    : m_name( name )
    , m_stroke( stroke )
    , m_stroke_color( stroke_color )
    , m_stroke_width( stroke_width )
    , m_shadow( shadow )
    , m_shadow_offset_x( shadow_offset_x )
    , m_shadow_offset_y( shadow_offset_y )
    , m_shadow_color( shadow_color )
    , m_margin_left( margin_left )
    , m_margin_right( margin_right )
    , m_margin_top( margin_top )
    , m_margin_bottom( margin_bottom )
    , m_color( color )
    , m_text_outline( text_outline )
    , m_text_crossing_out( text_crossing_out )
    , m_font_family( font_family )
    , m_font_size( font_size )
    , m_font_style( font_style )
    , m_text_shadow( text_shadow )
    , m_text_underline( text_underline )
    , m_font_weight( font_weight )
    , m_line_height( line_height )
    , m_text_align( text_align )
    , m_fill( fill )
    , m_fill_color( fill_color )
    , m_enable_numbering( enable_numbering )
{
}


void GraphicStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement style = doc.createElement( "style:style" );
    style.setAttribute( "style:name", m_name );
    style.setAttribute( "style:family", "graphics" );
    if ( m_name != "standard" )
        style.setAttribute( "style:parent-style-name", "standard" );

    QDomElement properties = doc.createElement( "style:properties" );
    if ( m_stroke != QString::null )
        properties.setAttribute( "draw:stroke", m_stroke );
    if ( m_stroke_dash != QString::null )
        properties.setAttribute( "draw:stroke-dash", m_stroke_dash );
    if ( m_stroke_color != QString::null )
        properties.setAttribute( "svg:stroke-color", m_stroke_color );
    if ( m_stroke_width != QString::null )
        properties.setAttribute( "svg:stroke-width", m_stroke_width );
    if ( m_shadow != QString::null )
        properties.setAttribute( "draw:shadow", m_shadow );
    if ( m_shadow_offset_x != QString::null )
        properties.setAttribute( "draw:shadow-offset-x", m_shadow_offset_x );
    if ( m_shadow_offset_y != QString::null )
        properties.setAttribute( "draw:shadow-offset-y", m_shadow_offset_y );
    if ( m_shadow_color != QString::null )
        properties.setAttribute( "draw:shadow-color", m_shadow_color );
    if ( m_margin_left != QString::null )
        properties.setAttribute( "fo:margin-left", m_margin_left );
    if ( m_margin_right != QString::null )
        properties.setAttribute( "fo:margin-right", m_margin_right );
    if ( m_margin_top != QString::null )
        properties.setAttribute( "fo:margin-top", m_margin_top );
    if ( m_margin_bottom != QString::null )
        properties.setAttribute( "fo:margin-bottom", m_margin_bottom );
    if ( m_color != QString::null )
        properties.setAttribute( "fo:color", m_color );
    if ( m_text_outline != QString::null )
        properties.setAttribute( "style:text-outline", m_text_outline );
    if ( m_text_crossing_out != QString::null )
        properties.setAttribute( "style:text-crossing-out", m_text_crossing_out );
    if ( m_font_family != QString::null )
        properties.setAttribute( "fo:font-family", m_font_family );
    if ( m_font_size != QString::null )
        properties.setAttribute( "fo:font-size", m_font_size );
    if ( m_font_style != QString::null )
        properties.setAttribute( "fo:font-style", m_font_style );
    if ( m_text_shadow != QString::null )
        properties.setAttribute( "fo:text-shadow", m_text_shadow );
    if ( m_text_underline != QString::null )
        properties.setAttribute( "style:text-underline", m_text_underline );
    if ( m_font_weight != QString::null )
        properties.setAttribute( "fo:font-weight", m_font_weight );
    if ( m_line_height != QString::null )
        properties.setAttribute( "fo:line-height", m_line_height );
    if ( m_text_align != QString::null )
        properties.setAttribute( "fo:text-align", m_text_align );
    if ( m_fill != QString::null )
        properties.setAttribute( "draw:fill", m_fill );
    if ( m_fill_color != QString::null )
        properties.setAttribute( "draw:fill-color", m_fill_color );
    if ( m_fill_hatch_name != QString::null )
        properties.setAttribute( "draw:fill-hatch-name", m_fill_hatch_name );
    if ( m_enable_numbering != QString::null )
        properties.setAttribute( "text:enable-numbering", m_enable_numbering );
    if ( m_marker_start != QString::null )
        properties.setAttribute( "draw:marker-start", m_marker_start );
    if ( m_marker_start_width != QString::null )
        properties.setAttribute( "draw:marker-start-width", m_marker_start_width );
    if ( m_marker_end != QString::null )
        properties.setAttribute( "draw:marker-end", m_marker_end );
    if ( m_marker_end_width != QString::null )
        properties.setAttribute( "draw:marker-end-width", m_marker_end_width );
    if ( m_fill_gradient_name != QString::null )
        properties.setAttribute( "draw:fill-gradient-name", m_fill_gradient_name );

    style.appendChild( properties );
    e.appendChild( style );
}

bool GraphicStyle::operator==( const GraphicStyle & graphicStyle ) const
{
    return ( m_stroke == graphicStyle.m_stroke &&
             m_stroke_dash == graphicStyle.m_stroke_dash &&
             m_stroke_color == graphicStyle.m_stroke_color &&
             m_stroke_width == graphicStyle.m_stroke_width &&
             m_shadow == graphicStyle.m_shadow &&
             m_shadow_offset_x == graphicStyle.m_shadow_offset_x &&
             m_shadow_offset_y == graphicStyle.m_shadow_offset_y &&
             m_shadow_color == graphicStyle.m_shadow_color &&
             m_margin_left == graphicStyle.m_margin_left &&
             m_margin_right == graphicStyle.m_margin_right &&
             m_margin_top == graphicStyle.m_margin_top &&
             m_margin_bottom == graphicStyle.m_margin_bottom &&
             m_color == graphicStyle.m_color &&
             m_text_outline == graphicStyle.m_text_outline &&
             m_text_crossing_out == graphicStyle.m_text_crossing_out &&
             m_font_family == graphicStyle.m_font_family &&
             m_font_size == graphicStyle.m_font_size &&
             m_font_style == graphicStyle.m_font_style &&
             m_text_shadow == graphicStyle.m_text_shadow &&
             m_text_underline == graphicStyle.m_text_underline &&
             m_font_weight == graphicStyle.m_font_weight &&
             m_line_height == graphicStyle.m_line_height &&
             m_text_align == graphicStyle.m_text_align &&
             m_fill == graphicStyle.m_fill &&
             m_fill_color == graphicStyle.m_fill_color &&
             m_fill_hatch_name == graphicStyle.m_fill_hatch_name &&
             m_enable_numbering == graphicStyle.m_enable_numbering &&
             m_marker_start == graphicStyle.m_marker_start &&
             m_marker_start_width == graphicStyle.m_marker_start_width &&
             m_marker_end == graphicStyle.m_marker_end &&
             m_marker_end_width == graphicStyle.m_marker_end_width &&
             m_fill_gradient_name == graphicStyle.m_fill_gradient_name );
}

ParagraphStyle::ParagraphStyle( QDomElement & e, const uint index )
{
    // some defaults that may be overwritten
    m_margin_left = "0cm";
    m_margin_right = "0cm";
    m_text_indent = "0cm";

    m_name = QString( "P%1" ).arg( index );
    if ( e.hasAttribute( "align" ) )
    {
        int align = e.attribute( "align" ).toInt();
        switch ( align )
        {
        case 0: // left
            m_text_align = "start";
            break;
        case 2: // right
            m_text_align = "end";
            break;
        case 4: // center
            m_text_align = "center";
        }
    }
}

void ParagraphStyle::toXML( QDomDocument & doc, QDomElement & e ) const
{
    QDomElement style = doc.createElement( "style:style" );
    style.setAttribute( "style:name", m_name );
    style.setAttribute( "style:family", "paragraph" );

    QDomElement properties = doc.createElement( "style:properties" );
    if ( m_margin_left != QString::null )
        properties.setAttribute( "fo:margin-left", m_margin_left );
    if ( m_margin_right != QString::null )
        properties.setAttribute( "fo:margin-right", m_margin_right );
    if ( m_text_indent != QString::null )
        properties.setAttribute( "fo:text-indent", m_text_indent );
    if ( m_text_align != QString::null )
        properties.setAttribute( "fo:text-align", m_text_align );
    if ( m_enable_numbering != QString::null )
        properties.setAttribute( "text:enable-numbering", m_enable_numbering );

    style.appendChild( properties );
    e.appendChild( style );
}

bool ParagraphStyle::operator==( const ParagraphStyle & paragraphStyle ) const
{
    return ( m_margin_left == paragraphStyle.m_margin_left &&
             m_margin_right == paragraphStyle.m_margin_right &&
             m_text_indent == paragraphStyle.m_text_indent &&
             m_text_align == paragraphStyle.m_text_align &&
             m_enable_numbering == paragraphStyle.m_enable_numbering );
}

