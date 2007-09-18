/* This file is part of the KDE project
   Copyright (C) 2002, 2003, The Karbon Developers
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#include "svgimport.h"
#include "color.h"

#include <vglobal.h>

#include <KoShape.h>
#include <KoShapeLayer.h>
#include <KoShapeContainer.h>
#include <KoShapeGroup.h>
#include <KoPathShape.h>
#include <KoPathShapeLoader.h>
#include <commands/KoShapeGroupCommand.h>
#include <KoFilterChain.h>
#include <KoStoreDevice.h>
#include <KoOasisStore.h>
#include <KoGenStyles.h>
#include <KoUnit.h>
#include <KoGlobal.h>
#include <KoImageData.h>
#include <pictureshape/PictureShape.h>
#include <pathshapes/rectangle/KoRectangleShape.h>
#include <pathshapes/ellipse/KoEllipseShape.h>
#include <text/TextShape.h>

#include <kgenericfactory.h>
#include <kdebug.h>
#include <kfilterdev.h>

#include <QtGui/QColor>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>

K_PLUGIN_FACTORY( SvgImportFactory, registerPlugin<SvgImport>(); )
K_EXPORT_PLUGIN( SvgImportFactory( "kofficefilters" ) )

SvgImport::SvgImport(QObject*parent, const QVariantList&)
    : KoFilter(parent)
{
}

SvgImport::~SvgImport()
{
    qDeleteAll( m_gc );
    m_gc.clear();
}

KoFilter::ConversionStatus SvgImport::convert(const QByteArray& from, const QByteArray& to)
{
    // check for proper conversion
    if( to != "application/vnd.oasis.opendocument.graphics" || from != "image/svg+xml" )
        return KoFilter::NotImplemented;

    //Find the last extension
    QString strExt;
    QString fileIn ( m_chain->inputFile() );
    const int result=fileIn.lastIndexOf('.');
    if (result>=0)
        strExt=fileIn.mid(result).toLower();

    QString strMime; // Mime type of the compressor
    if ((strExt==".gz")      //in case of .svg.gz (logical extension)
        ||(strExt==".svgz")) //in case of .svgz (extension used prioritary)
        strMime="application/x-gzip"; // Compressed with gzip
    else if (strExt==".bz2") //in case of .svg.bz2 (logical extension)
        strMime="application/x-bzip2"; // Compressed with bzip2
    else
        strMime="text/plain";

    /*kDebug(30514) <<"File extension: -" << strExt <<"- Compression:" << strMime;*/

    QIODevice* in = KFilterDev::deviceForFile(fileIn,strMime);

    if (!in->open(QIODevice::ReadOnly))
    {
        kError(30514) << "Cannot open file! Aborting!" << endl;
        delete in;
        return KoFilter::FileNotFound;
    }

    int line, col;
    QString errormessage;

    const bool parsed=inpdoc.setContent( in, &errormessage, &line, &col );

    in->close();
    delete in;

    if ( ! parsed )
    {
        kError(30514) << "Error while parsing file: "
                << "at line " << line << " column: " << col
                << " message: " << errormessage << endl;
        // ### TODO: feedback to the user
        return KoFilter::ParsingError;
    }

    // Do the conversion!
    convert();

    // create output store
    KoStore* storeout = KoStore::createStore( m_chain->outputFile(), KoStore::Write, to, KoStore::Zip );

    if ( !storeout )
    {
        kWarning() << "Couldn't open the requested file.";
        return KoFilter::FileNotFound;
    }

    // Tell KoStore not to touch the file names
    storeout->disallowNameExpansion();
    KoOasisStore oasisStore( storeout );
    KoXmlWriter* manifestWriter = oasisStore.manifestWriter( to );

    KoGenStyles mainStyles;

    bool success = m_document.saveOasis( storeout, manifestWriter, mainStyles );

    // cleanup
    oasisStore.closeManifestWriter();
    delete storeout;

    if( ! success )
        return KoFilter::CreationError;
    else
        return KoFilter::OK;
}

void SvgImport::convert()
{
    SvgGraphicsContext *gc = new SvgGraphicsContext;
    QDomElement docElem = inpdoc.documentElement();

    QRectF viewBox;

    if( ! docElem.attribute( "viewBox" ).isEmpty() )
    {
        // allow for viewbox def with ',' or whitespace
        QString viewbox = docElem.attribute( "viewBox" );
        QStringList points = viewbox.replace( ',', ' ').simplified().split( ' ' );
        viewBox.setWidth( points[2].toFloat() );
        viewBox.setHeight( points[3].toFloat() );
    }

    double width = !docElem.attribute( "width" ).isEmpty() ? parseUnit( docElem.attribute( "width" ), true, false, viewBox ) : 550.0;
    double height = !docElem.attribute( "height" ).isEmpty() ? parseUnit( docElem.attribute( "height" ), false, true, viewBox ) : 841.0;
    m_document.setPageSize( QSizeF( width, height ) );
    m_outerRect = QRectF( QPointF(0,0), m_document.pageSize() );

    if( ! docElem.attribute( "viewBox" ).isEmpty() )
    {
        gc->matrix.scale( width / viewBox.width() , height / viewBox.height() );
        m_outerRect.setWidth( m_outerRect.width() * ( viewBox.width() / width ) );
        m_outerRect.setHeight( m_outerRect.height() * ( viewBox.height() / height ) );
    }

    /*
    if( ! docElem.attribute( "viewBox" ).isEmpty() )
    {
        // allow for viewbox def with ',' or whitespace
        QString viewbox( docElem.attribute( "viewBox" ) );
        QStringList points = viewbox.replace( ',', ' ').simplified().split( ' ' );

        gc->matrix.scale( width / points[2].toFloat() , height / points[3].toFloat() );
        m_outerRect.setWidth( m_outerRect.width() * ( points[2].toFloat() / width ) );
        m_outerRect.setHeight( m_outerRect.height() * ( points[3].toFloat() / height ) );
    }
    */

    m_gc.push( gc );
    QList<KoShape*> shapes = parseGroup( docElem );
    foreach( KoShape * shape, shapes )
        m_document.add( shape );

    KoShapeLayer * layer = 0;
    // check if we have to insert a default layer
    if( m_document.layers().count() == 0 )
    {
        layer = new KoShapeLayer();
        m_document.insertLayer( layer );
    }
    else
        layer = m_document.layers().first();

    // add all toplevel shapes to the layer
    foreach( KoShape * shape, m_document.shapes() )
    {
        if( ! shape->parent() )
            layer->addChild( shape );
    }
}

