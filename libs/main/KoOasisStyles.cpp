/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoOasisStyles.h"

#include "KoDom.h"
#include "KoGenStyles.h"
#include "KoXmlNS.h"
#include "KoUnit.h"
#include "KoPictureShared.h"
#include "KoOasisLoadingContext.h"

#include <QtGui/QBrush>
#include <QtCore/QBuffer>
#include <QtGui/QPen>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <KoStyleStack.h>
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoStore.h>
#include <KoStoreDevice.h>

#include <math.h>

class KoOasisStyles::Private
{
public:
    QHash<QString /*family*/, QHash<QString /*name*/, KoXmlElement*> > customStyles;
    // auto-styles in content.xml
    QHash<QString /*family*/, QHash<QString /*name*/, KoXmlElement*> > contentAutoStyles;
    // auto-styles in styles.xml
    QHash<QString /*family*/, QHash<QString /*name*/, KoXmlElement*> > stylesAutoStyles;
    QHash<QString /*family*/, KoXmlElement*> defaultStyles;

    QHash<QString /*name*/, KoXmlElement*> styles; // page-layout, font-face etc.
    QHash<QString /*name*/, KoXmlElement*> masterPages;
    QHash<QString /*name*/, KoXmlElement*> listStyles;
    QHash<QString /*name*/, KoXmlElement*> drawStyles;

    KoXmlElement           officeStyle;
    KoXmlElement           layerSet;

    DataFormatsMap         dataFormats;
};

KoOasisStyles::KoOasisStyles()
    : d( new Private )
{
}

KoOasisStyles::~KoOasisStyles()
{
    foreach ( const QString& family, d->customStyles.keys() )
        qDeleteAll( d->customStyles[family] );
    foreach ( const QString& family, d->contentAutoStyles.keys() )
        qDeleteAll( d->contentAutoStyles[family] );
    foreach ( const QString& family, d->stylesAutoStyles.keys() )
        qDeleteAll( d->stylesAutoStyles[family] );
    qDeleteAll( d->defaultStyles );
    qDeleteAll( d->styles );
    qDeleteAll( d->masterPages );
    qDeleteAll( d->listStyles );
    qDeleteAll( d->drawStyles );
    delete d;
}

void KoOasisStyles::createStyleMap( const KoXmlDocument& doc, bool stylesDotXml )
{
   const KoXmlElement docElement  = doc.documentElement();
    // We used to have the office:version check here, but better let the apps do that
    KoXmlElement fontStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "font-face-decls" );

    if ( !fontStyles.isNull() ) {
        //kDebug(30003) <<"Starting reading in font-face-decls...";
        insertStyles( fontStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent );
    }// else
    //   kDebug(30003) <<"No items found";

    //kDebug(30003) <<"Starting reading in office:automatic-styles. stylesDotXml=" << stylesDotXml;

    KoXmlElement autoStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "automatic-styles" );
    if ( !autoStyles.isNull() ) {
        insertStyles( autoStyles, stylesDotXml ? AutomaticInStyles : AutomaticInContent );
    }// else
    //    kDebug(30003) <<"No items found";


    //kDebug(30003) <<"Reading in master styles";

    KoXmlNode masterStyles = KoDom::namedItemNS( docElement, KoXmlNS::office, "master-styles" );
    if ( !masterStyles.isNull() ) {
        KoXmlElement master;
        forEachElement( master, masterStyles )
        {
            if ( master.localName() == "master-page" &&
                 master.namespaceURI() == KoXmlNS::style ) {
                const QString name = master.attributeNS( KoXmlNS::style, "name", QString() );
                kDebug(30003) <<"Master style: '" << name <<"' loaded";
                d->masterPages.insert( name, new KoXmlElement( master ) );
            } else if( master.localName() == "layer-set" && master.namespaceURI() == KoXmlNS::draw ) {
                kDebug(30003) <<"Master style: layer-set loaded";
                d->layerSet = master;
            } else
                // OASIS docu mentions style:handout-master and draw:layer-set here
                kWarning(30003) << "Unknown tag " << master.tagName() << " in office:master-styles" << endl;
        }
    }


    kDebug(30003) <<"Starting reading in office:styles";

    const KoXmlElement officeStyle = KoDom::namedItemNS( docElement, KoXmlNS::office, "styles" );
    if ( !officeStyle.isNull() ) {
        d->officeStyle = officeStyle;
        insertOfficeStyles( officeStyle );
    }

    //kDebug(30003) <<"Styles read in.";
}

QHash<QString, KoXmlElement*> KoOasisStyles::customStyles(const QString& family) const
{
    if ( family.isNull() )
        return QHash<QString, KoXmlElement*>();
    return d->customStyles.value( family );
}

QHash<QString, KoXmlElement*> KoOasisStyles::autoStyles(const QString& family, bool stylesDotXml ) const
{
    if ( family.isNull() )
        return QHash<QString, KoXmlElement*>();
    return stylesDotXml ? d->stylesAutoStyles.value( family ) : d->contentAutoStyles.value( family );
}

const KoOasisStyles::DataFormatsMap& KoOasisStyles::dataFormats() const
{
    return d->dataFormats;
}

void KoOasisStyles::insertOfficeStyles( const KoXmlElement& styles )
{
    KoXmlElement e;
    forEachElement( e, styles )
    {
        const QString localName = e.localName();
        const QString ns = e.namespaceURI();
        if ( ( ns == KoXmlNS::svg && (
                   localName == "linearGradient"
                   || localName == "radialGradient" ) )
             || ( ns == KoXmlNS::draw && (
                      localName == "gradient"
                      || localName == "hatch"
                      || localName == "fill-image"
                      || localName == "marker"
                      || localName == "stroke-dash"
                      || localName == "opacity" ) )
             )
        {
            const QString name = e.attributeNS( KoXmlNS::draw, "name", QString() );
            Q_ASSERT( !name.isEmpty() );
            KoXmlElement* ep = new KoXmlElement( e );
            d->drawStyles.insert( name, ep );
        }
        else
            insertStyle( e, CustomInStyles );
    }
}


void KoOasisStyles::insertStyles( const KoXmlElement& styles, TypeAndLocation typeAndLocation )
{
    //kDebug(30003) <<"Inserting styles from" << styles.tagName();
    KoXmlElement e;
    forEachElement( e, styles )
        insertStyle( e, typeAndLocation );
}

