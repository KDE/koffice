/******************************************************************/
/* KPresenter - (c) by Reginald Stadlbauer 1997-1998              */
/* Version: 0.1.0                                                 */
/* Author: Reginald Stadlbauer                                    */
/* E-Mail: reggie@kde.org                                         */
/* Homepage: http://boch35.kfunigraz.ac.at/~rs                    */
/* needs c++ library Qt (http://www.troll.no)                     */
/* written for KDE (http://www.kde.org)                           */
/* needs mico (http://diamant.vsb.cs.uni-frankfurt.de/~mico/)     */
/* needs OpenParts and Kom (weis@kde.org)                         */
/* License: GNU GPL                                               */
/******************************************************************/
/* Module: clipart object                                        */
/******************************************************************/

#include "kpclipartobject.h"
#include "kpgradient.h"

#include <qpainter.h>
#include <qwmatrix.h>
#include <qfileinfo.h>

/******************************************************************/
/* Class: KPClipartObject                                        */
/******************************************************************/

/*================ default constructor ===========================*/
KPClipartObject::KPClipartObject( KPClipartCollection *_clipartCollection )
    : KPObject()
{
    brush = Qt::NoBrush;
    gradient = 0;
    fillType = FT_BRUSH;
    gType = BCT_GHORZ;
    redrawPix = false;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    gColor1 = Qt::red;
    gColor2 = Qt::green;
    clipartCollection = _clipartCollection;
    picture = 0L;
}

/*================== overloaded constructor ======================*/
KPClipartObject::KPClipartObject( KPClipartCollection *_clipartCollection, const QString &_filename, QDateTime _lastModified )
    : KPObject()
{
    brush = Qt::NoBrush;
    gradient = 0;
    fillType = FT_BRUSH;
    gType = BCT_GHORZ;
    redrawPix = false;
    pen = QPen( Qt::black, 1, Qt::NoPen );
    gColor1 = Qt::red;
    gColor2 = Qt::green;
    clipartCollection = _clipartCollection;

    if ( !_lastModified.isValid() )
    {
        QFileInfo inf( _filename );
        _lastModified = inf.lastModified();
    }

    picture = 0L;
    setFileName( _filename, _lastModified );
}

/*================================================================*/
void KPClipartObject::setFileName( const QString &_filename, QDateTime _lastModified )
{
    if ( !_lastModified.isValid() )
    {
        QFileInfo inf( _filename );
        _lastModified = inf.lastModified();
    }

    if ( picture )
        clipartCollection->removeRef( key );

    key = KPClipartCollection::Key( _filename, _lastModified );
    picture = clipartCollection->findClipart( key );
}

/*================================================================*/
void KPClipartObject::setFillType( FillType _fillType )
{
    fillType = _fillType;

    if ( fillType == FT_BRUSH && gradient )
    {
        delete gradient;
        gradient = 0;
    }
    if ( fillType == FT_GRADIENT && !gradient )
    {
        gradient = new KPGradient( gColor1, gColor2, gType, getSize() );
        redrawPix = true;
        pix.resize( getSize() );
    }
}

/*================================================================*/
void KPClipartObject::setSize( int _width, int _height )
{
    KPObject::setSize( _width, _height );
    if ( move ) return;

    if ( fillType == FT_GRADIENT && gradient )
    {
        gradient->setSize( getSize() );
        redrawPix = true;
        pix.resize( getSize() );
    }
}

/*================================================================*/
void KPClipartObject::resizeBy( int _dx, int _dy )
{
    KPObject::resizeBy( _dx, _dy );
    if ( move ) return;

    if ( fillType == FT_GRADIENT && gradient )
    {
        gradient->setSize( getSize() );
        redrawPix = true;
        pix.resize( getSize() );
    }
}