#define DPI 90

// Helper functions
// ---------------------------------------------------------------------------------------

double SvgImport::toPercentage( QString s )
{
    if( s.endsWith( '%' ) )
        return s.remove( '%' ).toDouble();
    else
        return s.toDouble() * 100.0;
}

double SvgImport::fromPercentage( QString s )
{
    if( s.endsWith( '%' ) )
        return s.remove( '%' ).toDouble() / 100.0;
    else
        return s.toDouble();
}

double SvgImport::getScalingFromMatrix( QMatrix &matrix )
{
    double xscale = matrix.m11() + matrix.m12();
    double yscale = matrix.m22() + matrix.m21();
    return sqrt( xscale*xscale + yscale*yscale ) / sqrt( 2.0 );
}

// parses the number into parameter number
const char * getNumber( const char *ptr, double &number )
{
    int integer, exponent;
    double decimal, frac;
    int sign, expsign;

    exponent = 0;
    integer = 0;
    frac = 1.0;
    decimal = 0;
    sign = 1;
    expsign = 1;

    // read the sign
    if(*ptr == '+')
        ptr++;
    else if(*ptr == '-')
    {
        ptr++;
        sign = -1;
    }

    // read the integer part
    while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
        integer = (integer * 10) + *(ptr++) - '0';
    if(*ptr == '.') // read the decimals
    {
        ptr++;
        while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
            decimal += (*(ptr++) - '0') * (frac *= 0.1);
    }

    if(*ptr == 'e' || *ptr == 'E') // read the exponent part
    {
        ptr++;

        // read the sign of the exponent
        if(*ptr == '+')
            ptr++;
        else if(*ptr == '-')
        {
            ptr++;
            expsign = -1;
        }

        exponent = 0;
        while(*ptr != '\0' && *ptr >= '0' && *ptr <= '9')
        {
            exponent *= 10;
            exponent += *ptr - '0';
            ptr++;
        }
    }
    number = integer + decimal;
    number *= sign * pow( (double)10, double( expsign * exponent ) );

    return ptr;
}

void SvgImport::addGraphicContext()
{
    SvgGraphicsContext *gc = new SvgGraphicsContext;
    // set as default
    if( m_gc.top() )
        *gc = *( m_gc.top() );
    m_gc.push( gc );
}

void SvgImport::removeGraphicContext()
{
    delete( m_gc.pop() );
}

void SvgImport::setupTransform( const QDomElement &e )
{
    SvgGraphicsContext *gc = m_gc.top();

    QMatrix mat = parseTransform( e.attribute( "transform" ) );
    gc->matrix = mat * gc->matrix;
}

QMatrix SvgImport::parseTransform( const QString &transform )
{
    QMatrix result;

    // Split string for handling 1 transform statement at a time
    QStringList subtransforms = transform.split(')', QString::SkipEmptyParts);
    QStringList::ConstIterator it = subtransforms.begin();
    QStringList::ConstIterator end = subtransforms.end();
    for(; it != end; ++it)
    {
        QStringList subtransform = (*it).split('(', QString::SkipEmptyParts);

        subtransform[0] = subtransform[0].trimmed().toLower();
        subtransform[1] = subtransform[1].simplified();
        QRegExp reg("[,( ]");
        QStringList params = subtransform[1].split(reg, QString::SkipEmptyParts);

        if(subtransform[0].startsWith(';') || subtransform[0].startsWith(','))
            subtransform[0] = subtransform[0].right(subtransform[0].length() - 1);

        if(subtransform[0] == "rotate")
        {
            if(params.count() == 3)
            {
                double x = params[1].toDouble();
                double y = params[2].toDouble();

                result.translate(x, y);
                result.rotate(params[0].toDouble());
                result.translate(-x, -y);
            }
            else
                result.rotate(params[0].toDouble());
        }
        else if(subtransform[0] == "translate")
        {
            if(params.count() == 2)
                result.translate(params[0].toDouble(), params[1].toDouble());
            else    // Spec : if only one param given, assume 2nd param to be 0
                result.translate(params[0].toDouble() , 0);
        }
        else if(subtransform[0] == "scale")
        {
            if(params.count() == 2)
                result.scale(params[0].toDouble(), params[1].toDouble());
            else    // Spec : if only one param given, assume uniform scaling
                result.scale(params[0].toDouble(), params[0].toDouble());
        }
        else if(subtransform[0] == "skewx")
            result.shear(tan(params[0].toDouble() * VGlobal::pi_180), 0.0F);
        else if(subtransform[0] == "skewy")
            result.shear(tan(params[0].toDouble() * VGlobal::pi_180), 0.0F);
        else if(subtransform[0] == "skewy")
            result.shear(0.0F, tan(params[0].toDouble() * VGlobal::pi_180));
        else if(subtransform[0] == "matrix")
        {
            if(params.count() >= 6)
                result.setMatrix(params[0].toDouble(), params[1].toDouble(), params[2].toDouble(), params[3].toDouble(), params[4].toDouble(), params[5].toDouble());
        }
    }

    return result;
}

KoShape * SvgImport::findObject( const QString &name, KoShapeContainer * group )
{
    if( ! group )
        return 0L;

    foreach( KoShape * shape, group->iterator() )
    {
        if( shape->name() == name )
            return shape;

        KoShapeContainer * container = dynamic_cast<KoShapeContainer*>( shape );
        if( container )
        {
            KoShape * resultShape = findObject( name, container );
            if( resultShape )
                return resultShape;
        }
    }

    return 0;
}

KoShape * SvgImport::findObject( const QString &name )
{
    foreach( KoShapeLayer * layer, m_document.layers() )
    {
        KoShape * shape = findObject( name, layer );
        if( shape )
            return shape;
    }

    return 0;
}

SvgGradientHelper* SvgImport::findGradient( const QString &id, const QString &href)
{
    // check if gradient was already parsed, and return it
    if( m_gradients.contains( id ) )
        return &m_gradients[ id ];

    // check if gradient was stored for later parsing
    if( !m_defs.contains( id ) )
        return 0L;

    QDomElement e = m_defs[ id ];
    if(e.childNodes().count() == 0)
    {
        QString mhref = e.attribute("xlink:href").mid(1);

        if(m_defs.contains(mhref))
            return findGradient(mhref, id);
        else
            return 0L;
    }
    else
    {
        // ok parse gradient now
        parseGradient( m_defs[ id ], m_defs[ href ] );
    }

    // return successfully parsed gradient or NULL
    QString n;
    if(href.isEmpty())
        n = id;
    else
        n = href;

    if( m_gradients.contains( n ) )
        return &m_gradients[ n ];
    else
        return 0L;
}

