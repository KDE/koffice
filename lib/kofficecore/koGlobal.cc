/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

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

#include <koGlobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kprinter.h>
#include <qfont.h>
#include <qfontinfo.h>
#include <kglobalsettings.h>
#include <kglobal.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

int KoPageFormat::printerPageSize( KoFormat format )
{
    switch ( format ) {
        case PG_DIN_A3:
            return KPrinter::A3;
        case PG_DIN_A4:
            return KPrinter::A4;
        case PG_DIN_A5:
            return KPrinter::A5;
        case PG_US_LETTER:
            return KPrinter::Letter;
        case PG_US_LEGAL:
            return KPrinter::Legal;
        case PG_SCREEN:
            kdWarning() << "You use the page layout SCREEN. Printing in DIN A4 LANDSCAPE." << endl;
            return KPrinter::A4;
        case PG_CUSTOM:
            kdWarning() << "The used page layout (CUSTOM) is not supported by KPrinter. Printing in A4." << endl;
            return KPrinter::A4;
        case PG_DIN_B5:
            return KPrinter::B5;
        case PG_US_EXECUTIVE:
            return KPrinter::Executive;
        case PG_DIN_A0:
            return KPrinter::A0;
        case PG_DIN_A1:
            return KPrinter::A1;
        case PG_DIN_A2:
            return KPrinter::A2;
        case PG_DIN_A6:
            return KPrinter::A6;
        case PG_DIN_A7:
            return KPrinter::A7;
        case PG_DIN_A8:
            return KPrinter::A8;
        case PG_DIN_A9:
            return KPrinter::A9;
        case PG_DIN_B0:
            return KPrinter::B0;
        case PG_DIN_B1:
            return KPrinter::B1;
        case PG_DIN_B10:
            return KPrinter::B10;
        case PG_DIN_B2:
            return KPrinter::B2;
        case PG_DIN_B3:
            return KPrinter::B3;
        case PG_DIN_B4:
            return KPrinter::B4;
        case PG_DIN_B6:
            return KPrinter::B6;
    }
    return KPrinter::A4;   // let's make Tru64's cxx happy
}

static const double s_widths[ PG_LAST_FORMAT+1 ] = {
    PG_A3_WIDTH,
    PG_A4_WIDTH,
    PG_A5_WIDTH,
    PG_US_LETTER_WIDTH,
    PG_US_LEGAL_WIDTH,
    PG_A4_HEIGHT,  // since we fallback on A4 landscape
    0, // custom
    PG_B5_WIDTH,
    PG_US_EXECUTIVE_WIDTH,
    841.0,
    594.0,
    420.0,
    105.0,
    74.0,
    52.0,
    37.0,
    1030.0, // B0
    728.0,
    32.0,
    515.0,
    364.0,
    257.0,
    128.0 // B6
};

static const double s_heights[ PG_LAST_FORMAT+1 ] = {
    PG_A3_HEIGHT,
    PG_A4_HEIGHT,
    PG_A5_HEIGHT,
    PG_US_LETTER_HEIGHT,
    PG_US_LEGAL_HEIGHT,
    PG_A4_WIDTH, // since we fallback on A4 landscape
    0, // custom
    PG_B5_HEIGHT,
    PG_US_EXECUTIVE_HEIGHT,
    1189.0, // A0
    841.0,
    594.0,
    148.0,
    105.0,
    74.0,
    52.0,
    1456.0, // B0
    1030.0,
    45.0,
    728.0,
    515.0,
    364.0,
    182.0 // B6
};

double KoPageFormat::width( KoFormat format, KoOrientation orientation )
{
    if ( orientation == PG_LANDSCAPE )
        return height( format, PG_PORTRAIT );
    if ( format <= PG_LAST_FORMAT )
        return s_widths[ format ];
    return PG_A4_WIDTH;   // should never happen
}

double KoPageFormat::height( KoFormat format, KoOrientation orientation )
{
    if ( orientation == PG_LANDSCAPE )
        return width( format, PG_PORTRAIT );
    if ( format <= PG_LAST_FORMAT )
        return s_heights[ format ];
    return PG_A4_HEIGHT;
}

