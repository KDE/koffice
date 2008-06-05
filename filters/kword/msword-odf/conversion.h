/* This file is part of the KOffice project
   Copyright (C) 2002 Werner Trobin <trobin@kde.org>
   Copyright (C) 2002 David Faure <faure@kde.org>
   Copyright (C) 2008 Benjamin Cail <cricketc@gmail.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   version 2 of the License, or (at your option) version 3 or,
   at the discretion of KDE e.V (which shall act as a proxy as in
   section 14 of the GPLv3), any later version..

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef CONVERSION_H
#define CONVERSION_H

#include <wv2/ustring.h>
#include <QString>
#include <QColor>

class QDomElement;
namespace wvWare {
    namespace Word97 {
        class LSPD;
        class BRC;
    }
    class FLD;
}

// Static methods for simple MSWord->KWord conversions
// (enums etc.)

namespace Conversion
{
    // UString -> QConstString conversion. Use .string() to get the QString.
    // Always store the QConstString into a variable first, to avoid a deep copy.
    inline QConstString string( const wvWare::UString& str ) {
        // Let's hope there's no copying of the QConstString happening...
        return QConstString( reinterpret_cast<const QChar*>( str.data() ), str.length() );
    }

    // Prepare text for inclusion in XML
    void encodeText(QString &text);

    // Convert color code (ico) to QString with hex code
    QString color(int number, int defaultcolor, bool defaultWhite = false);

    // Convert pattern style (ipat) to QBrush::BrushStyle
    int fillPatternStyle( int ipat );

    // Hackery for gray levels
    int ditheringToGray( int ipat, bool* ok );

    // Convert alignment code to string
    //QString alignment( int jc );

    // Convert linespacing struct to string
    QString lineSpacing( const wvWare::Word97::LSPD& lspd );

    // Convert number format code to a value for the odt style:num-format attribute
    QString numberFormatCode( int nfc );

    // Set the 3 color attributes in the XML element, from a color code (ico)
    // prefix: if empty, the attribute names will be red/blue/green
    // if not empty, they will be xRed/xBlue/xGreen
    void setColorAttributes( QDomElement& element, int ico, const QString& prefix = QString(), bool defaultWhite = false );

    // Return an ODF attribute for border characteristics from a BRC structure
    QString setBorderAttributes( const wvWare::Word97::BRC& brc );

    // Convert wv2's Header Type to a KWord frameInfo value
    int headerTypeToFrameInfo( unsigned char type );

    // Convert wv2's Header Type to a KWord frameset name
    QString headerTypeToFramesetName( unsigned char type );

    // Convert a mask of Header Types to the hType value for KWord
    int headerMaskToHType( unsigned char mask );

    // Convert a mask of Header Types to the fType value for KWord
    int headerMaskToFType( unsigned char mask );

    // Convert wv2's FLD to KWord FIELD.subtype (or -1 if can't be handled)
    int fldToFieldType( const wvWare::FLD* fld );

    bool isHeader( unsigned char type );
}

#endif