QDomElement SvgImport::mergeStyles( const QDomElement &referencedBy, const QDomElement &referencedElement )
{
    // First use all the style attributes of the element being referenced.
    QDomElement e = referencedElement;

    // Now go through the style attributes of the element that is referencing and substitute the original ones.
    if( !referencedBy.attribute( "color" ).isEmpty() )
        e.setAttribute( "color", referencedBy.attribute( "color" ) );
    if( !referencedBy.attribute( "fill" ).isEmpty() )
        e.setAttribute( "fill", referencedBy.attribute( "fill" ) );
    if( !referencedBy.attribute( "fill-rule" ).isEmpty() )
        e.setAttribute( "fill-rule", referencedBy.attribute( "fill-rule" ) );
    if( !referencedBy.attribute( "stroke" ).isEmpty() )
        e.setAttribute( "stroke", referencedBy.attribute( "stroke" ) );
    if( !referencedBy.attribute( "stroke-width" ).isEmpty() )
        e.setAttribute( "stroke-width", referencedBy.attribute( "stroke-width" ) );
    if( !referencedBy.attribute( "stroke-linejoin" ).isEmpty() )
        e.setAttribute( "stroke-linejoin", referencedBy.attribute( "stroke-linejoin" ) );
    if( !referencedBy.attribute( "stroke-linecap" ).isEmpty() )
        e.setAttribute( "stroke-linecap", referencedBy.attribute( "stroke-linecap" ) );
    if( !referencedBy.attribute( "stroke-dasharray" ).isEmpty() )
        e.setAttribute( "stroke-dasharray", referencedBy.attribute( "stroke-dasharray" ) );
    if( !referencedBy.attribute( "stroke-dashoffset" ).isEmpty() )
        e.setAttribute( "stroke-dashoffset", referencedBy.attribute( "stroke-dashoffset" ) );
    if( !referencedBy.attribute( "stroke-opacity" ).isEmpty() )
        e.setAttribute( "stroke-opacity", referencedBy.attribute( "stroke-opacity" ) );
    if( !referencedBy.attribute( "stroke-miterlimit" ).isEmpty() )
        e.setAttribute( "stroke-miterlimit", referencedBy.attribute( "stroke-miterlimit" ) );
    if( !referencedBy.attribute( "fill-opacity" ).isEmpty() )
        e.setAttribute( "fill-opacity", referencedBy.attribute( "fill-opacity" ) );
    if( !referencedBy.attribute( "opacity" ).isEmpty() )
        e.setAttribute( "opacity", referencedBy.attribute( "opacity" ) );

    // TODO merge style attribute too.

    return e;
}


// Parsing functions
// ---------------------------------------------------------------------------------------

double SvgImport::parseUnit( const QString &unit, bool horiz, bool vert, QRectF bbox )
{
    // TODO : percentage?
    double value = 0;
    const char *start = unit.toLatin1();
    if(!start) {
        return 0;
    }
    const char *end = getNumber( start, value );

    if( int( end - start ) < unit.length() )
    {
        if( unit.right( 2 ) == "pt" )
            value = ( value / 72.0 ) * DPI;
        else if( unit.right( 2 ) == "cm" )
            value = ( value / 2.54 ) * DPI;
        else if( unit.right( 2 ) == "pc" )
            value = ( value / 6.0 ) * DPI;
        else if( unit.right( 2 ) == "mm" )
            value = ( value / 25.4 ) * DPI;
        else if( unit.right( 2 ) == "in" )
            value = value * DPI;
        else if( unit.right( 2 ) == "pt" )
            value = ( value / 72.0 ) * DPI;
        else if( unit.right( 2 ) == "em" )
            value = value * m_gc.top()->font.pointSize() / ( sqrt( pow( m_gc.top()->matrix.m11(), 2 ) + pow( m_gc.top()->matrix.m22(), 2 ) ) / sqrt( 2.0 ) );
        else if( unit.right( 1 ) == "%" )
        {
            if( horiz && vert )
                value = ( value / 100.0 ) * (sqrt( pow( bbox.width(), 2 ) + pow( bbox.height(), 2 ) ) / sqrt( 2.0 ) );
            else if( horiz )
                value = ( value / 100.0 ) * bbox.width();
            else if( vert )
                value = ( value / 100.0 ) * bbox.height();
        }
    }
    /*else
    {
        if( m_gc.top() )
        {
            if( horiz && vert )
                value *= sqrt( pow( m_gc.top()->matrix.m11(), 2 ) + pow( m_gc.top()->matrix.m22(), 2 ) ) / sqrt( 2.0 );
            else if( horiz )
                value /= m_gc.top()->matrix.m11();
            else if( vert )
                value /= m_gc.top()->matrix.m22();
        }
    }*/
    return value;
}

QColor SvgImport::parseColor( const QString &rgbColor )
{
    int r, g, b;
    keywordToRGB( rgbColor, r, g, b );
    return QColor( r, g, b );
}

void SvgImport::parseColor( QColor &color, const QString &s )
{
    if( s.startsWith( "rgb(" ) )
    {
        QString parse = s.trimmed();
        QStringList colors = parse.split( ',' );
        QString r = colors[0].right( ( colors[0].length() - 4 ) );
        QString g = colors[1];
        QString b = colors[2].left( ( colors[2].length() - 1 ) );

        if( r.contains( "%" ) )
        {
            r = r.left( r.length() - 1 );
            r = QString::number( int( ( double( 255 * r.toDouble() ) / 100.0 ) ) );
        }

        if( g.contains( "%" ) )
        {
            g = g.left( g.length() - 1 );
            g = QString::number( int( ( double( 255 * g.toDouble() ) / 100.0 ) ) );
        }

        if( b.contains( "%" ) )
        {
            b = b.left( b.length() - 1 );
            b = QString::number( int( ( double( 255 * b.toDouble() ) / 100.0 ) ) );
        }

        color = QColor( r.toInt(), g.toInt(), b.toInt() );
    }
    else if( s == "currentColor" )
    {
        SvgGraphicsContext *gc = m_gc.top();
        color = gc->color;
    }
    else
    {
        QString rgbColor = s.trimmed();
        if( rgbColor.startsWith( '#' ) )
            color.setNamedColor( rgbColor );
        else
            color = parseColor( rgbColor );
    }
}