/*========================= save =================================*/
void KPClipartObject::save( ostream& out )
{
    out << indent << "<ORIG x=\"" << orig.x() << "\" y=\"" << orig.y() << "\"/>" << endl;
    out << indent << "<SIZE width=\"" << ext.width() << "\" height=\"" << ext.height() << "\"/>" << endl;
    out << indent << "<SHADOW distance=\"" << shadowDistance << "\" direction=\""
        << static_cast<int>( shadowDirection ) << "\" red=\"" << shadowColor.red() << "\" green=\"" << shadowColor.green()
        << "\" blue=\"" << shadowColor.blue() << "\"/>" << endl;
    out << indent << "<EFFECTS effect=\"" << static_cast<int>( effect ) << "\" effect2=\""
        << static_cast<int>( effect2 ) << "\"/>" << endl;
    out << indent << "<PRESNUM value=\"" << presNum << "\"/>" << endl;
    out << indent << "<ANGLE value=\"" << angle << "\"/>" << endl;
    out << indent << "<KEY " << key << " />" << endl;
    out << indent << "<FILLTYPE value=\"" << static_cast<int>( fillType ) << "\"/>" << endl;
    out << indent << "<GRADIENT red1=\"" << gColor1.red() << "\" green1=\"" << gColor1.green()
        << "\" blue1=\"" << gColor1.blue() << "\" red2=\"" << gColor2.red() << "\" green2=\""
        << gColor2.green() << "\" blue2=\"" << gColor2.blue() << "\" type=\""
        << static_cast<int>( gType ) << "\"/>" << endl;
    out << indent << "<PEN red=\"" << pen.color().red() << "\" green=\"" << pen.color().green()
        << "\" blue=\"" << pen.color().blue() << "\" width=\"" << pen.width()
        << "\" style=\"" << static_cast<int>( pen.style() ) << "\"/>" << endl;
    out << indent << "<BRUSH red=\"" << brush.color().red() << "\" green=\"" << brush.color().green()
        << "\" blue=\"" << brush.color().blue() << "\" style=\"" << static_cast<int>( brush.style() ) << "\"/>" << endl;
}

