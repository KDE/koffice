/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <david@mandrakesoft.com>

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

//#include <koGlobal.h>
#include "koUnit.h"
#include <klocale.h>
#include <kglobal.h>
#include <qregexp.h>
#include <kdebug.h>

QStringList KoUnit::listOfUnitName()
{
    QStringList lst;
    for ( uint i = 0 ; i <= KoUnit::U_LASTUNIT ; ++i )
    {
        KoUnit::Unit unit = static_cast<KoUnit::Unit>( i );
        lst.append( KoUnit::unitDescription( unit ) );
    }
    return lst;
}

QString KoUnit::unitDescription( Unit _unit )
{
    switch ( _unit )
    {
    case KoUnit::U_MM:
        return i18n("Millimeters (mm)");
    case KoUnit::U_CM:
        return i18n("Centimeters (cm)");
    case KoUnit::U_DM:
        return i18n("Decimeters (dm)");
    case KoUnit::U_INCH:
        return i18n("Inches (in)");
    case KoUnit::U_PI:
        return i18n("Pica (pi)");
    case KoUnit::U_DD:
        return i18n("Didot (dd)");
    case KoUnit::U_CC:
        return i18n("Cicero (cc)");
    case KoUnit::U_PT:
        return i18n("Points (pt)" );
    default:
        return i18n("Error!");
    }
}

double KoUnit::ptToUnit( double ptValue, Unit unit )
{
    switch ( unit ) {
    case U_MM:
        return toMM( ptValue );
    case U_CM:
        return toCM( ptValue );
    case U_DM:
        return toDM( ptValue );
    case U_INCH:
        return toInch( ptValue );
    case U_PI:
        return toPI( ptValue );
    case U_DD:
        return toDD( ptValue );
    case U_CC:
        return toCC( ptValue );
    case U_PT:
    default:
        return toPoint( ptValue );
    }
}

QString KoUnit::userValue( double ptValue, Unit unit )
{
    return KGlobal::locale()->formatNumber( ptToUnit( ptValue, unit ) );
}

double KoUnit::ptFromUnit( double value, Unit unit )
{
    switch ( unit ) {
    case U_MM:
        return MM_TO_POINT( value );
    case U_CM:
        return CM_TO_POINT( value );
    case U_DM:
        return DM_TO_POINT( value );
    case U_INCH:
        return INCH_TO_POINT( value );
    case U_PI:
        return PI_TO_POINT( value );
    case U_DD:
        return DD_TO_POINT( value );
    case U_CC:
        return CC_TO_POINT( value );
    case U_PT:
    default:
        return value;
    }
}

double KoUnit::fromUserValue( const QString& value, Unit unit )
{
    bool ok; // TODO pass as parameter
    return ptFromUnit( KGlobal::locale()->readNumber( value, &ok ), unit );
}

double KoUnit::parseValue( QString value, double defaultVal )
{
    value.simplifyWhiteSpace();
    value.remove( ' ' );

    if( value.isEmpty() )
        return defaultVal;

    int index = value.find( QRegExp( "[a-z]+$" ) );
    if ( index == -1 )
        return value.toDouble();

    QString unit = value.mid( index );
    value.truncate ( index );
    double val = value.toDouble();

    if ( unit == "pt" )
        return val;

    Unit u = KoUnit::unit( unit );
    if( u != U_PT )
        return ptFromUnit( val, u );
    if( unit == "m" )
        return ptFromUnit( val * 10.0, U_DM );
    else if( unit == "km" )
        return ptFromUnit( val * 10000.0, U_DM );
    kdWarning() << "KoUnit::parseValue: Unit " << unit << "is not supported, please report." << endl;

    // TODO : add support for mi/ft ?
    return defaultVal;
}