void SvgImport::parseColorStops( QGradient *gradient, const QDomElement &e )
{
    QGradientStops stops;
    QColor c;
    for( QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement stop = n.toElement();
        if( stop.tagName() == "stop" )
        {
            float offset;
            QString temp = stop.attribute( "offset" );
            if( temp.contains( '%' ) )
            {
                temp = temp.left( temp.length() - 1 );
                offset = temp.toFloat() / 100.0;
            }
            else
                offset = temp.toFloat();

            if( !stop.attribute( "stop-color" ).isEmpty() )
                parseColor( c, stop.attribute( "stop-color" ) );
            else
            {
                // try style attr
                QString style = stop.attribute( "style" ).simplified();
                QStringList substyles = style.split( ';', QString::SkipEmptyParts );
                for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
                {
                    QStringList substyle = it->split( ':' );
                    QString command	= substyle[0].trimmed();
                    QString params	= substyle[1].trimmed();
                    if( command == "stop-color" )
                        parseColor( c, params );
                    if( command == "stop-opacity" )
                        c.setAlphaF( params.toDouble() );
                }

            }
            if( !stop.attribute( "stop-opacity" ).isEmpty() )
                c.setAlphaF( stop.attribute( "stop-opacity" ).toDouble() );
            stops.append( QPair<qreal,QColor>(offset, c ) );
        }
    }
    if( stops.count() )
        gradient->setStops( stops );
}

void SvgImport::parseGradient( const QDomElement &e , const QDomElement &referencedBy)
{
    // IMPROVEMENTS:
    // - Store the parsed colorstops in some sort of a cache so they don't need to be parsed again.
    // - A gradient inherits attributes it does not have from the referencing gradient.
    // - Gradients with no color stops have no fill or stroke.
    // - Gradients with one color stop have a solid color.

    SvgGraphicsContext *gc = m_gc.top();
    if( !gc ) return;

    SvgGradientHelper gradhelper;

    if(e.childNodes().count() == 0)
    {
        QString href = e.attribute("xlink:href").mid(1);
        if(href.isEmpty())
        {
            //gc->fill.setType( VFill::none ); // <--- TODO Fill OR Stroke are none
            return;
        }
        else 
        {
            // copy the referenced gradient if found
            SvgGradientHelper * pGrad = findGradient( href );
            if( pGrad )
                gradhelper = *pGrad;
        }
    }

    // Use the gradient that is referencing, or if there isn't one, the original gradient.
    QDomElement b;
    if( !referencedBy.isNull() )
        b = referencedBy;
    else
        b = e;

    QString id = b.attribute("id");
    if( !id.isEmpty() )
    {
        // Copy existing gradient if it exists
        if( m_gradients.find( id ) != m_gradients.end() )
            gradhelper.copyGradient( m_gradients[ id ].gradient() );
    }

    gradhelper.setBoundboxUnits( b.attribute( "gradientUnits" ) != "userSpaceOnUse" );

    // parse color prop
    QColor c = m_gc.top()->color;

    if( !b.attribute( "color" ).isEmpty() )
    {
        parseColor( c, b.attribute( "color" ) );
    }
    else
    {
        // try style attr
        QString style = b.attribute( "style" ).simplified();
        QStringList substyles = style.split( ';', QString::SkipEmptyParts );
        for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
        {
            QStringList substyle = it->split( ':' );
            QString command	= substyle[0].trimmed();
            QString params	= substyle[1].trimmed();
            if( command == "color" )
                parseColor( c, params );
        }
    }
    m_gc.top()->color = c;

    if( b.tagName() == "linearGradient" )
    {
        QLinearGradient * g = new QLinearGradient();
        if( gradhelper.boundboxUnits() )
        {
            g->setStart( QPointF( toPercentage( b.attribute( "x1", "0%" ) ), toPercentage( b.attribute( "y1", "0%" ) ) ) );
            g->setFinalStop( QPointF( toPercentage( b.attribute( "x2", "100%" ) ), toPercentage( b.attribute( "y2", "0%" ) ) ) );
        }
        else
        {
            g->setStart( QPointF( b.attribute( "x1" ).toDouble(), b.attribute( "y1" ).toDouble() ) );
            g->setFinalStop( QPointF( b.attribute( "x2" ).toDouble(), b.attribute( "y2" ).toDouble() ) );
        }
        // preserve color stops
        if( gradhelper.gradient() )
            g->setStops( gradhelper.gradient()->stops() );
        gradhelper.setGradient( g );
    }
    else
    {
        QRadialGradient * g = new QRadialGradient();
        if( gradhelper.boundboxUnits() )
        {
            g->setCenter( QPointF( toPercentage( b.attribute( "cx", "50%" ) ), toPercentage( b.attribute( "cy", "50%" ) ) ) );
            g->setRadius( toPercentage( b.attribute( "r", "50%" ) ) );
            g->setFocalPoint( QPointF( toPercentage( b.attribute( "fx", "50%" ) ), toPercentage( b.attribute( "fy", "50%" ) ) ) );
        }
        else
        {
            g->setCenter( QPointF( b.attribute( "cx" ).toDouble(), b.attribute( "cy" ).toDouble() ) );
            g->setFocalPoint( QPointF( b.attribute( "fx" ).toDouble(), b.attribute( "fy" ).toDouble() ) );
            g->setRadius( b.attribute( "r" ).toDouble() );
        }
        // preserve color stops
        if( gradhelper.gradient() )
            g->setStops( gradhelper.gradient()->stops() );
        gradhelper.setGradient( g );
    }

    // handle spread method
    QString spreadMethod = b.attribute( "spreadMethod" );
    if( !spreadMethod.isEmpty() )
    {
        if( spreadMethod == "reflect" )
            gradhelper.gradient()->setSpread( QGradient::ReflectSpread );
        else if( spreadMethod == "repeat" )
            gradhelper.gradient()->setSpread( QGradient::RepeatSpread );
        else
            gradhelper.gradient()->setSpread( QGradient::PadSpread );
    }
    else
        gradhelper.gradient()->setSpread( QGradient::PadSpread );

    // Parse the color stops. The referencing gradient does not have colorstops, 
    // so use the stops from the gradient it references to (e in this case and not b)
    parseColorStops( gradhelper.gradient(), e );
    //gradient.setGradientTransform( parseTransform( e.attribute( "gradientTransform" ) ) );
    gradhelper.setTransform( parseTransform( b.attribute( "gradientTransform" ) ) );
    m_gradients.insert( b.attribute( "id" ), gradhelper );
}

