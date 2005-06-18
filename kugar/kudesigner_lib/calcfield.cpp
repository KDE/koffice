/* This file is part of the KDE project
 Copyright (C) 2002-2004 Alexander Dymo <adymo@mksat.net>

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
#include "calcfield.h"

#include <klocale.h>

#include <qmap.h>

#include <property.h>

namespace Kudesigner
{

CalculatedField::CalculatedField( int x, int y, int width, int height, Canvas *canvas ) :
        Field( x, y, width, height, canvas, false )
{
    QMap<QString, QVariant> m;

    props.setGroupDescription( "Calculation", i18n( "Calculation" ) );
    m[ i18n( "Count" ) ] = "0";
    m[ i18n( "Sum" ) ] = "1";
    m[ i18n( "Average" ) ] = "2";
    m[ i18n( "Variance" ) ] = "3";
    m[ i18n( "StandardDeviation" ) ] = "4";
    props.addProperty( new Property( "CalculationType", i18n( "Type" ), i18n( "Calculation Type" ), m, "1" ), "Calculation" );

    registerAs( Rtti_Calculated );
}

void CalculatedField::draw( QPainter &painter )
{
    Field::draw( painter );
}

QString CalculatedField::getXml()
{
    return "\t\t<CalculatedField" + ReportItem::getXml() + " />\n";
}

}