/*========================== load ================================*/
void KPClipartObject::load( KOMLParser& parser, vector<KOMLAttrib>& lst )
{
    string tag;
    string name;

    while ( parser.open( 0L, tag ) )
    {
        KOMLParser::parseTag( tag.c_str(), name, lst );

        // orig
        if ( name == "ORIG" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "x" )
                    orig.setX( atoi( ( *it ).m_strValue.c_str() ) );
                if ( ( *it ).m_strName == "y" )
                    orig.setY( atoi( ( *it ).m_strValue.c_str() ) );
            }
        }

        // size
        else if ( name == "SIZE" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "width" )
                    ext.setWidth( atoi( ( *it ).m_strValue.c_str() ) );
                if ( ( *it ).m_strName == "height" )
                    ext.setHeight( atoi( ( *it ).m_strValue.c_str() ) );
            }
        }

        // shadow
        else if ( name == "SHADOW" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "distance" )
                    shadowDistance = atoi( ( *it ).m_strValue.c_str() );
                if ( ( *it ).m_strName == "direction" )
                    shadowDirection = ( ShadowDirection )atoi( ( *it ).m_strValue.c_str() );
                if ( ( *it ).m_strName == "red" )
                    shadowColor.setRgb( atoi( ( *it ).m_strValue.c_str() ),
                                        shadowColor.green(), shadowColor.blue() );
                if ( ( *it ).m_strName == "green" )
                    shadowColor.setRgb( shadowColor.red(), atoi( ( *it ).m_strValue.c_str() ),
                                        shadowColor.blue() );
                if ( ( *it ).m_strName == "blue" )
                    shadowColor.setRgb( shadowColor.red(), shadowColor.green(),
                                        atoi( ( *it ).m_strValue.c_str() ) );
            }
        }

        // effects
        else if ( name == "EFFECTS" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "effect" )
                    effect = ( Effect )atoi( ( *it ).m_strValue.c_str() );
                if ( ( *it ).m_strName == "effect2" )
                    effect2 = ( Effect2 )atoi( ( *it ).m_strValue.c_str() );
            }
        }

        // angle
        else if ( name == "ANGLE" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "value" )
                    angle = atof( ( *it ).m_strValue.c_str() );
            }
        }

        // presNum
        else if ( name == "PRESNUM" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "value" )
                    presNum = atoi( ( *it ).m_strValue.c_str() );
            }
        }

        // filename
        else if ( name == "FILENAME" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "filename" )
                    key.filename = ( *it ).m_strValue.c_str();

            }

            key.lastModified.setDate( clipartCollection->tmpDate() );
            key.lastModified.setTime( clipartCollection->tmpTime() );
        }

        // key
        else if ( name == "KEY" )
        {
            int year, month, day, hour, minute, second, msec;

            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "filename" )
                    key.filename = ( *it ).m_strValue.c_str();
                else if ( ( *it ).m_strName == "year" )
                    year = atoi( ( *it ).m_strValue.c_str() );
                else if ( ( *it ).m_strName == "month" )
                    month = atoi( ( *it ).m_strValue.c_str() );
                else if ( ( *it ).m_strName == "day" )
                    day = atoi( ( *it ).m_strValue.c_str() );
                else if ( ( *it ).m_strName == "hour" )
                    hour = atoi( ( *it ).m_strValue.c_str() );
                else if ( ( *it ).m_strName == "minute" )
                    minute = atoi( ( *it ).m_strValue.c_str() );
                else if ( ( *it ).m_strName == "second" )
                    second = atoi( ( *it ).m_strValue.c_str() );
                else if ( ( *it ).m_strName == "msec" )
                    msec = atoi( ( *it ).m_strValue.c_str() );
            }
            key.lastModified.setDate( QDate( year, month, day ) );
            key.lastModified.setTime( QTime( hour, minute, second, msec ) );
        }

        // pen
        else if ( name == "PEN" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "red" )
                    pen.setColor( QColor( atoi( ( *it ).m_strValue.c_str() ), pen.color().green(), pen.color().blue() ) );
                if ( ( *it ).m_strName == "green" )
                    pen.setColor( QColor( pen.color().red(), atoi( ( *it ).m_strValue.c_str() ), pen.color().blue() ) );
                if ( ( *it ).m_strName == "blue" )
                    pen.setColor( QColor( pen.color().red(), pen.color().green(), atoi( ( *it ).m_strValue.c_str() ) ) );
                if ( ( *it ).m_strName == "width" )
                    pen.setWidth( atoi( ( *it ).m_strValue.c_str() ) );
                if ( ( *it ).m_strName == "style" )
                    pen.setStyle( ( Qt::PenStyle )atoi( ( *it ).m_strValue.c_str() ) );
            }
            setPen( pen );
        }

        // brush
        else if ( name == "BRUSH" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "red" )
                    brush.setColor( QColor( atoi( ( *it ).m_strValue.c_str() ), brush.color().green(), brush.color().blue() ) );
                if ( ( *it ).m_strName == "green" )
                    brush.setColor( QColor( brush.color().red(), atoi( ( *it ).m_strValue.c_str() ), brush.color().blue() ) );
                if ( ( *it ).m_strName == "blue" )
                    brush.setColor( QColor( brush.color().red(), brush.color().green(), atoi( ( *it ).m_strValue.c_str() ) ) );
                if ( ( *it ).m_strName == "style" )
                    brush.setStyle( ( Qt::BrushStyle )atoi( ( *it ).m_strValue.c_str() ) );
            }
            setBrush( brush );
        }

        // fillType
        else if ( name == "FILLTYPE" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "value" )
                    fillType = static_cast<FillType>( atoi( ( *it ).m_strValue.c_str() ) );
            }
            setFillType( fillType );
        }

        // gradient
        else if ( name == "GRADIENT" )
        {
            KOMLParser::parseTag( tag.c_str(), name, lst );
            vector<KOMLAttrib>::const_iterator it = lst.begin();
            for( ; it != lst.end(); it++ )
            {
                if ( ( *it ).m_strName == "red1" )
                    gColor1 = QColor( atoi( ( *it ).m_strValue.c_str() ), gColor1.green(), gColor1.blue() );
                if ( ( *it ).m_strName == "green1" )
                    gColor1 = QColor( gColor1.red(), atoi( ( *it ).m_strValue.c_str() ), gColor1.blue() );
                if ( ( *it ).m_strName == "blue1" )
                    gColor1 = QColor( gColor1.red(), gColor1.green(), atoi( ( *it ).m_strValue.c_str() ) );
                if ( ( *it ).m_strName == "red2" )
                    gColor2 = QColor( atoi( ( *it ).m_strValue.c_str() ), gColor2.green(), gColor2.blue() );
                if ( ( *it ).m_strName == "green2" )
                    gColor2 = QColor( gColor2.red(), atoi( ( *it ).m_strValue.c_str() ), gColor2.blue() );
                if ( ( *it ).m_strName == "blue2" )
                    gColor2 = QColor( gColor2.red(), gColor2.green(), atoi( ( *it ).m_strValue.c_str() ) );
                if ( ( *it ).m_strName == "type" )
                    gType = static_cast<BCType>( atoi( ( *it ).m_strValue.c_str() ) );
            }
            setGColor1( gColor1 );
            setGColor2( gColor2 );
            setGType( gType );
        }

        else
            cerr << "Unknown tag '" << tag << "' in CLIPART_OBJECT" << endl;

        if ( !parser.close( tag ) )
        {
            cerr << "ERR: Closing Child" << endl;
            return;
        }
    }
}