void SvgImport::parsePA( KoShape *obj, SvgGraphicsContext *gc, const QString &command, const QString &params )
{
    QColor fillcolor = gc->fill.color();
    QColor strokecolor = gc->stroke.color();

    if( params == "inherit" ) 
        return;

    if( command == "fill" )
    {
        if( params == "none" )
            gc->fill.setStyle( Qt::NoBrush );
        else if( params.startsWith( "url(" ) )
        {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.lastIndexOf(')');
            QString key = params.mid( start, end - start );
            SvgGradientHelper * gradHelper = findGradient( key );
            if( gradHelper && obj )
            {
                if( gradHelper->boundboxUnits() )
                {
                    // adjust to bbox
                    QRectF bbox = QRectF( QPoint(), obj->size() );
                    gc->fill = gradHelper->adjustedFill( bbox );
                    gc->fill.setMatrix( gradHelper->transform() );
                }
                else
                {
                    gc->fill = QBrush( *gradHelper->gradient() );
                    gc->fill.setMatrix( gradHelper->transform() * gc->matrix * obj->transformation().inverted() );
                }
            }
            else
                gc->fill.setStyle( Qt::NoBrush );
        }
        else
        {
            parseColor( fillcolor,  params );
            gc->fill.setStyle( Qt::SolidPattern );
        }
    }
    else if( command == "fill-rule" )
    {
        if( params == "nonzero" )
            gc->fillRule = Qt::WindingFill;
        else if( params == "evenodd" )
            gc->fillRule = Qt::OddEvenFill;
    }
    else if( command == "stroke" )
    {
        if( params == "none" )
            gc->stroke.setLineStyle( Qt::NoPen, QVector<qreal>() );
        /* TODO add gradient support to borders
        else if( params.startsWith( "url(" ) )
        {
            unsigned int start = params.find("#") + 1;
            unsigned int end = params.findRev(")");
            QString key = params.mid( start, end - start );

            GradientHelper *gradHelper = findGradient( key );
            if( gradHelper )
            {
                gc->stroke.gradient() = gradHelper->gradient;
                gc->stroke.gradient().transform( gradHelper->gradientTransform );
                gc->stroke.gradient().transform( gc->matrix );
                gc->stroke.setType( VStroke::grad );
            }
            else 
                gc->stroke.setType( VStroke::none );
        }
        */
        else
        {
            parseColor( strokecolor, params );
            gc->stroke.setLineStyle( Qt::SolidLine, QVector<qreal>() );
        }
    }
    else if( command == "stroke-width" )
        gc->stroke.setLineWidth( parseUnit( params, true, true, m_outerRect ) );
    else if( command == "stroke-linejoin" )
    {
        if( params == "miter" )
            gc->stroke.setJoinStyle( Qt::MiterJoin );
        else if( params == "round" )
            gc->stroke.setJoinStyle( Qt::RoundJoin );
        else if( params == "bevel" )
            gc->stroke.setJoinStyle( Qt::BevelJoin );
    }
    else if( command == "stroke-linecap" )
    {
        if( params == "butt" )
            gc->stroke.setCapStyle( Qt::FlatCap );
        else if( params == "round" )
            gc->stroke.setCapStyle( Qt::RoundCap );
        else if( params == "square" )
            gc->stroke.setCapStyle( Qt::SquareCap );
    }
    else if( command == "stroke-miterlimit" )
        gc->stroke.setMiterLimit( params.toFloat() );
    else if( command == "stroke-dasharray" )
    {
        QVector<qreal> array;
        if(params != "none")
        {
            QStringList dashes = params.split( ' ' );
            for( QStringList::Iterator it = dashes.begin(); it != dashes.end(); ++it )
                array.append( (*it).toFloat() );
        }
        gc->stroke.setLineStyle( Qt::CustomDashLine, array );
    }
    /* TODO add support for dash offset to border
    else if( command == "stroke-dashoffset" )
        gc->stroke.dashPattern().setOffset( params.toFloat() );
    */
    // handle opacity
    else if( command == "stroke-opacity" )
        strokecolor.setAlphaF( fromPercentage( params ) );
    else if( command == "fill-opacity" )
    {
        float opacity = fromPercentage( params );
        if( opacity < 0.0 )
            opacity = 0.0;
        if( opacity > 1.0 )
            opacity = 1.0;
        fillcolor.setAlphaF( opacity );
    }
    else if( command == "opacity" )
    {
        fillcolor.setAlphaF( fromPercentage( params ) );
        strokecolor.setAlphaF( fromPercentage( params ) );
    }
    else if( command == "font-family" )
    {
        QString family = params;
        family.replace( '\'' , ' ' );
        gc->font.setFamily( family );
    }
    else if( command == "font-size" )
    {
        float pointSize = parseUnit( params );
        gc->font.setPointSizeF( pointSize * getScalingFromMatrix( gc->matrix ) );
    }
    else if( command == "font-weight" )
    {
        int weight = QFont::Normal;

        // map svg weight to qt weight
        // svg value		qt value
        // 100,200,300		1, 17, 33
        // 400				50			(normal)
        // 500,600			58,66
        // 700				75			(bold)
        // 800,900			87,99

        if( params == "bold" )
            weight = QFont::Bold;
        else if( params == "lighter" )
        {
            weight = gc->font.weight();
            if( weight <= 17 ) 
                weight = 1;
            else if( weight <= 33 )
                weight = 17;
            else if( weight <= 50 )
                weight = 33;
            else if( weight <= 58 )
                weight = 50;
            else if( weight <= 66 )
                weight = 58;
            else if( weight <= 75 )
                weight = 66;
            else if( weight <= 87 )
                weight = 75;
            else if( weight <= 99 )
                weight = 87;
        }
        else if( params == "bolder" )
        {
            weight = gc->font.weight();
            if( weight >= 87 ) 
                weight = 99;
            else if( weight >= 75 )
                weight = 87;
            else if( weight >= 66 )
                weight = 75;
            else if( weight >= 58 )
                weight = 66;
            else if( weight >= 50 )
                weight = 58;
            else if( weight >= 33 )
                weight = 50;
            else if( weight >= 17 )
                weight = 50;
            else if( weight >= 1 )
                weight = 17;
        }
        else
        {
            bool ok;
            // try to read numerical weight value
            weight = params.toInt( &ok, 10 );

            if( !ok )
                return;

            switch( weight )
            {
                case 100: weight = 1; break;
                case 200: weight = 17; break;
                case 300: weight = 33; break;
                case 400: weight = 50; break;
                case 500: weight = 58; break;
                case 600: weight = 66; break;
                case 700: weight = 75; break;
                case 800: weight = 87; break;
                case 900: weight = 99; break;
            }
        }
        gc->font.setWeight( weight );
    }
    else if( command == "text-decoration" )
    {
        if( params == "line-through" )
            gc->font.setStrikeOut( true );
        else if( params == "underline" )
            gc->font.setUnderline( true );
    }
    else if( command == "color" )
    {
        QColor color;
        parseColor( color, params );
        gc->color = color;
    }
    if( gc->fill.style() != Qt::NoBrush )
        gc->fill.setColor( fillcolor );
    //if( gc->stroke.type() == VStroke::solid )
        gc->stroke.setColor( strokecolor );
}