void KoOasisStyles::insertStyle( const KoXmlElement& e, TypeAndLocation typeAndLocation )
{
    const QString localName = e.localName();
    const QString ns = e.namespaceURI();

    const QString name = e.attributeNS( KoXmlNS::style, "name", QString() );
    if ( ns == KoXmlNS::style && localName == "style" ) {
        const QString family = e.attributeNS( KoXmlNS::style, "family", QString() );

        if ( typeAndLocation == AutomaticInContent ) {
            QHash<QString, KoXmlElement*>& dict = d->contentAutoStyles[ family ];
            if ( dict.contains( name ) )
            {
                kDebug(30003) <<"Auto-style: '" << name <<"' already exists";
                delete dict.take( name );
            }
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else if ( typeAndLocation == AutomaticInStyles ) {
            QHash<QString, KoXmlElement*>& dict = d->stylesAutoStyles[ family ];
            if ( dict.contains( name ) )
            {
                kDebug(30003) <<"Auto-style: '" << name <<"' already exists";
                delete dict.take( name );
            }
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) <<"Style: '" << name <<"' loaded as a style auto style";
        } else {
            QHash<QString, KoXmlElement*>& dict = d->customStyles[ family ];
            if ( dict.contains( name ) )
            {
                kDebug(30003) <<"Style: '" << name <<"' already exists";
                delete dict.take( name );
            }
            dict.insert( name, new KoXmlElement( e ) );
            //kDebug(30003) <<"Style: '" << name <<"' loaded";
        }
    } else if ( ns == KoXmlNS::style && (
                localName == "page-layout"
             || localName == "font-face"
             || localName == "presentation-page-layout" ) )
    {
        if ( d->styles.contains( name ) )
        {
            kDebug(30003) <<"Style: '" << name <<"' already exists";
            delete d->styles.take( name );
        }
        d->styles.insert( name, new KoXmlElement( e ) );
    } else if ( localName == "default-style" && ns == KoXmlNS::style ) {
        const QString family = e.attributeNS( KoXmlNS::style, "family", QString() );
        if ( !family.isEmpty() )
            d->defaultStyles.insert( family, new KoXmlElement( e ) );
    } else if ( localName == "list-style" && ns == KoXmlNS::text ) {
        d->listStyles.insert( name, new KoXmlElement( e ) );
        //kDebug(30003) <<"List style: '" << name <<"' loaded";
    } else if ( ns == KoXmlNS::number && (
                   localName == "number-style"
                || localName == "currency-style"
                || localName == "percentage-style"
                || localName == "boolean-style"
                || localName == "text-style"
                || localName == "date-style"
                || localName == "time-style" ) ) {
        importDataStyle( e );
    }
    // The rest (text:*-configuration and text:outline-style) is to be done by the apps.
}

// OO spec 2.5.4. p68. Conversion to Qt format: see qdate.html
// OpenCalcImport::loadFormat has similar code, but slower, intermixed with other stuff,
// lacking long-textual forms.
void KoOasisStyles::importDataStyle( const KoXmlElement& parent )
{
    NumericStyleFormat dataStyle;

    const QString localName = parent.localName();
    if (localName == "number-style")
      dataStyle.type = NumericStyleFormat::Number;
    else if (localName == "currency-style")
      dataStyle.type = NumericStyleFormat::Currency;
    else if (localName == "percentage-style")
      dataStyle.type = NumericStyleFormat::Percentage;
    else if (localName == "boolean-style")
      dataStyle.type = NumericStyleFormat::Boolean;
    else if (localName == "text-style")
      dataStyle.type = NumericStyleFormat::Text;
    else if (localName == "date-style")
      dataStyle.type = NumericStyleFormat::Date;
    else if (localName == "time-style")
      dataStyle.type = NumericStyleFormat::Time;

    QString format;
    int precision = -1;
    int leadingZ  = 1;
#ifdef __GNUC__
#warning Nothing changes thousandsSep - dead constant
#endif
    bool thousandsSep = false;
    //todo negred
    //bool negRed = false;
    bool ok = false;
    int i = 0;
    KoXmlElement e;
    QString prefix;
    QString suffix;
    forEachElement( e, parent )
    {
        if ( e.namespaceURI() != KoXmlNS::number )
            continue;
        QString localName = e.localName();
        const QString numberStyle = e.attributeNS( KoXmlNS::number, "style", QString() );
        const bool shortForm = numberStyle == "short" || numberStyle.isEmpty();
        if ( localName == "day" ) {
            format += shortForm ? "d" : "dd";
        } else if ( localName == "day-of-week" ) {
            format += shortForm ? "ddd" : "dddd";
        } else if ( localName == "month" ) {
            if ( e.attributeNS( KoXmlNS::number, "possessive-form", QString() ) == "true" ) {
                format += shortForm ? "PPP" : "PPPP";
            }
            // TODO the spec has a strange mention of number:format-source
            else if ( e.attributeNS( KoXmlNS::number, "textual", QString() ) == "true" ) {
                format += shortForm ? "MMM" : "MMMM";
            } else { // month number
                format += shortForm ? "M" : "MM";
            }
        } else if ( localName == "year" ) {
            format += shortForm ? "yy" : "yyyy";
        } else if ( localName == "era" ) {
            //TODO I don't know what is it... (define into oo spec)
        } else if ( localName == "week-of-year" || localName == "quarter") {
            // ### not supported in Qt
        } else if ( localName == "hours" ) {
            format += shortForm ? "h" : "hh";
        } else if ( localName == "minutes" ) {
            format += shortForm ? "m" : "mm";
        } else if ( localName == "seconds" ) {
            format += shortForm ? "s" : "ss";
        } else if ( localName == "am-pm" ) {
            format += "ap";
        } else if ( localName == "text" ) { // literal
            format += e.text();
        } else if ( localName == "suffix" ) {
            suffix = e.text();
            kDebug(30003)<<" suffix :"<<suffix;
        } else if ( localName == "prefix" ) {
            prefix = e.text();
            kDebug(30003)<<" prefix :"<<prefix;
        } else if ( localName == "currency-symbol" ) {
            dataStyle.currencySymbol = e.text();
            kDebug(30003)<<" currency-symbol:"<<dataStyle.currencySymbol;
            format += e.text();
            //TODO
            // number:language="de" number:country="DE">€</number:currency-symbol>
            // Stefan: localization of the symbol?
        } else if ( localName == "number" ) {
            // TODO: number:grouping="true"
            if ( e.hasAttributeNS( KoXmlNS::number, "decimal-places" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "decimal-places", QString() ).toInt( &ok );
                if ( ok )
                    precision = d;
            }
            if ( e.hasAttributeNS( KoXmlNS::number, "min-integer-digits" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "min-integer-digits", QString() ).toInt( &ok );
                if ( ok )
                    leadingZ = d;
            }
            if ( thousandsSep && leadingZ <= 3 )
            {
                format += "#,";
                for ( i = leadingZ; i <= 3; ++i )
                    format += '#';
            }
            for ( i = 1; i <= leadingZ; ++i )
            {
                format +=  '0';
                if ( ( i % 3 == 0 ) && thousandsSep )
                    format =+ ',' ;
            }
            if (precision > -1)
            {
                format += '.';
                for ( i = 0; i < precision; ++i )
                    format += '0';
            }
        }
        else if ( localName == "scientific-number" ) {
            if (dataStyle.type == NumericStyleFormat::Number)
                dataStyle.type = NumericStyleFormat::Scientific;
            int exp = 2;

            if ( e.hasAttributeNS( KoXmlNS::number, "decimal-places" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "decimal-places", QString() ).toInt( &ok );
                if ( ok )
                    precision = d;
            }

            if ( e.hasAttributeNS( KoXmlNS::number, "min-integer-digits" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "min-integer-digits", QString() ).toInt( &ok );
                if ( ok )
                    leadingZ = d;
            }

            if ( e.hasAttributeNS( KoXmlNS::number, "min-exponent-digits" ) )
            {
                int d = e.attributeNS( KoXmlNS::number, "min-exponent-digits", QString() ).toInt( &ok );
                if ( ok )
                    exp = d;
                if ( exp <= 0 )
                    exp = 1;
            }

            if ( thousandsSep && leadingZ <= 3 )
            {
                format += "#,";
                for ( i = leadingZ; i <= 3; ++i )
                    format += '#';
            }

            for ( i = 1; i <= leadingZ; ++i )
            {
                format+='0';
                if ( ( i % 3 == 0 ) && thousandsSep )
                    format+=',';
            }

            if (precision > -1)
            {
                format += '.';
                for ( i = 0; i < precision; ++i )
                    format += '0';
            }

            format+="E+";
            for ( i = 0; i < exp; ++i )
                format+='0';
        } else if ( localName == "fraction" ) {
                if (dataStyle.type == NumericStyleFormat::Number)
                    dataStyle.type = NumericStyleFormat::Fraction;
                int integer = 0;
                int numerator = 1;
                int denominator = 1;
                int denominatorValue=0;
                if ( e.hasAttributeNS( KoXmlNS::number, "min-integer-digits" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "min-integer-digits", QString() ).toInt( &ok );
                    if ( ok )
                        integer = d;
                }
                if ( e.hasAttributeNS( KoXmlNS::number, "min-numerator-digits" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "min-numerator-digits", QString() ).toInt( &ok );
                    if ( ok )
                        numerator = d;
                }
                if ( e.hasAttributeNS( KoXmlNS::number, "min-denominator-digits" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "min-denominator-digits", QString() ).toInt( &ok );
                    if ( ok )
                        denominator = d;
                }
                if ( e.hasAttributeNS( KoXmlNS::number, "denominator-value" ) )
                {
                    int d = e.attributeNS( KoXmlNS::number, "denominator-value", QString() ).toInt( &ok );
                    if ( ok )
                        denominatorValue = d;
                }

                for ( i = 0; i < integer; ++i )
                    format+='#';

                format+=' ';

                for ( i = 0; i < numerator; ++i )
                    format+='?';

                format+='/';

                if ( denominatorValue != 0 )
                    format+=QString::number( denominatorValue );
                else
                {
                    for ( i = 0; i < denominator; ++i )
                        format+='?';
                }
            }
        // Not needed:
        //  <style:map style:condition="value()&gt;=0" style:apply-style-name="N106P0"/>
        // we handle painting negative numbers in red differently

    }

    const QString styleName = parent.attributeNS( KoXmlNS::style, "name", QString() );
    kDebug(30003) <<"data style:" << styleName <<" qt format=" << format;
    if ( !prefix.isEmpty() )
    {
        kDebug(30003)<<" format.left( prefix.length() ) :"<<format.left( prefix.length() )<<" prefix :"<<prefix;
        if ( format.left( prefix.length() )==prefix )
        {
            format = format.right( format.length()-prefix.length() );
        }
        else
            prefix.clear();
    }
    if ( !suffix.isEmpty() )
    {
        kDebug(30003)<<"format.right( suffix.length() ) :"<<format.right( suffix.length() )<<" suffix :"<<suffix;
        if ( format.right( suffix.length() )==suffix )
        {
            format = format.left( format.length()-suffix.length() );
        }
        else
            suffix.clear();
    }

    dataStyle.formatStr=format;
    dataStyle.prefix=prefix;
    dataStyle.suffix=suffix;
    dataStyle.precision = precision;
    kDebug(30003)<<" finish insert format :"<<format<<" prefix :"<<prefix<<" suffix :"<<suffix;
    d->dataFormats.insert( styleName, dataStyle );
}

