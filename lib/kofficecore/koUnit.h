/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>, Torben Weis <weis@kde.org>

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

#ifndef kounit_h
#define kounit_h
#include <qstring.h>

// 1 inch ^= 72 pt
// 1 inch ^= 25.399956 mm (-pedantic ;p)
// Note: I don't use division but multiplication with the inverse value
// because it's faster ;p (Werner)
#define POINT_TO_MM(px) ((px)*0.352777167)
#define MM_TO_POINT(mm) ((mm)*2.83465058)
#define POINT_TO_CM(px) ((px)*0.0352777167)
#define CM_TO_POINT(cm) ((cm)*28.3465058)
#define POINT_TO_DM(px) ((px)*0.00352777167)
#define DM_TO_POINT(dm) ((dm)*283.465058)
#define POINT_TO_INCH(px) ((px)*0.01388888888889)
#define INCH_TO_POINT(inch) ((inch)*72.0)
#define MM_TO_INCH(mm) ((mm)*0.039370147)
#define INCH_TO_MM(inch) ((inch)*25.399956)

/**
 * KWord stores everthing in pt (using "double") internally.
 * When displaying a value to the user, the value is converted to the user's unit
 * of choice, and rounded to a reasonable precision to avoid 0.999999
 */
class KoUnit
{
public:
    enum Unit {
        U_MM = 0,
        U_PT = 1,
        U_INCH = 2,
        U_CM = 3,
        U_DM = 4
    };

    // Prepare ptValue to be displayed in pt
    static double toPoint( double ptValue ) {
        // No conversion, only rounding (to 0.001 precision)
        return qRound( ptValue * 1000.0 ) / 1000.0;
    }

    // Prepare ptValue to be displayed in mm
    static double toMM( double ptValue ) {
        // "mm" values are rounded to 0.0001 millimeters
        return qRound( POINT_TO_MM( ptValue ) * 10000.0 ) / 10000.0;
    }

    // Prepare ptValue to be displayed in cm
    static double toCM( double ptValue ) {
        return qRound( POINT_TO_CM( ptValue ) * 10000.0 ) / 10000.0;
    }

    // Prepare ptValue to be displayed in dm
    static double toDM( double ptValue ) {
        return qRound( POINT_TO_DM( ptValue ) * 10000.0 ) / 10000.0;
    }

    // Prepare ptValue to be displayed in inch
    static double toInch( double ptValue ) {
        // "in" values are rounded to 0.00001 inches
        return qRound( POINT_TO_INCH( ptValue ) * 100000.0 ) / 100000.0;
    }

    // This method is the one to use to display a value in a dialog
    // Return the value @ptValue converted to @p unit and rounded, ready to be displayed
    static double ptToUnit( double ptValue, Unit unit );

    // This method is the one to use to display a value in a dialog
    // Return the value @ptValue converted to @p unit and rounded, ready to be displayed
    static QString userValue( double ptValue, Unit unit );

    // This method is the one to use to read a value from a dialog
    // Return the value in @p unit, converted to points for internal use
    static double ptFromUnit( double value, Unit unit );

    // This method is the one to use to read a value from a dialog
    // Return the value in @p unit, converted to points for internal use
    static double fromUserValue( const QString& value, Unit unit );

    // Convert a unit name into a Unit enum
    static Unit unit( const QString &_unitName ) {
        if ( _unitName == QString::fromLatin1( "mm" ) ) return U_MM;
        if ( _unitName == QString::fromLatin1( "cm" ) ) return U_CM;
        if ( _unitName == QString::fromLatin1( "dm" ) ) return U_DM;
        if ( _unitName == QString::fromLatin1( "in" )
            || _unitName == QString::fromLatin1("inch") /*compat*/ ) return U_INCH;
        return U_PT;
    }
    // Get the name of a unit
    static QString unitName( Unit _unit ) {
        if ( _unit == U_MM ) return QString::fromLatin1( "mm" );
        if ( _unit == U_CM ) return QString::fromLatin1( "cm" );
        if ( _unit == U_DM ) return QString::fromLatin1( "dm" );
        if ( _unit == U_INCH ) return QString::fromLatin1( "in" );
        return QString::fromLatin1( "pt" );
    }
    // Get the full (translated) description of a unit
    static QString unitDescription( Unit _unit );
};


#endif