void SvgImport::parseStyle( KoShape *obj, const QDomElement &e )
{
    SvgGraphicsContext *gc = m_gc.top();
    if( !gc ) return;

    // try normal PA
    if( !e.attribute( "color" ).isEmpty() )
        parsePA( obj, gc, "color", e.attribute( "color" ) );
    if( !e.attribute( "fill" ).isEmpty() )
        parsePA( obj, gc, "fill", e.attribute( "fill" ) );
    if( !e.attribute( "fill-rule" ).isEmpty() )
        parsePA( obj, gc, "fill-rule", e.attribute( "fill-rule" ) );
    if( !e.attribute( "stroke" ).isEmpty() )
        parsePA( obj, gc, "stroke", e.attribute( "stroke" ) );
    if( !e.attribute( "stroke-width" ).isEmpty() )
        parsePA( obj, gc, "stroke-width", e.attribute( "stroke-width" ) );
    if( !e.attribute( "stroke-linejoin" ).isEmpty() )
        parsePA( obj, gc, "stroke-linejoin", e.attribute( "stroke-linejoin" ) );
    if( !e.attribute( "stroke-linecap" ).isEmpty() )
        parsePA( obj, gc, "stroke-linecap", e.attribute( "stroke-linecap" ) );
    if( !e.attribute( "stroke-dasharray" ).isEmpty() )
        parsePA( obj, gc, "stroke-dasharray", e.attribute( "stroke-dasharray" ) );
    if( !e.attribute( "stroke-dashoffset" ).isEmpty() )
        parsePA( obj, gc, "stroke-dashoffset", e.attribute( "stroke-dashoffset" ) );
    if( !e.attribute( "stroke-opacity" ).isEmpty() )
        parsePA( obj, gc, "stroke-opacity", e.attribute( "stroke-opacity" ) );
    if( !e.attribute( "stroke-miterlimit" ).isEmpty() )
        parsePA( obj, gc, "stroke-miterlimit", e.attribute( "stroke-miterlimit" ) );
    if( !e.attribute( "fill-opacity" ).isEmpty() )
        parsePA( obj, gc, "fill-opacity", e.attribute( "fill-opacity" ) );
    if( !e.attribute( "opacity" ).isEmpty() )
        parsePA( obj, gc, "opacity", e.attribute( "opacity" ) );

    // try style attr
    QString style = e.attribute( "style" ).simplified();
    QStringList substyles = style.split( ';', QString::SkipEmptyParts );
    for( QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it )
    {
        QStringList substyle = it->split( ':' );
        QString command = substyle[0].trimmed();
        QString params  = substyle[1].trimmed();
        parsePA( obj, gc, command, params );
    }

    if(!obj)
        return;

    obj->setBackground( gc->fill );
    KoPathShape * path = dynamic_cast<KoPathShape*>( obj );
    if( path )
        path->setFillRule( gc->fillRule );
    // stroke scaling
    double lineWidth = gc->stroke.lineWidth() * getScalingFromMatrix( gc->matrix );
    KoLineBorder * border = new KoLineBorder( gc->stroke );
    border->setLineWidth( lineWidth );
    obj->setBorder( border );
}

void SvgImport::parseFont( const QDomElement &e )
{
    SvgGraphicsContext *gc = m_gc.top();
    if( !gc ) return;

    if( ! e.attribute( "font-family" ).isEmpty() )	
        parsePA( 0L, m_gc.top(), "font-family", e.attribute( "font-family" ) );
    if( ! e.attribute( "font-size" ).isEmpty() )	
        parsePA( 0L, m_gc.top(), "font-size", e.attribute( "font-size" ) );
    if( ! e.attribute( "font-weight" ).isEmpty() )	
        parsePA( 0L, m_gc.top(), "font-weight", e.attribute( "font-weight" ) );
    if( ! e.attribute( "text-decoration" ).isEmpty() )
        parsePA( 0L, m_gc.top(), "text-decoration", e.attribute( "text-decoration" ) );
}

QList<KoShape*> SvgImport::parseUse( const QDomElement &e )
{
    QList<KoShape*> shapes;

    QString id = e.attribute( "xlink:href" );
    // 
    if( !id.isEmpty() )
    {
        addGraphicContext();
        setupTransform( e );

        QString key = id.mid( 1 );

        if( !e.attribute( "x" ).isEmpty() && !e.attribute( "y" ).isEmpty() )
        {
            double tx = e.attribute( "x" ).toDouble();
            double ty = e.attribute( "y" ).toDouble();

            m_gc.top()->matrix.translate(tx,ty);
        }

        if(m_defs.contains(key))
        {
            QDomElement a = m_defs[key];
            if(a.tagName() == "g" || a.tagName() == "a")
            {
                QList<KoShape*> childShapes = parseGroup( a);
                shapes += childShapes;
            }
            else
            {
                // Create the object with the merged styles.
                // The object inherits all style attributes from the use tag, but keeps it's own attributes.
                // So, not just use the style attributes of the use tag, but merge them first.
                KoShape * shape = createObject( a, mergeStyles(e, a) );
                if( shape )
                    shapes.append( shape );
            }
        }
        removeGraphicContext();
    }

    return shapes;
}

void SvgImport::addToGroup( QList<KoShape*> shapes, KoShapeGroup * group )
{
    foreach( KoShape * shape, shapes )
        m_document.add( shape );

    if( ! group )
        return;

    KoShapeGroupCommand cmd( group, shapes );
    cmd.redo();
}