#define addTextNumber( text, elementWriter ) { \
        if ( !text.isEmpty() ) \
        { \
            elementWriter.startElement( "number:text" ); \
            elementWriter.addTextNode( text ); \
            elementWriter.endElement(); \
            text=""; \
        } \
}

void KoOasisStyles::parseOasisTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text )
{
    kDebug(30003)<<"parseOasisTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text ) :"<<format;
    do
    {
        if ( !saveOasisKlocaleTimeFormat( elementWriter, format, text ) )
        {
            text += format[0];
            format = format.remove( 0, 1 );
        }
    }
    while ( format.length() > 0 );
    addTextNumber( text, elementWriter );
}

bool KoOasisStyles::saveOasisKlocaleTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text )
{
    bool changed = false;
    if ( format.startsWith( "%H" ) ) //hh
    {
        //hour in 24h
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "%k" ) )//h
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "%I" ) )// ?????
    {
        //TODO hour in 12h
        changed = true;
    }
    else if ( format.startsWith( "%l" ) )
    {
        //TODO hour in 12h with 1 digit
        changed = true;
    }
    else if ( format.startsWith( "%M" ) )// mm
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:minutes" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;

    }
    else if ( format.startsWith( "%S" ) ) //ss
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:seconds" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( "%p" ) )
    {
        //TODO am or pm
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:am-pm" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    return changed;
}