KoFormat KoPageFormat::guessFormat( double width, double height )
{
    for ( int i = 0 ; i <= PG_LAST_FORMAT ; ++i )
    {
        // We have some tolerance. 1pt is a third of a mm, this is
        // barely noticeable for a page size.
        if ( i != PG_CUSTOM
             && QABS( width - s_widths[i] ) < 1
             && QABS( height - s_heights[i] ) < 1 )
            return static_cast<KoFormat>(i);
    }
    return PG_CUSTOM;
}

QString KoPageFormat::formatString( KoFormat format )
{
    switch( format )
    {
        case PG_DIN_A3:
            return QString::fromLatin1( "A3" );
        case PG_DIN_A4:
            return QString::fromLatin1( "A4" );
        case PG_DIN_A5:
            return QString::fromLatin1( "A5" );
        case PG_US_LETTER:
            return QString::fromLatin1( "Letter" );
        case PG_US_LEGAL:
            return QString::fromLatin1( "Legal" );
        case PG_SCREEN:
            return QString::fromLatin1( "Screen" );
        case PG_CUSTOM:
            return QString::fromLatin1( "Custom" );
        case PG_DIN_B5:
            return QString::fromLatin1( "B5" );
        case PG_US_EXECUTIVE:
            return QString::fromLatin1( "Executive" );
        case PG_DIN_A0:
            return QString::fromLatin1( "A0" );
        case PG_DIN_A1:
            return QString::fromLatin1( "A1" );
        case PG_DIN_A2:
            return QString::fromLatin1( "A2" );
        case PG_DIN_A6:
            return QString::fromLatin1( "A6" );
        case PG_DIN_A7:
            return QString::fromLatin1( "A7" );
        case PG_DIN_A8:
            return QString::fromLatin1( "A8" );
        case PG_DIN_A9:
            return QString::fromLatin1( "A9" );
        case PG_DIN_B0:
            return QString::fromLatin1( "B0" );
        case PG_DIN_B1:
            return QString::fromLatin1( "B1" );
        case PG_DIN_B10:
            return QString::fromLatin1( "B10" );
        case PG_DIN_B2:
            return QString::fromLatin1( "B2" );
        case PG_DIN_B3:
            return QString::fromLatin1( "B3" );
        case PG_DIN_B4:
            return QString::fromLatin1( "B4" );
        case PG_DIN_B6:
            return QString::fromLatin1( "B6" );
    }
    return QString::fromLatin1( "A4" );   // let's make Tru64's cxx happy
}

KoFormat KoPageFormat::formatFromString( const QString & string )
{
    if ( string == "A3" )
        return PG_DIN_A3;
    if ( string == "A4" )
        return PG_DIN_A4;
    if ( string == "A5" )
        return PG_DIN_A5;
    if ( string == "Letter" )
        return PG_US_LETTER;
    if ( string == "Legal" )
        return PG_US_LEGAL;
    if ( string == "Screen" )
        return PG_SCREEN;
    if ( string == "Custom" )
        return PG_CUSTOM;
    if ( string == "B5" )
        return PG_DIN_B5;
    if ( string == "Executive" )
        return PG_US_EXECUTIVE;
    if ( string == "A0" )
        return PG_DIN_A0;
    if ( string == "A1" )
        return PG_DIN_A1;
    if ( string == "A2" )
        return PG_DIN_A2;
    if ( string == "A6" )
        return PG_DIN_A6;
    if ( string == "A7" )
        return PG_DIN_A7;
    if ( string == "A8" )
        return PG_DIN_A8;
    if ( string == "A9" )
        return PG_DIN_A9;
    if ( string == "B0" )
        return PG_DIN_B0;
    if ( string == "B1" )
        return PG_DIN_B1;
    if ( string == "B10" )
        return PG_DIN_B10;
    if ( string == "B2" )
        return PG_DIN_B2;
    if ( string == "B3" )
        return PG_DIN_B3;
    if ( string == "B4" )
        return PG_DIN_B4;
    if ( string == "B6" )
        return PG_DIN_B6;
    // We do not know the format name, so we have a custom format
    return PG_CUSTOM;
}