QList<KoShape*> SvgImport::parseGroup( const QDomElement &e )
{
    QList<KoShape*> shapes;

    for( QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement b = n.toElement();
        if( b.isNull() ) continue;

        // treat svg link <a> as group so we don't miss its child elements
        if( b.tagName() == "g" || b.tagName() == "a" )
        {
            addGraphicContext();
            setupTransform( b );

            KoShapeGroup * group = new KoShapeGroup();
            group->setZIndex( nextZIndex() );

            parseStyle( 0, b );
            parseFont( b );

            QList<KoShape*> childShapes = parseGroup( b );

            // handle id
            if( !b.attribute("id").isEmpty() )
                group->setName( b.attribute("id") );

            addToGroup( childShapes, group );

            shapes.append( group );

            removeGraphicContext();

            continue;
        }
        if( b.tagName() == "defs" )
        {
            parseDefs( b );
            continue;
        }
        else if( b.tagName() == "linearGradient" || b.tagName() == "radialGradient" )
        {
            parseGradient( b );
            continue;
        }
        if( b.tagName() == "rect" ||
            b.tagName() == "ellipse" ||
            b.tagName() == "circle" ||
            b.tagName() == "line" ||
            b.tagName() == "polyline" ||
            b.tagName() == "polygon" ||
            b.tagName() == "path" ||
            b.tagName() == "image" )
        {
            KoShape * shape = createObject( b );
            if( shape )
                shapes.append( shape );
            continue;
        }
        else if( b.tagName() == "text" )
        {
            KoShape * shape = createText( b );
            if( shape )
                shapes.append( shape );
            continue;
        }
        else if( b.tagName() == "use" )
        {
            shapes += parseUse( b );
            continue;
        }
    }

    return shapes;
}

void SvgImport::parseDefs( const QDomElement &e )
{
    for( QDomNode n = e.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        QDomElement b = n.toElement();
        if( b.isNull() ) continue;

        QString definition = b.attribute( "id" );
        if( !definition.isEmpty() )
        {
            if( !m_defs.contains( definition ) )
                m_defs.insert( definition, b );
        }
    }
}


// Creating functions
// ---------------------------------------------------------------------------------------

KoShape * SvgImport::createText( const QDomElement &b )
{
    QString content;
    QString anchor;
    double offset = 0.0;

    QPointF textPosition;
    TextShape * text = 0;

    addGraphicContext();
    setupTransform( b );

    parseFont( b );

    if( ! b.attribute( "text-anchor" ).isEmpty() )
        anchor = b.attribute( "text-anchor" );

    kDebug() << "creating text object";

    if( b.hasChildNodes() )
    {
        if( textPosition.isNull() && ! b.attribute( "x" ).isEmpty() && ! b.attribute( "y" ).isEmpty() )
        {
            textPosition.setX( parseUnit( b.attribute( "x" ) ) );
            textPosition.setY( parseUnit( b.attribute( "y" ) ) );
        }

        text = new TextShape();
        QTextDocument * textDoc = text->textShapeData()->document();

        for( QDomNode n = b.firstChild(); !n.isNull(); n = n.nextSibling() )
        {
            QDomElement e = n.toElement();
            if( e.isNull() )
            {
                content += n.toCharacterData().data();
            }
            /*

            // TODO implement text on path support
            else if( e.tagName() == "textPath" )
            {
                if( e.attribute( "xlink:href" ).isEmpty() )
                    continue;

                QString key = e.attribute( "xlink:href" ).mid( 1 );
                if( ! m_defs.contains(key) )
                {
                    // try to find referenced object in document
                    KoShape * obj = findObject( key );
                    // try to find referenced object in actual group, which is not yet part of document
                    if( ! obj )
                        obj = findObject( key, grp );
                    if( obj ) 
                        path = dynamic_cast<VPath*>( obj );
                }
                else
                {
                    QDomElement p = m_defs[key];
                    createObject( grp, p, VObject::deleted);
                }
                if( ! path )
                    continue;
                base = *path->paths().getFirst();
                content += e.text();

                if( ! e.attribute( "startOffset" ).isEmpty() )
                {
                    QString start = e.attribute( "startOffset" );
                    if( start.endsWith( '%' ) )
                        offset = 0.01 * start.remove( '%' ).toDouble();
                    else
                    {
                        float pathLength = 0;
                        VSubpathIterator pIt( base );

                        for( ; pIt.current(); ++pIt )
                            pathLength += pIt.current()->length();

                        if( pathLength > 0.0 )
                            offset = start.toDouble() / pathLength;
                    }
                }
            }
            else if( e.tagName() == "tspan" )
            {
                // only use text of tspan element, as we are not supporting text 
                // with different styles
                content += e.text();
                if( textPosition.isNull() && ! e.attribute( "x" ).isEmpty() && ! e.attribute( "y" ).isEmpty() )
                {
                    QStringList posX = e.attribute( "x" ).split( ", " );
                    QStringList posY = e.attribute( "y" ).split( ", " );
                    if( posX.count() && posY.count() )
                    {
                        textPosition.setX( parseUnit( posX.first() ) );
                        textPosition.setY( parseUnit( posY.first() ) );
                    }
                }
            }
            else if( e.tagName() == "tref" )
            {
                if( e.attribute( "xlink:href" ).isEmpty() )
                    continue;

                QString key = e.attribute( "xlink:href" ).mid( 1 );
                if( ! m_defs.contains(key) )
                {
                    // try to find referenced object in document
                    KoShape * obj = findObject( key );
                    // try to find referenced object in actual group, which is not yet part of document
                    if( ! obj )
                        obj = findObject( key, grp );
                    if( obj ) 
                        content += dynamic_cast<VText*>( obj )->text();
                }
                else
                {
                    QDomElement p = m_defs[key];
                    content += p.text();
                }
            }
            else
                continue;

            if( ! e.attribute( "text-anchor" ).isEmpty() )
                anchor = e.attribute( "text-anchor" );
            */
        }
        textDoc->setPlainText( content.simplified() );
        textDoc->setDefaultFont( m_gc.top()->font );
        text->setPosition( textPosition );
    }
    else
    {
        kDebug() << "single text line";
        // a single text line
        textPosition.setX( parseUnit( b.attribute( "x" ) ) );
        textPosition.setY( parseUnit( b.attribute( "y" ) ) );
        text = new TextShape();
        QTextDocument * textDoc = text->textShapeData()->document();
        textDoc->setDefaultFont( m_gc.top()->font );
        textDoc->setPlainText( b.text().simplified() );
        text->setPosition( textPosition );
    }

    if( ! text )
    {
        removeGraphicContext();
        return 0;
    }

    parseStyle( text, b );

    //text->setFont( m_gc.top()->font );

    text->applyAbsoluteTransformation( m_gc.top()->matrix );

    if( !b.attribute("id").isEmpty() )
        text->setName( b.attribute("id") );

    /*
    if( anchor == "middle" )
        text->setAlignment( VText::Center );
    else if( anchor == "end" )
        text->setAlignment( VText::Right );
    */

    //if( offset > 0.0 )
    //    text->setOffset( offset );

    removeGraphicContext();

    return text;
}

