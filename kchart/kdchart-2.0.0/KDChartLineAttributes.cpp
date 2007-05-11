/****************************************************************************
 ** Copyright (C) 2006 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be distributed and/or modified under the terms of the
 ** GNU General Public License version 2 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.
 **
 ** Licensees holding valid commercial KD Chart licenses may use this file in
 ** accordance with the KD Chart Commercial License Agreement provided with
 ** the Software.
 **
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 ** See http://www.kdab.net/kdchart for
 **   information about KDChart Commercial License Agreements.
 **
 ** Contact info@kdab.net if any conditions of this
 ** licensing are not clear to you.
 **
 **********************************************************************/

#include "KDChartLineAttributes.h"
#include <QDebug>

#include <KDABLibFakes>

#define d d_func()

using namespace KDChart;

class LineAttributes::Private
{
    friend class LineAttributes;
public:
    Private();

private:
    //Areas
    MissingValuesPolicy missingValuesPolicy;
    bool displayArea;
    uint transparency;
};


LineAttributes::Private::Private()
    : missingValuesPolicy( MissingValuesAreBridged )
    , displayArea( false )
    , transparency( 255 )
{
}


LineAttributes::LineAttributes()
    : _d( new Private() )
{
}

LineAttributes::LineAttributes( const LineAttributes& r )
    : _d( new Private( *r.d ) )
{
}

LineAttributes& LineAttributes::operator= ( const LineAttributes& r )
{
    if( this == &r )
        return *this;

    *d = *r.d;

    return *this;
}

LineAttributes::~LineAttributes()
{
    delete _d; _d = 0;
}

bool LineAttributes::operator==( const LineAttributes& r ) const
{
    return
        missingValuesPolicy() == r.missingValuesPolicy() &&
        displayArea() == r.displayArea() &&
        transparency() == r.transparency();
}

void LineAttributes::setMissingValuesPolicy( MissingValuesPolicy policy )
{
    d->missingValuesPolicy = policy;
}

LineAttributes::MissingValuesPolicy LineAttributes::missingValuesPolicy() const
{
    return d->missingValuesPolicy;
}

void LineAttributes::setDisplayArea( bool display )
{
    d->displayArea = display;
}

bool LineAttributes::displayArea() const
{
   return d->displayArea;
}

void LineAttributes::setTransparency( uint alpha )
{
     if ( alpha > 255 )
        alpha = 255;
    d->transparency = alpha;
}

uint LineAttributes::transparency() const
{
     return d->transparency;
}

#if !defined(QT_NO_DEBUG_STREAM)
QDebug operator<<(QDebug dbg, const KDChart::LineAttributes& a)
{
    dbg << "KDChart::LineAttributes("
            //     MissingValuesPolicy missingValuesPolicy;
            << "bool="<<a.displayArea()
            << "transparency="<<a.transparency()
            << ")";
    return dbg;

}
#endif /* QT_NO_DEBUG_STREAM */