/*========================= draw =================================*/
void KPClipartObject::draw( QPainter *_painter, int _diffx, int _diffy )
{
    if ( move )
    {
        KPObject::draw( _painter, _diffx, _diffy );
        return;
    }

    if ( !picture )
        return;

    int ox = orig.x() - _diffx;
    int oy = orig.y() - _diffy;
    int ow = ext.width();
    int oh = ext.height();
    KRect r;

    _painter->save();
    r = _painter->viewport();

    _painter->setPen( pen );
    _painter->setBrush( brush );

    int pw = pen.width();

    _painter->save();
    _painter->setViewport( ox, oy, r.width(), r.height() );
    if ( fillType == FT_BRUSH || !gradient )
        _painter->drawRect( pw, pw, ext.width() - 2 * pw, ext.height() - 2 * pw );
    else
    {
        if ( angle == 0 )
            _painter->drawPixmap( pw, pw, *gradient->getGradient(), 0, 0, ow - 2 * pw, oh - 2 * pw );
        else
        {
            QPixmap pix( ow - 2 * pw, oh - 2 * pw );
            QPainter p;
            p.begin( &pix );
            p.drawPixmap( 0, 0, *gradient->getGradient() );
            p.end();

            _painter->drawPixmap( pw, pw, pix );
        }

        _painter->setPen( pen );
        _painter->setBrush( Qt::NoBrush );
        _painter->drawRect( pw, pw, ow - 2 * pw, oh - 2 * pw );
    }
    _painter->setViewport( r );
    _painter->restore();

    // ********* TODO: shadow for cliparts

    _painter->save();
    r = _painter->viewport();
    _painter->setViewport( ox + 1, oy + 1, ext.width() - 2, ext.height() - 2 );

    if ( angle == 0 )
        _painter->drawPicture( *picture );
    else
    {
        KRect br = KRect( 0, 0, ow, oh );
        int pw = br.width();
        int ph = br.height();
        int yPos = -br.y();
        int xPos = -br.x();
        br.moveTopLeft( KPoint( -br.width() / 2, -br.height() / 2 ) );

        QWMatrix m, mtx;
        mtx.rotate( angle );
        m.translate( pw / 2, ph / 2 );
        m = mtx * m;

        QPixmap pm( pw, ph );
        pm.fill( Qt::white );
        QPainter pnt;
        pnt.begin( &pm );
        pnt.drawPicture( *picture );
        pnt.end();

        _painter->setViewport( ox, oy, r.width(), r.height() );
        _painter->setWorldMatrix( m );

        _painter->drawPixmap( br.left() + xPos, br.top() + yPos, pm );
    }

    _painter->setViewport( r );
    _painter->restore();

    KPObject::draw( _painter, _diffx, _diffy );
}