bool KoOasisStyles::saveOasisTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text, bool &antislash )
{
    bool changed = false;
    //we can also add time to date.
    if ( antislash )
    {
        text+=format[0];
        format = format.remove( 0, 1 );
        antislash = false;
        changed = true;
    }
    else if ( format.startsWith( "hh" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( 'h' ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:hours" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 1 );
        changed = true;
    }
    else if ( format.startsWith( "mm" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:minutes" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( 'm' ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:minutes" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 1 );
        changed = true;
    }
    else if ( format.startsWith( "ss" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:seconds" );
        elementWriter.addAttribute( "number:style", "long" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    else if ( format.startsWith( 's' ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:seconds" );
        elementWriter.addAttribute( "number:style", "short" );
        elementWriter.endElement();
        format = format.remove( 0, 1 );
        changed = true;
    }
    else if ( format.startsWith( "ap" ) )
    {
        addTextNumber( text, elementWriter );

        elementWriter.startElement( "number:am-pm" );
        elementWriter.endElement();
        format = format.remove( 0, 2 );
        changed = true;
    }
    return changed;
}

QString KoOasisStyles::saveOasisTimeStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat,
                                           const QString & _prefix, const QString & _suffix )
{
    Q_UNUSED(_prefix);
    Q_UNUSED(_suffix);
    kDebug(30003)<<"QString KoOasisStyles::saveOasisTimeStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );
    KoGenStyle currentStyle( KoGenStyle::StyleNumericTime );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    if ( klocaleFormat )
    {
        parseOasisTimeKlocale( elementWriter, format, text );
    }
    else
    {
        bool antislash = false;
        do
        {
            if ( !saveOasisTimeFormat( elementWriter, format, text, antislash ) )
            {
                QString elem( format[0] );
                format = format.remove( 0, 1 );
                if ( elem == "\\" )
                {
                     antislash = true;
                }
                else
                {
                    text += elem;
                    antislash = false;
                }
            }
        }
        while ( format.length() > 0 );
        addTextNumber( text, elementWriter );
    }
    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

//convert klocale string to good format
void KoOasisStyles::parseOasisDateKlocale(KoXmlWriter &elementWriter, QString & format, QString & text )
{
    kDebug(30003)<<"KoOasisStyles::parseOasisDateKlocale(KoXmlWriter &elementWriter, QString & format, QString & text ) :"<<format;
    do
    {
        if ( format.startsWith( "%Y" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:year" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%y" ) )
        {

            addTextNumber( text, elementWriter );

            elementWriter.startElement( "number:year" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%n" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.addAttribute( "number:textual", "false");
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%m" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.addAttribute( "number:textual", "false"); //not necessary remove it
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%e" ) )
        {
            addTextNumber( text, elementWriter );

            elementWriter.startElement( "number:day" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%d" ) )
        {
            addTextNumber( text, elementWriter );

            elementWriter.startElement( "number:day" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%b" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.addAttribute( "number:textual", "true");
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%B" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:month" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.addAttribute( "number:textual", "true");
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%a" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:day-of-week" );
            elementWriter.addAttribute( "number:style", "short" );
            elementWriter.endElement();

            format = format.remove( 0, 2 );
        }
        else if ( format.startsWith( "%A" ) )
        {
            addTextNumber( text, elementWriter );
            elementWriter.startElement( "number:day-of-week" );
            elementWriter.addAttribute( "number:style", "long" );
            elementWriter.endElement();
            format = format.remove( 0, 2 );
        }
        else
        {
            if ( !saveOasisKlocaleTimeFormat( elementWriter, format, text ) )
            {
                text += format[0];
                format = format.remove( 0, 1 );
            }
        }
    }
    while ( format.length() > 0 );
    addTextNumber( text, elementWriter );
}

QString KoOasisStyles::saveOasisDateStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat,
                                           const QString & _prefix, const QString & _suffix )
{
    Q_UNUSED(_prefix);
    Q_UNUSED(_suffix);
    kDebug(30003)<<"QString KoOasisStyles::saveOasisDateStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );

    // Not supported into Qt: "era" "week-of-year" "quarter"

    KoGenStyle currentStyle( KoGenStyle::StyleNumericDate );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    if ( klocaleFormat )
    {
        parseOasisDateKlocale( elementWriter, format, text );
    }
    else
    {
        bool antislash = false;
        do
        {
            if ( antislash )
            {
                text+=format[0];
                format = format.remove( 0, 1 );
            }
            //TODO implement loading ! What is it ?
            else if ( format.startsWith( "MMMMM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:textual", "true");
                elementWriter.endElement();
                format = format.remove( 0, 5 );
            }
            else if ( format.startsWith( "MMMM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.addAttribute( "number:textual", "true");
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "MMM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "true");
                elementWriter.endElement();
                format = format.remove( 0, 3 );
            }
            else if ( format.startsWith( "MM" ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.addAttribute( "number:textual", "false"); //not necessary remove it
                elementWriter.endElement();
                format = format.remove( 0, 2 );
            }
            else if ( format.startsWith( 'M' ) )
            {
                addTextNumber( text, elementWriter );
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "false");
                elementWriter.endElement();
                format = format.remove( 0, 1 );
            }
            else if ( format.startsWith( "PPPP" ) )
            {
                addTextNumber( text, elementWriter );
                //<number:month number:possessive-form="true" number:textual="true" number:style="long"/>
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "false");
                elementWriter.addAttribute( "number:possessive-form", "true" );
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "PPP" ) )
            {
                addTextNumber( text, elementWriter );
                //<number:month number:possessive-form="true" number:textual="true" number:style="short"/>
                elementWriter.startElement( "number:month" );
                elementWriter.addAttribute( "number:possessive-form", "true" );

                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.addAttribute( "number:textual", "false");
                elementWriter.endElement();
                format = format.remove( 0, 3 );
            }
            else if ( format.startsWith( "dddd" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day-of-week" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "ddd" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day-of-week" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.endElement();
                format = format.remove( 0, 3 );
            }
            else if ( format.startsWith( "dd" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.endElement();
                format = format.remove( 0, 2 );
            }
            else if ( format.startsWith( 'd' ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:day" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.endElement();
                format = format.remove( 0, 1 );
            }
            else if ( format.startsWith( "yyyy" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:year" );
                elementWriter.addAttribute( "number:style", "long" );
                elementWriter.endElement();
                format = format.remove( 0, 4 );
            }
            else if ( format.startsWith( "yy" ) )
            {
                addTextNumber( text, elementWriter );

                elementWriter.startElement( "number:year" );
                elementWriter.addAttribute( "number:style", "short" );
                elementWriter.endElement();
                format = format.remove( 0, 2 );
            }
            else
            {
                if ( !saveOasisTimeFormat( elementWriter, format, text,antislash ) )
                {
                    QString elem( format[0] );
                    format = format.remove( 0, 1 );
                    if ( elem == "\\" )
                    {
                        antislash = true;
                    }
                    else
                    {
                        text += elem;
                        antislash = false;
                    }
                }
            }
        }
        while ( format.length() > 0 );
        addTextNumber( text, elementWriter );
    }

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}


QString KoOasisStyles::saveOasisFractionStyle( KoGenStyles &mainStyles, const QString & _format,
                                               const QString &_prefix, const QString &_suffix )
{
    kDebug(30003)<<"QString saveOasisFractionStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::StyleNumericFraction );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int integer = 0;
    int numerator = 0;
    int denominator = 0;
    int denominatorValue = 0;
    bool beforeSlash = true;
    do
    {
        if ( format[0]=='#' )
            integer++;
        else if ( format[0]=='/' )
            beforeSlash = false;
        else if ( format[0]=='?' )
        {
            if ( beforeSlash )
                numerator++;
            else
                denominator++;
        }
        else
        {
            bool ok;
            int value = format.toInt( &ok );
            if ( ok )
            {
                denominatorValue=value;
                break;
            }
        }
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );

    text= _prefix;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:fraction" );
    elementWriter.addAttribute( "number:min-integer-digits", integer );
    elementWriter.addAttribute( "number:min-numerator-digits",numerator );
    elementWriter.addAttribute( "number:min-denominator-digits",denominator );
    if ( denominatorValue != 0 )
        elementWriter.addAttribute( "number:denominator-value",denominatorValue );
    elementWriter.endElement();

    addKofficeNumericStyleExtension( elementWriter, _suffix, _prefix );

    text=_suffix;
    addTextNumber(text, elementWriter );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}


QString KoOasisStyles::saveOasisNumberStyle( KoGenStyles &mainStyles, const QString & _format,
                                             const QString &_prefix, const QString &_suffix )
{
    kDebug(30003)<<"QString saveOasisNumberStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::StyleNumericNumber );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do
    {
        if ( format[0]=='.' || format[0]==',' )
            beforeSeparator = false;
        else if ( format[0]=='0' && beforeSeparator )
            integerdigits++;
        else if ( format[0]=='0' && !beforeSeparator )
            decimalplaces++;
        else
            kDebug(30003)<<" error format 0";
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text= _prefix ;
    addTextNumber(text, elementWriter );
    elementWriter.startElement( "number:number" );
    kDebug(30003)<<" decimalplaces :"<<decimalplaces<<" integerdigits :"<<integerdigits;
    if (!beforeSeparator)
        elementWriter.addAttribute( "number:decimal-places", decimalplaces );
    elementWriter.addAttribute( "number:min-integer-digits", integerdigits );
    elementWriter.endElement();

    text =_suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

QString KoOasisStyles::saveOasisPercentageStyle( KoGenStyles &mainStyles, const QString & _format,
                                                 const QString &_prefix, const QString &_suffix )
{
    //<number:percentage-style style:name="N11">
    //<number:number number:decimal-places="2" number:min-integer-digits="1"/>
    //<number:text>%</number:text>
    //</number:percentage-style>

    kDebug(30003)<<"QString saveOasisPercentageStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::StyleNumericPercentage );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do
    {
        if ( format[0]=='.' || format[0]==',' )
            beforeSeparator = false;
        else if ( format[0]=='0' && beforeSeparator )
            integerdigits++;
        else if ( format[0]=='0' && !beforeSeparator )
            decimalplaces++;
        else
            kDebug(30003)<<" error format 0";
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text= _prefix ;
    addTextNumber(text, elementWriter );
    elementWriter.startElement( "number:number" );
    if (!beforeSeparator)
        elementWriter.addAttribute( "number:decimal-places", decimalplaces );
    elementWriter.addAttribute( "number:min-integer-digits", integerdigits );
    elementWriter.endElement();

    addTextNumber(QString( "%" ), elementWriter );

    text =_suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );

}

QString KoOasisStyles::saveOasisScientificStyle( KoGenStyles &mainStyles, const QString & _format,
                                                 const QString &_prefix, const QString &_suffix )
{
    //<number:number-style style:name="N60">
    //<number:scientific-number number:decimal-places="2" number:min-integer-digits="1" number:min-exponent-digits="3"/>
    //</number:number-style>

    //example 000,000e+0000
    kDebug(30003)<<"QString saveOasisScientificStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::StyleNumericScientific );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    int decimalplace = 0;
    int integerdigits = 0;
    int exponentdigits = 0;
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    bool beforeSeparator = true;
    bool exponential = false;
    bool positive = true;
    do
    {
        if ( !exponential )
        {
            if ( format[0]=='0' && beforeSeparator )
                integerdigits++;
            else if ( format[0]==',' || format[0]=='.' )
                beforeSeparator = false;
            else if (  format[0]=='0' && !beforeSeparator )
                decimalplace++;
            else if ( format[0].toLower()=='e' )
            {
                format.remove( 0, 1 );
                if ( format[0]=='+' )
                    positive = true;
                else if ( format[0]=='-' )
                    positive = false;
                else
                    kDebug(30003)<<"Error into scientific number";
                exponential = true;
            }
        }
        else
        {
            if ( format[0]=='0' && positive )
                exponentdigits++;
            else if ( format[0]=='0' && !positive )
                exponentdigits--;
            else
                kDebug(30003)<<" error into scientific number exponential value";
        }
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text =  _prefix ;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:scientific-number" );
    kDebug(30003)<<" decimalplace :"<<decimalplace<<" integerdigits :"<<integerdigits<<" exponentdigits :"<<exponentdigits;
    if (!beforeSeparator)
        elementWriter.addAttribute( "number:decimal-places", decimalplace );
    elementWriter.addAttribute( "number:min-integer-digits",integerdigits );
    elementWriter.addAttribute( "number:min-exponent-digits",exponentdigits );
    elementWriter.endElement();

    text = _suffix;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

QString KoOasisStyles::saveOasisCurrencyStyle( KoGenStyles &mainStyles,
                                               const QString & _format, const QString &symbol,
                                               const QString &_prefix, const QString &_suffix )
{

    //<number:currency-style style:name="N107P0" style:volatile="true">
    //<number:number number:decimal-places="2" number:min-integer-digits="1" number:grouping="true"/>
    //<number:text> </number:text>
    //<number:currency-symbol>VEB</number:currency-symbol>
    //</number:currency-style>

    kDebug(30003)<<"QString saveOasisCurrencyStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::StyleNumericCurrency );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    int decimalplaces = 0;
    int integerdigits = 0;
    bool beforeSeparator = true;
    do
    {
        if ( format[0]=='.' || format[0]==',' )
            beforeSeparator = false;
        else if ( format[0]=='0' && beforeSeparator )
            integerdigits++;
        else if ( format[0]=='0' && !beforeSeparator )
            decimalplaces++;
        else
            kDebug(30003)<<" error format 0";
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );

    text =  _prefix ;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:number" );
    kDebug(30003)<<" decimalplaces :"<<decimalplaces<<" integerdigits :"<<integerdigits;
    if (!beforeSeparator)
      elementWriter.addAttribute( "number:decimal-places", decimalplaces );
    elementWriter.addAttribute( "number:min-integer-digits", integerdigits );
    elementWriter.endElement();

    text =  _suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );

    elementWriter.startElement( "number:currency-symbol" );
    kDebug(30003)<<" currency-symbol:"<<symbol;
    elementWriter.addTextNode( symbol.toUtf8() );
    elementWriter.endElement();

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

QString KoOasisStyles::saveOasisTextStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix, const QString &_suffix )
{

    //<number:text-style style:name="N100">
    //<number:text-content/>
    ///</number:text-style>

    kDebug(30003)<<"QString saveOasisTextStyle( KoGenStyles &mainStyles, const QString & _format ) :"<<_format;
    QString format( _format );

    KoGenStyle currentStyle( KoGenStyle::StyleNumericText );
    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
    QString text;
    do
    {
        format.remove( 0,1 );
    }
    while ( format.length() > 0 );
    text =  _prefix ;
    addTextNumber(text, elementWriter );

    elementWriter.startElement( "number:text-style" );

    text =  _suffix ;
    addTextNumber(text, elementWriter );
    addKofficeNumericStyleExtension( elementWriter, _suffix,_prefix );
    elementWriter.endElement();

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    currentStyle.addChildElement( "number", elementContents );
    return mainStyles.lookup( currentStyle, "N" );
}

//This is an extension of numeric style. For the moment we used namespace of 
//oasis format for specific koffice extension. Change it for the future.
void KoOasisStyles::addKofficeNumericStyleExtension( KoXmlWriter & elementWriter, const QString &_suffix, const QString &_prefix )
 {
     if ( !_suffix.isEmpty() )
     {
         elementWriter.startElement( "number:suffix" );
         elementWriter.addTextNode( _suffix );
         elementWriter.endElement();
     }
     if ( !_prefix.isEmpty() )
     {
         elementWriter.startElement( "number:prefix" );
         elementWriter.addTextNode( _prefix );
         elementWriter.endElement();
     }
}

void KoOasisStyles::saveOasisFillStyle( KoGenStyle &styleFill, KoGenStyles& mainStyles, const QBrush & brush )
{
    switch( brush.style() )
    {
    case Qt::SolidPattern:
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::Dense1Pattern:
        styleFill.addProperty( "draw:transparency", "94%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::Dense2Pattern:
        styleFill.addProperty( "draw:transparency", "88%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::Dense3Pattern:
        styleFill.addProperty( "draw:transparency", "63%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::Dense4Pattern:
        styleFill.addProperty( "draw:transparency", "50%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::Dense5Pattern:
        styleFill.addProperty( "draw:transparency", "37%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::Dense6Pattern:
        styleFill.addProperty( "draw:transparency", "12%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::Dense7Pattern:
        styleFill.addProperty( "draw:transparency", "6%" );
        styleFill.addProperty( "draw:fill","solid" );
        styleFill.addProperty( "draw:fill-color", brush.color().name() );
        break;
    case Qt::LinearGradientPattern:
    case Qt::RadialGradientPattern:
    case Qt::ConicalGradientPattern:
        styleFill.addProperty( "draw:fill","gradient" );
        styleFill.addProperty( "draw:fill-gradient-name", saveOasisGradientStyle( mainStyles, brush ) );
        break;
    default: //otherstyle
        styleFill.addProperty( "draw:fill","hatch" );
        styleFill.addProperty( "draw:fill-hatch-name", saveOasisHatchStyle( mainStyles, brush ) );
        break;
    }
}

QString KoOasisStyles::saveOasisHatchStyle( KoGenStyles& mainStyles, const QBrush &brush )
{
    KoGenStyle hatchStyle( KoGenStyle::StyleHatch /*no family name*/);
    hatchStyle.addAttribute( "draw:color", brush.color().name() );
    //hatchStyle.addAttribute( "draw:distance", m_distance ); not implemented into kpresenter
    switch( brush.style() )
    {
    case Qt::HorPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 0);
        break;
    case Qt::BDiagPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 450);
        break;
    case Qt::VerPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 900);
        break;
    case Qt::FDiagPattern:
        hatchStyle.addAttribute( "draw:style", "single" );
        hatchStyle.addAttribute( "draw:rotation", 1350);
        break;
    case Qt::CrossPattern:
        hatchStyle.addAttribute( "draw:style", "double" );
        hatchStyle.addAttribute( "draw:rotation", 0);
        break;
    case Qt::DiagCrossPattern:
        hatchStyle.addAttribute( "draw:style", "double" );
        hatchStyle.addAttribute( "draw:rotation", 450);
        break;
    default:
        break;
    }

    return mainStyles.lookup( hatchStyle, "hatch" );
}

QString KoOasisStyles::saveOasisGradientStyle( KoGenStyles &mainStyles, const QBrush &brush )
{
    KoGenStyle gradientStyle;
    if( brush.style() == Qt::RadialGradientPattern )
    {
        const QRadialGradient *gradient = static_cast<const QRadialGradient*>( brush.gradient() );
        gradientStyle = KoGenStyle( KoGenStyle::StyleGradientRadial /*no family name*/ );
        gradientStyle.addAttribute( "draw:style", "radial" );
        gradientStyle.addAttributePt( "svg:cx", gradient->center().x() );
        gradientStyle.addAttributePt( "svg:cy", gradient->center().y() );
        gradientStyle.addAttributePt( "svg:r",  gradient->radius() );
        gradientStyle.addAttributePt( "svg:fx", gradient->focalPoint().x() );
        gradientStyle.addAttributePt( "svg:fy", gradient->focalPoint().y() );
    }
    else if( brush.style() == Qt::LinearGradientPattern )
    {
        const QLinearGradient *gradient = static_cast<const QLinearGradient*>( brush.gradient() );
        gradientStyle = KoGenStyle( KoGenStyle::StyleGradientLinear /*no family name*/ );
        gradientStyle.addAttribute( "draw:style", "linear" );
        gradientStyle.addAttributePt( "svg:x1", gradient->start().x() );
        gradientStyle.addAttributePt( "svg:y1", gradient->start().y() );
        gradientStyle.addAttributePt( "svg:x2", gradient->finalStop().x() );
        gradientStyle.addAttributePt( "svg:y2", gradient->finalStop().y() );
    }
    const QGradient * gradient = brush.gradient();
    if( gradient->spread() == QGradient::RepeatSpread )
        gradientStyle.addAttribute( "svg:spreadMethod", "repeat" );
    else if( gradient->spread() == QGradient::ReflectSpread )
        gradientStyle.addAttribute( "svg:spreadMethod", "reflect" );
    else
        gradientStyle.addAttribute( "svg:spreadMethod", "pad" );

    QBuffer buffer;
    buffer.open( QIODevice::WriteOnly );
    KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level

    // save stops
    QGradientStops stops = gradient->stops();
    foreach( QGradientStop stop, stops )
    {
        elementWriter.startElement( "svg:stop" );
        elementWriter.addAttribute( "svg:offset", QString( "%1" ).arg( stop.first ) );
        elementWriter.addAttribute( "svg:color", stop.second.name() );
        if( stop.second.alphaF() < 1.0 )
            elementWriter.addAttribute( "svg:stop-opacity", QString( "%1" ).arg( stop.second.alphaF() ) );
        elementWriter.endElement();
    }

    QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
    gradientStyle.addChildElement( "svg:stop", elementContents );

    return mainStyles.lookup( gradientStyle, "gradient" );
}

QBrush KoOasisStyles::loadOasisGradientStyle( const KoStyleStack &styleStack, const KoOasisStyles & oasisStyles, const QSizeF &size )
{
    QString styleName = styleStack.property( KoXmlNS::draw, "fill-gradient-name" );

    KoXmlElement* e = oasisStyles.drawStyles()[styleName];
    if( ! e )
        return QBrush();

    QGradient * gradient = 0;

    if( e->namespaceURI() == KoXmlNS::draw && e->localName() == "gradient" )
    {
        // FIXME seems like oo renders the gradient start stop color at the center of the
        // radial gradient, and the start color at the radius of the radial gradient
        // whereas it is not mentioned in the spec how it should be rendered
        // note that svg defines that exactly as the opposite as oo does
        // so what should we do?
        QString type = e->attributeNS( KoXmlNS::draw, "style", QString() );
        if( type == "radial" )
        {
            QRadialGradient * rg = new QRadialGradient();
            // TODO : find out whether Oasis works with boundingBox only?
            double cx = KoUnit::parseValue( e->attributeNS( KoXmlNS::draw, "cx", QString() ).remove("%") );
            double cy = KoUnit::parseValue( e->attributeNS( KoXmlNS::draw, "cy", QString() ).remove("%") );
            rg->setCenter( QPointF( size.width() * 0.01 * cx, size.height() * 0.01 * cy ) );
            rg->setFocalPoint( rg->center() );
            double dx = 0.5 * size.width();
            double dy = 0.5 * size.height();
            rg->setRadius( sqrt( dx*dx + dy*dy ) );
            gradient = rg;
        }
        else if( type == "linear" )
        {
            QLinearGradient * lg = new QLinearGradient();
            double angle = 90 + e->attributeNS( KoXmlNS::draw, "angle", "0" ).toDouble();
            double radius = 0.5 * sqrt( size.width()*size.width() + size.height()*size.height() );
            double sx = cos( angle * M_PI / 180 ) * radius;
            double sy = sin( angle * M_PI / 180 ) * radius;
            lg->setStart( QPointF( 0.5 * size.width() + sx, 0.5 * size.height() + sy ) );
            lg->setFinalStop( QPointF( 0.5 * size.width() - sx, 0.5 * size.height() - sy ) );
            gradient = lg;
        }
        else
            return QBrush();

        QGradientStop start;
        start.first = 0.0;
        start.second = QColor( e->attributeNS( KoXmlNS::draw, "start-color", QString() ) );
        start.second.setAlphaF( 0.01 * e->attributeNS( KoXmlNS::draw, "start-intensity", "100" ).remove("%").toDouble() );

        QGradientStop end;
        end.first = 1.0;
        end.second = QColor( e->attributeNS( KoXmlNS::draw, "end-color", QString() ) );
        end.second.setAlphaF( 0.01 * e->attributeNS( KoXmlNS::draw, "end-intensity", "100" ).remove("%").toDouble() );

        QGradientStops stops;
        gradient->setStops( stops << start << end );
    }
    else if( e->namespaceURI() == KoXmlNS::svg )
    {
        if( e->localName() == "linearGradient" )
        {
            QLinearGradient * lg = new QLinearGradient();
            QPointF start, stop;
            start.setX( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "x1", QString() ) ) );
            start.setY( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "y1", QString() ) ) );
            stop.setX( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "x2", QString() ) ) );
            stop.setY( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "y2", QString() ) ) );
            lg->setStart( start );
            lg->setFinalStop( stop );
            gradient = lg;
        }
        else if( e->localName() == "radialGradient" )
        {
            QRadialGradient * rg = new QRadialGradient();
            QPointF center, focalPoint;
            center.setX( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "cx", QString() ) ) );
            center.setY( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "cy", QString() ) ) );
            double r = KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "r", QString() ) );
            focalPoint.setX( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "fx", QString() ) ) );
            focalPoint.setY( KoUnit::parseValue( e->attributeNS( KoXmlNS::svg, "fy", QString() ) ) );
            rg->setCenter( center );
            rg->setFocalPoint( focalPoint );
            rg->setRadius( r );
            gradient = rg;
        }

        QString strSpread( e->attributeNS( KoXmlNS::svg, "spreadMethod", "pad" ) );
        if( strSpread == "repeat" )
            gradient->setSpread( QGradient::RepeatSpread );
        else if( strSpread == "reflect" )
            gradient->setSpread( QGradient::ReflectSpread );
        else
            gradient->setSpread( QGradient::PadSpread );

        QGradientStops stops;

        // load stops
        KoXmlElement colorstop;
        forEachElement(colorstop, (*e))
        {
            if( colorstop.namespaceURI() == KoXmlNS::svg && colorstop.localName() == "stop" )
            {
                QGradientStop stop;
                stop.second = QColor( colorstop.attributeNS( KoXmlNS::svg, "color", QString() ) );
                stop.second.setAlphaF( colorstop.attributeNS( KoXmlNS::svg, "stop-opacity", "1.0" ).toDouble() );
                stop.first = colorstop.attributeNS( KoXmlNS::svg, "offset", "0.0" ).toDouble();
                stops.append( stop );
            }
        }
        // TODO should the stops be sorted?
        gradient->setStops( stops );
    }

    if( ! gradient )
        return QBrush();

    QBrush resultBrush( *gradient );
    delete gradient;
    return resultBrush;
}

QBrush KoOasisStyles::loadOasisFillStyle( const KoStyleStack &styleStack, const QString & fill, const KoOasisStyles & oasisStyles )
{
    QBrush tmpBrush;
    if ( fill == "solid" )
    {
        tmpBrush.setStyle(static_cast<Qt::BrushStyle>( 1 ) );
        if ( styleStack.hasProperty( KoXmlNS::draw, "fill-color" ) )
            tmpBrush.setColor(styleStack.property( KoXmlNS::draw, "fill-color" ) );
        if ( styleStack.hasProperty( KoXmlNS::draw, "transparency" ) )
        {
            QString transparency = styleStack.property( KoXmlNS::draw, "transparency" );
            if ( transparency == "94%" )
            {
                tmpBrush.setStyle(Qt::Dense1Pattern);
            }
            else if ( transparency == "88%" )
            {
                tmpBrush.setStyle(Qt::Dense2Pattern);
            }
            else if ( transparency == "63%" )
            {
                tmpBrush.setStyle(Qt::Dense3Pattern);

            }
            else if ( transparency == "50%" )
            {
                tmpBrush.setStyle(Qt::Dense4Pattern);

            }
            else if ( transparency == "37%" )
            {
                tmpBrush.setStyle(Qt::Dense5Pattern);

            }
            else if ( transparency == "12%" )
            {
                tmpBrush.setStyle(Qt::Dense6Pattern);

            }
            else if ( transparency == "6%" )
            {
                tmpBrush.setStyle(Qt::Dense7Pattern);

            }
            else
                kDebug(30003)<<" transparency is not defined into kpresenter :"<<transparency;
        }
    }
    else if ( fill == "hatch" )
    {
        QString style = styleStack.property( KoXmlNS::draw, "fill-hatch-name" );
        kDebug(30003)<<" hatch style is  :"<<style;

        //type not defined by default
        //try to use style.
        KoXmlElement* draw = oasisStyles.drawStyles()[style];
        if ( draw)
        {
            kDebug(30003)<<"We have a style";
            int angle = 0;
            if( draw->hasAttributeNS( KoXmlNS::draw, "rotation" ))
            {
                angle = (draw->attributeNS( KoXmlNS::draw, "rotation", QString() ).toInt())/10;
                kDebug(30003)<<"angle :"<<angle;
            }
            if(draw->hasAttributeNS( KoXmlNS::draw, "color" ) )
            {
                //kDebug(30003)<<" draw:color :"<<draw->attributeNS( KoXmlNS::draw,"color", QString() );
                tmpBrush.setColor(draw->attributeNS( KoXmlNS::draw, "color", QString() ) );
            }
            if( draw->hasAttributeNS( KoXmlNS::draw, "distance" ))
            {
                //todo implemente it into kpresenter
            }
            if( draw->hasAttributeNS( KoXmlNS::draw, "display-name"))
            {
                //todo implement it into kpresenter
            }
            if( draw->hasAttributeNS( KoXmlNS::draw, "style" ))
            {
                //todo implemente it into kpresenter
                QString styleHash = draw->attributeNS( KoXmlNS::draw, "style", QString() );
                if( styleHash == "single")
                {
                    switch( angle )
                    {
                    case 0:
                    case 180:
                        tmpBrush.setStyle(Qt::HorPattern );
                        break;
                    case 45:
                    case 225:
                        tmpBrush.setStyle(Qt::BDiagPattern );
                        break;
                    case 90:
                    case 270:
                        tmpBrush.setStyle(Qt::VerPattern );
                        break;
                    case 135:
                    case 315:
                        tmpBrush.setStyle(Qt::FDiagPattern );
                        break;
                    default:
                        //todo fixme when we will have a kopaint
                        kDebug(30003)<<" draw:rotation 'angle' :"<<angle;
                        break;
                    }
                }
                else if( styleHash == "double")
                {
                    switch( angle )
                    {
                    case 0:
                    case 180:
                    case 90:
                    case 270:
                        tmpBrush.setStyle(Qt::CrossPattern );
                        break;
                    case 45:
                    case 135:
                    case 225:
                    case 315:
                        tmpBrush.setStyle(Qt::DiagCrossPattern );
                        break;
                    default:
                        //todo fixme when we will have a kopaint
                        kDebug(30003)<<" draw:rotation 'angle' :"<<angle;
                        break;
                    }

                }
                else if( styleHash == "triple")
                {
                    kDebug(30003)<<" it is not implemented :(";
                }
            }
        }
    }

    return tmpBrush;
}

QBrush KoOasisStyles::loadOasisPatternStyle( const KoStyleStack &styleStack, KoOasisLoadingContext & context, const QSizeF &size )
{
    QString styleName = styleStack.property( KoXmlNS::draw, "fill-image-name" );

    KoXmlElement* e = context.oasisStyles().drawStyles()[styleName];
    if( ! e )
        return QBrush();

    const QString href = e->attributeNS( KoXmlNS::xlink, "href", QString() );

    if( href.isEmpty() )
        return QBrush();

    QString strExtension;
    const int result=href.lastIndexOf(".");
    if (result>=0)
    {
        strExtension=href.mid(result+1); // As we are using KoPicture, the extension should be without the dot.
    }
    QString filename(href);

    KoPictureShared picture;
    KoStore* store = context.store();
    if ( store->open( filename ) )
    {
        KoStoreDevice dev(store);
        if ( ! picture.load( &dev, strExtension ) )
            kWarning() << "Cannot load picture: " << filename << " " << href << endl;
        store->close();
    }

    // read the pattern repeat style
    QString style = styleStack.property( KoXmlNS::style, "repeat" );
    kDebug() <<"pattern style =" << style;

    QSize imageSize = picture.getOriginalSize();

    if( style == "stretch" )
    {
        imageSize = size.toSize();
    }
    else
    {
        // optional attributes which can override original image size
        if( styleStack.hasProperty( KoXmlNS::draw, "fill-image-height" ) && styleStack.hasProperty( KoXmlNS::draw, "fill-image-width" ) )
        {
            QString height = styleStack.property( KoXmlNS::draw, "fill-image-height" );
            double newHeight = 0.0;
            if( height.endsWith( '%' ) )
                newHeight = 0.01 * height.remove( "%" ).toDouble() * imageSize.height();
            else
                newHeight = KoUnit::parseValue( height );
            QString width = styleStack.property( KoXmlNS::draw, "fill-image-width" );
            double newWidth = 0.0;
            if( width.endsWith( '%' ) )
                newWidth = 0.01 * width.remove( "%" ).toDouble() * imageSize.width();
            else
                newWidth = KoUnit::parseValue( width );
            if( newHeight > 0.0 )
                imageSize.setHeight( static_cast<int>( newHeight ) );
            if( newWidth > 0.0 )
                imageSize.setWidth( static_cast<int>( newWidth ) );
        }
    }

    kDebug() <<"shape size =" << size;
    kDebug() <<"original image size =" << picture.getOriginalSize();
    kDebug() <<"resulting image size =" << imageSize; 

    QBrush resultBrush( picture.generatePixmap( imageSize, true ) );

    if( style == "repeat" )
    {
        QMatrix matrix;
        if( styleStack.hasProperty( KoXmlNS::draw, "fill-image-ref-point" ) )
        {
            // align pattern to the given size
            QString align = styleStack.property( KoXmlNS::draw, "fill-image-ref-point" );
            kDebug() <<"pattern align =" << align;
            if( align == "top-left" )
                matrix.translate( 0, 0 );
            else if( align == "top" )
                matrix.translate( 0.5*size.width(), 0 );
            else if( align == "top-right" )
                matrix.translate( size.width(), 0 );
            else if( align == "left" )
                matrix.translate( 0, 0.5*size.height() );
            else if( align == "center" )
                matrix.translate( 0.5*size.width(), 0.5*size.height() );
            else if( align == "right" )
                matrix.translate( size.width(), 0.5*size.height() );
            else if( align == "bottom-left" )
                matrix.translate( 0, size.height() );
            else if( align == "bottom" )
                matrix.translate( 0.5*size.width(), size.height() );
            else if( align == "bottom-right" )
                matrix.translate( size.width(), size.height() );
        }
        if( styleStack.hasProperty( KoXmlNS::draw, "fill-image-ref-point-x" ) )
        {
            QString pointX = styleStack.property( KoXmlNS::draw, "fill-image-ref-point-x" );
            matrix.translate( 0.01 * pointX.remove( '%' ).toDouble() * imageSize.width(), 0 );
        }
        if( styleStack.hasProperty( KoXmlNS::draw, "fill-image-ref-point-y" ) )
        {
            QString pointY = styleStack.property( KoXmlNS::draw, "fill-image-ref-point-y" );
            matrix.translate( 0, 0.01 * pointY.remove( '%' ).toDouble() * imageSize.height() );
        }
        resultBrush.setMatrix( matrix );
    }

    return resultBrush;
}

QPen KoOasisStyles::loadOasisStrokeStyle( const KoStyleStack &styleStack, const QString & stroke, const KoOasisStyles & oasisStyles )
{
    QPen tmpPen;

    if( stroke == "solid" || stroke == "dash" )
    {
        if ( styleStack.hasProperty( KoXmlNS::svg, "stroke-color" ) )
            tmpPen.setColor( styleStack.property( KoXmlNS::svg, "stroke-color" ) );
        if ( styleStack.hasProperty( KoXmlNS::svg, "stroke-opacity" ) )
        {
            QColor color = tmpPen.color();
            QString opacity = styleStack.property( KoXmlNS::svg, "stroke-opacity" );
            color.setAlphaF( opacity.toDouble() );
            tmpPen.setColor( color );
        }
        if( styleStack.hasProperty( KoXmlNS::svg, "stroke-width" ) )
            tmpPen.setWidthF( KoUnit::parseValue( styleStack.property( KoXmlNS::svg, "stroke-width" ) ) );
        if( styleStack.hasProperty( KoXmlNS::draw, "stroke-linejoin" ) )
        {
            QString join = styleStack.property( KoXmlNS::draw, "stroke-linejoin" );
            if( join == "bevel" )
                tmpPen.setJoinStyle( Qt::BevelJoin );
            else if( join == "round" )
                tmpPen.setJoinStyle( Qt::RoundJoin );
            else
                tmpPen.setJoinStyle( Qt::MiterJoin );
        }

        if( stroke == "dash" && styleStack.hasProperty( KoXmlNS::draw, "stroke-dash" ) )
        {
            QString dashStyleName = styleStack.property( KoXmlNS::draw, "stroke-dash" );

            KoXmlElement * dashElement = oasisStyles.drawStyles()[ dashStyleName ];
            if( dashElement )
            {
                QVector<qreal> dashes;
                if( dashElement->hasAttributeNS( KoXmlNS::draw, "dots1" ) )
                {
                    double dotLength = KoUnit::parseValue( dashElement->attributeNS( KoXmlNS::draw, "dots1-length", QString() ) );
                    dashes.append( dotLength / tmpPen.width() );
                    double dotDistance = KoUnit::parseValue( dashElement->attributeNS( KoXmlNS::draw, "distance", QString() ) );
                    dashes.append( dotDistance / tmpPen.width() );
                    if( dashElement->hasAttributeNS( KoXmlNS::draw, "dots2" ) )
                    {
                        dotLength = KoUnit::parseValue( dashElement->attributeNS( KoXmlNS::draw, "dots2-length", QString() ) );
                        dashes.append( dotLength / tmpPen.width() );
                        dashes.append( dotDistance / tmpPen.width() );
                    }
                    tmpPen.setDashPattern( dashes );
                }
            }
        }
    }

    return tmpPen;
}

const KoXmlElement* KoOasisStyles::defaultStyle( const QString& family ) const
{
    return d->defaultStyles[family];
}

const KoXmlElement& KoOasisStyles::officeStyle() const
{
    return d->officeStyle;
}

const KoXmlElement& KoOasisStyles::layerSet() const
{
    return d->layerSet;
}

const QHash<QString, KoXmlElement*>& KoOasisStyles::listStyles() const
{
    return d->listStyles;
}

const QHash<QString, KoXmlElement*>& KoOasisStyles::masterPages() const
{
    return d->masterPages;
}

const QHash<QString, KoXmlElement*>& KoOasisStyles::drawStyles() const
{
    return d->drawStyles;
}

const KoXmlElement* KoOasisStyles::findStyle( const QString& name ) const
{
    return d->styles[ name ];
}

const KoXmlElement* KoOasisStyles::findStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = findStyleCustomStyle( styleName, family );
    if ( !style )
        style = findStyleAutoStyle( styleName, family );
    if ( !style )
        style = findContentAutoStyle( styleName, family );
    return style;
}

const KoXmlElement* KoOasisStyles::findStyleCustomStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->customStyles.value( family ).value( styleName );
    if ( style && !family.isEmpty() ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString() );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KoXmlElement* KoOasisStyles::findStyleAutoStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->stylesAutoStyles.value( family ).value( styleName );
    if ( style ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString() );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}

const KoXmlElement* KoOasisStyles::findContentAutoStyle( const QString& styleName, const QString& family ) const
{
    const KoXmlElement* style = d->contentAutoStyles.value( family ).value( styleName );
    if ( style ) {
        const QString styleFamily = style->attributeNS( KoXmlNS::style, "family", QString() );
        if ( styleFamily != family ) {
            kWarning() << "KoOasisStyles: was looking for style " << styleName
                        << " in family " << family << " but got " << styleFamily << endl;
        }
    }
    return style;
}