QString KoPageFormat::name( KoFormat format )
{
    switch( format ) {
        case PG_DIN_A3:
            return i18n("DIN A3");
        case PG_DIN_A4:
            return i18n("DIN A4");
        case PG_DIN_A5:
            return i18n("DIN A5");
        case PG_US_LETTER:
            return i18n("US Letter");
        case PG_US_LEGAL:
            return i18n("US Legal");
        case PG_SCREEN:
            return i18n("Screen");
        case PG_CUSTOM:
            return i18n("Custom");
        case PG_DIN_B5:
            return i18n("DIN B5");
        case PG_US_EXECUTIVE:
            return i18n("US Executive");
        case PG_DIN_A0:
            return i18n( "DIN A0" );
        case PG_DIN_A1:
            return i18n( "DIN A1" );
        case PG_DIN_A2:
            return i18n( "DIN A2" );
        case PG_DIN_A6:
            return i18n( "DIN A6" );
        case PG_DIN_A7:
            return i18n( "DIN A7" );
        case PG_DIN_A8:
            return i18n( "DIN A8" );
        case PG_DIN_A9:
            return i18n( "DIN A9" );
        case PG_DIN_B0:
            return i18n( "DIN B0" );
        case PG_DIN_B1:
            return i18n( "DIN B1" );
        case PG_DIN_B10:
            return i18n( "DIN B10" );
        case PG_DIN_B2:
            return i18n( "DIN B2" );
        case PG_DIN_B3:
            return i18n( "DIN B3" );
        case PG_DIN_B4:
            return i18n( "DIN B4" );
        case PG_DIN_B6:
            return i18n( "DIN B6" );
    }
    return i18n("DIN A4");   // let's make Tru64's cxx happy
}

QStringList KoPageFormat::allFormats()
{
    QStringList lst;
    lst += i18n( "DIN A3" );
    lst += i18n( "DIN A4" );
    lst += i18n( "DIN A5" );
    lst += i18n( "US Letter" );
    lst += i18n( "US Legal" );
    lst += i18n( "Screen" );
    lst += i18n( "Custom" );
    lst += i18n( "DIN B5" );
    lst += i18n( "US Executive" );
    lst += i18n( "DIN A0" );
    lst += i18n( "DIN A1" );
    lst += i18n( "DIN A2" );
    lst += i18n( "DIN A6" );
    lst += i18n( "DIN A7" );
    lst += i18n( "DIN A8" );
    lst += i18n( "DIN A9" );
    lst += i18n( "DIN B0" );
    lst += i18n( "DIN B1" );
    lst += i18n( "DIN B10" );
    lst += i18n( "DIN B2" );
    lst += i18n( "DIN B3" );
    lst += i18n( "DIN B4" );
    lst += i18n( "DIN B6" );
    return lst;
}

int KoGlobal::s_pointSize = -1;
QStringList KoGlobal::s_languageList = QStringList();
QStringList KoGlobal::s_languageTag = QStringList();

QFont KoGlobal::defaultFont()
{
    QFont font = KGlobalSettings::generalFont();
    // we have to use QFontInfo, in case the font was specified with a pixel size
    if ( font.pointSize() == -1 )
    {
        // cache size into s_pointSize, since QFontInfo loads the font -> slow
        if ( s_pointSize == -1 )
            s_pointSize = QFontInfo(font).pointSize();
        Q_ASSERT( s_pointSize != -1 );
        font.setPointSize( s_pointSize );
    }
    //kdDebug()<<k_funcinfo<<"QFontInfo(font).pointSize() :"<<QFontInfo(font).pointSize()<<endl;
    //kdDebug()<<k_funcinfo<<"font.name() :"<<font.family ()<<endl;
    return font;
}

QStringList KoGlobal::listOfLanguage()
{
    if ( s_languageList.count()==0 )
    {
        QStringList alllang = KGlobal::dirs()->findAllResources("locale",
                                                                QString::fromLatin1("*/entry.desktop"));
        QStringList langlist=alllang;
        kdDebug()<<" alllang :"<<alllang.count()<<endl;
        for ( QStringList::ConstIterator it = langlist.begin();
              it != langlist.end(); ++it )
        {
            KSimpleConfig entry(*it);
            entry.setGroup("KCM Locale");
            QString name = entry.readEntry("Name",
                                           KGlobal::locale()->translate("without name"));

            QString tag = *it;
            int index = tag.findRev('/');
            tag = tag.left(index);
            index = tag.findRev('/');
            tag = tag.mid(index+1);
            s_languageList.append(name);
            s_languageTag.append(tag);
        }
    }
    return s_languageList;
}

QString KoGlobal::tagOfLanguage( const QString & _lang)
{
    int pos = s_languageList.findIndex( _lang );
    if ( pos != -1)
    {
        return s_languageTag[ pos ];
    }
    return QString::null;
}