KoShape * SvgImport::createObject( const QDomElement &b, const QDomElement &style )
{
    KoShape *obj = 0L;

    addGraphicContext();
    setupTransform( b );

    if( b.tagName() == "rect" )
    {
        double x = parseUnit( b.attribute( "x" ), true, false, m_outerRect );
        double y = parseUnit( b.attribute( "y" ), false, true, m_outerRect );
        double w = parseUnit( b.attribute( "width" ), true, false, m_outerRect );
        double h = parseUnit( b.attribute( "height" ), false, true, m_outerRect );
        double rx = b.attribute( "rx" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "rx" ) );
        double ry = b.attribute( "ry" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "ry" ) );
        KoRectangleShape * rect = new KoRectangleShape();
        rect->setSize( QSizeF(w,h) );
        rect->setPosition( QPointF(x,y) );
        if( rx >= 0.0 )
            rect->setCornerRadiusX( qMin( 100.0, rx / (0.5 * w) * 100.0 ) );
        rect->setPosition( QPointF(x,y) );
        if( ry >= 0.0 )
            rect->setCornerRadiusY( qMin( 100.0, ry / (0.5 * h) * 100.0 ) );
        obj = rect;
    }
    else if( b.tagName() == "ellipse" )
    {
        double rx   = parseUnit( b.attribute( "rx" ) );
        double ry   = parseUnit( b.attribute( "ry" ) );
        double left = parseUnit( b.attribute( "cx" ) ) - rx;
        double top  = parseUnit( b.attribute( "cy" ) ) - ry;
        obj = new KoEllipseShape();
        obj->setSize( QSizeF(2*rx, 2*ry) );
        obj->setPosition( QPointF(left,top) );
    }
    else if( b.tagName() == "circle" )
    {
        double r    = parseUnit( b.attribute( "r" ) );
        double left = parseUnit( b.attribute( "cx" ) ) - r;
        double top  = parseUnit( b.attribute( "cy" ) ) - r;
        obj = new KoEllipseShape();
        obj->setSize( QSizeF(2*r, 2*r) );
        obj->setPosition( QPointF(left,top) );
    }
    else if( b.tagName() == "line" )
    {
        KoPathShape * path = new KoPathShape();
        double x1 = b.attribute( "x1" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "x1" ) );
        double y1 = b.attribute( "y1" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "y1" ) );
        double x2 = b.attribute( "x2" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "x2" ) );
        double y2 = b.attribute( "y2" ).isEmpty() ? 0.0 : parseUnit( b.attribute( "y2" ) );
        path->moveTo( QPointF( x1, y1 ) );
        path->lineTo( QPointF( x2, y2 ) );
        obj = path;
    }
    else if( b.tagName() == "polyline" || b.tagName() == "polygon" )
    {
        KoPathShape * path = new KoPathShape();
        bool bFirst = true;

        QString points = b.attribute( "points" ).simplified();
        points.replace( ',', ' ' );
        points.remove( '\r' );
        points.remove( '\n' );
        QStringList pointList = points.split( ' ' );
        for( QStringList::Iterator it = pointList.begin(); it != pointList.end(); ++it)
        {
            QPointF point;
            point.setX( (*it).toDouble() );
            ++it;
            point.setY( (*it).toDouble() );
            if( bFirst )
            {
                path->moveTo( point );
                bFirst = false;
            }
            else
                path->lineTo( point );
        }
        if( b.tagName() == "polygon" ) 
            path->close();

        path->setPosition( path->normalize() );

        obj = path;
    }
    else if( b.tagName() == "path" )
    {
        KoPathShape *path = new KoPathShape();
        KoPathShapeLoader loader( path );
        loader.parseSvg( b.attribute( "d" ), true );
        path->setPosition( path->normalize() );
        obj = path;
    }
    else if( b.tagName() == "image" )
    {
        QString fname = b.attribute("xlink:href");
        QImage img;
        if( fname.startsWith( "data:" ) )
        {
            int start = fname.indexOf( "base64," );
            if( start < 0 )
                return 0;
            if( ! img.loadFromData( QByteArray::fromBase64( fname.mid( start + 7 ).toLatin1() ) ) )
                return 0;
        }
        else if( ! img.load( absoluteFilePath( fname ) ) )
            return 0;

        KoImageData * data = new KoImageData( m_document.imageCollection() );
        data->setImage( img );

        double x = parseUnit( b.attribute( "x" ) );
        double y = parseUnit( b.attribute( "y" ) );
        double w = parseUnit( b.attribute( "width" ) );
        double h = parseUnit( b.attribute( "height" ) );

        PictureShape * picture = new PictureShape();
        picture->setUserData( data );
        picture->setSize( QSizeF(w,h) );
        picture->setPosition( QPointF(x,y) );

        obj = picture;
    }

    if( !obj )
    {
        removeGraphicContext();
        return 0;
    }

    obj->applyAbsoluteTransformation( m_gc.top()->matrix );

    if( !style.isNull() )
        parseStyle( obj, style );
    else
        parseStyle( obj, b );

    // handle id
    if( !b.attribute("id").isEmpty() )
        obj->setName( b.attribute("id") );

    removeGraphicContext();

    obj->setZIndex( nextZIndex() );

    return obj;
}

int SvgImport::nextZIndex()
{
    static int zIndex = 0;

    return zIndex++;
}

QString SvgImport::absoluteFilePath( const QString &href )
{
    QFileInfo info( href );
    if( ! info.isRelative() )
        return href;

    QFileInfo pathInfo( QFileInfo( m_chain->inputFile() ).filePath() );

    QString relFile = href;
    while( relFile.startsWith( "../" ) )
    {
        relFile = relFile.mid( 3 );
        pathInfo.setFile( pathInfo.dir(), QString() );
    }

    QString absFile = pathInfo.absolutePath() + '/' + relFile;

    return absFile;
}

#include <svgimport.moc>
