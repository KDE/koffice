/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <KPresenterObjectIface.h>

#include <kpobject.h>
#include <kptextobject.h>

#include <qpainter.h>
#include <qwmatrix.h>
#include <qpointarray.h>
#include <qregion.h>
#include <qdom.h>

#include <stdlib.h>
#include <fstream.h>
#include <math.h>

#include <kozoomhandler.h>
#include <koRect.h>
#include <koSize.h>
#include <koPoint.h>
#include <kdebug.h>

/******************************************************************/
/* Class: KPObject                                                */
/******************************************************************/

const QString &KPObject::tagORIG=KGlobal::staticQString("ORIG");
const QString &KPObject::attrX=KGlobal::staticQString("x");
const QString &KPObject::attrY=KGlobal::staticQString("y");
const QString &KPObject::tagSIZE=KGlobal::staticQString("SIZE");
const QString &KPObject::attrWidth=KGlobal::staticQString("width");
const QString &KPObject::attrHeight=KGlobal::staticQString("height");
const QString &KPObject::tagSHADOW=KGlobal::staticQString("SHADOW");
const QString &KPObject::attrDistance=KGlobal::staticQString("distance");
const QString &KPObject::attrDirection=KGlobal::staticQString("direction");
const QString &KPObject::attrColor=KGlobal::staticQString("color");
const QString &KPObject::tagEFFECTS=KGlobal::staticQString("EFFECTS");
const QString &KPObject::attrEffect=KGlobal::staticQString("effect");
const QString &KPObject::attrEffect2=KGlobal::staticQString("effect2");
const QString &KPObject::tagPRESNUM=KGlobal::staticQString("PRESNUM");
const QString &KPObject::tagANGLE=KGlobal::staticQString("ANGLE");
const QString &KPObject::tagDISAPPEAR=KGlobal::staticQString("DISAPPEAR");
const QString &KPObject::attrDoit=KGlobal::staticQString("doit");
const QString &KPObject::attrNum=KGlobal::staticQString("num");
const QString &KPObject::tagFILLTYPE=KGlobal::staticQString("FILLTYPE");
const QString &KPObject::tagGRADIENT=KGlobal::staticQString("GRADIENT");
const QString &KPObject::tagPEN=KGlobal::staticQString("PEN");
const QString &KPObject::tagBRUSH=KGlobal::staticQString("BRUSH");
const QString &KPObject::attrValue=KGlobal::staticQString("value");
const QString &KPObject::attrC1=KGlobal::staticQString("color1");
const QString &KPObject::attrC2=KGlobal::staticQString("color2");
const QString &KPObject::attrType=KGlobal::staticQString("type");
const QString &KPObject::attrUnbalanced=KGlobal::staticQString("unbalanced");
const QString &KPObject::attrXFactor=KGlobal::staticQString("xfactor");
const QString &KPObject::attrYFactor=KGlobal::staticQString("yfactor");
const QString &KPObject::attrStyle=KGlobal::staticQString("style");

/*======================== constructor ===========================*/
KPObject::KPObject()
    : orig(), ext(), shadowColor( Qt::gray ), sticky( FALSE )
{
    presNum = 0;
    disappearNum = 1;
    effect = EF_NONE;
    effect2 = EF2_NONE;
    effect3 = EF3_NONE;
    disappear = false;
    appearTimer = 1;
    disappearTimer = 1;
    appearSoundEffect = false;
    disappearSoundEffect = false;
    a_fileName = QString::null;
    d_fileName = QString::null;
    angle = 0.0;
    shadowDirection = SD_RIGHT_BOTTOM;
    shadowDistance = 0;
    selected = false;
    ownClipping = true;
    subPresStep = 0;
    specEffects = false;
    onlyCurrStep = true;
    inObjList = true;
    cmds = 0;
    resize = false;
    sticky = false;
    dcop = 0;
}

KPObject::~KPObject()
{
}

QDomDocumentFragment KPObject::save( QDomDocument& doc, int offset )
{
    QDomDocumentFragment fragment=doc.createDocumentFragment();
    QDomElement elem=doc.createElement(tagORIG);
    elem.setAttribute(attrX, orig.x());
    elem.setAttribute(attrY, orig.y()+offset);
    fragment.appendChild(elem);
    elem=doc.createElement(tagSIZE);
    elem.setAttribute(attrWidth, ext.width());
    elem.setAttribute(attrHeight, ext.height());
    fragment.appendChild(elem);
    if(shadowDistance!=0 || shadowDirection!=SD_RIGHT_BOTTOM) { // no, we don't check the color :)
        elem=doc.createElement(tagSHADOW);
        elem.setAttribute(attrDistance, shadowDistance);
        elem.setAttribute(attrDirection, static_cast<int>( shadowDirection ));
        elem.setAttribute(attrColor, shadowColor.name());
        fragment.appendChild(elem);
    }
    if(effect!=EF_NONE || effect2!=EF2_NONE) {
        elem=doc.createElement(tagEFFECTS);
        elem.setAttribute(attrEffect, static_cast<int>( effect ));
        elem.setAttribute(attrEffect2, static_cast<int>( effect2 ));
        fragment.appendChild(elem);
    }
    if(presNum!=0)
        fragment.appendChild(KPObject::createValueElement(tagPRESNUM, presNum, doc));
    if(angle!=0.0) {
        elem=doc.createElement(tagANGLE);
        elem.setAttribute(attrValue, angle);
        fragment.appendChild(elem);
    }
    if(effect3!=EF3_NONE || disappear) {
        elem=doc.createElement(tagDISAPPEAR);
        elem.setAttribute(attrEffect, static_cast<int>( effect3 ));
        elem.setAttribute(attrDoit, static_cast<int>( disappear ));
        elem.setAttribute(attrNum, disappearNum);
        fragment.appendChild(elem);
    }
    if(appearTimer!=1 || disappearTimer!=1) {
        elem=doc.createElement("TIMER");
        elem.setAttribute("appearTimer", appearTimer);
        elem.setAttribute("disappearTimer", disappearTimer);
        fragment.appendChild(elem);
    }
    if(appearSoundEffect || !a_fileName.isEmpty()) {
        elem=doc.createElement("APPEARSOUNDEFFECT");
        elem.setAttribute("appearSoundEffect", static_cast<int>(appearSoundEffect));
        elem.setAttribute("appearSoundFileName", a_fileName);
        fragment.appendChild(elem);
    }
    if(disappearSoundEffect || !d_fileName.isEmpty()) {
        elem=doc.createElement("DISAPPEARSOUNDEFFECT");
        elem.setAttribute("disappearSoundEffect", static_cast<int>(disappearSoundEffect));
        elem.setAttribute("disappearSoundFileName", d_fileName);
        fragment.appendChild(elem);
    }

    return fragment;
}

int KPObject::load(const QDomElement &element) {

    int offset=0;
    QDomElement e=element.namedItem(tagORIG).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrX))
            orig.setX(e.attribute(attrX).toDouble());
        if(e.hasAttribute(attrY))
        {
            offset=e.attribute(attrY).toInt();
            orig.setY(0);
        }
        origTopLeftPointInGroup = orig;
    }
    e=element.namedItem(tagSIZE).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrWidth))
            ext.setWidth(e.attribute(attrWidth).toDouble());
        if(e.hasAttribute(attrHeight))
            ext.setHeight(e.attribute(attrHeight).toDouble());

        origSizeInGroup = ext;
    }
    e=element.namedItem(tagSHADOW).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrDistance))
            shadowDistance=e.attribute(attrDistance).toInt();
        if(e.hasAttribute(attrDirection))
            shadowDirection=static_cast<ShadowDirection>(e.attribute(attrDirection).toInt());
        shadowColor=retrieveColor(e);
    }
    else {
        shadowDistance=0;
        shadowDirection=SD_RIGHT_BOTTOM;
        shadowColor=Qt::gray;
    }
    e=element.namedItem(tagEFFECTS).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrEffect))
            effect=static_cast<Effect>(e.attribute(attrEffect).toInt());
        if(e.hasAttribute(attrEffect2))
            effect2=static_cast<Effect2>(e.attribute(attrEffect2).toInt());
    }
    else {
        effect=EF_NONE;
        effect2=EF2_NONE;
    }
    e=element.namedItem(tagANGLE).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrValue))
            angle=e.attribute(attrValue).toFloat();
    }
    else
        angle=0.0;
    e=element.namedItem(tagPRESNUM).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrValue))
            presNum=e.attribute(attrValue).toInt();
    }
    else
        presNum=0;
    e=element.namedItem(tagDISAPPEAR).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrEffect))
            effect3=static_cast<Effect3>(e.attribute(attrEffect).toInt());
        if(e.hasAttribute(attrDoit))
            disappear=static_cast<bool>(e.attribute(attrDoit).toInt());
        if(e.hasAttribute(attrNum))
            disappearNum=e.attribute(attrNum).toInt();
    }
    else {
        effect3=EF3_NONE;
        disappear=false;
        disappearNum=1;
    }
    e=element.namedItem("TIMER").toElement();
    if(!e.isNull()) {
        if(e.hasAttribute("appearTimer"))
            appearTimer = e.attribute("appearTimer").toInt();
        if(e.hasAttribute("disappearTimer"))
            disappearTimer = e.attribute("disappearTimer").toInt();
    }
    else {
        appearTimer = 1;
        disappearTimer = 1;
    }
    e=element.namedItem("APPEARSOUNDEFFECT").toElement();
    if(!e.isNull()) {
        if(e.hasAttribute("appearSoundEffect"))
            appearSoundEffect = static_cast<bool>(e.attribute("appearSoundEffect").toInt());
        if(e.hasAttribute("appearSoundFileName"))
            a_fileName = e.attribute("appearSoundFileName");
    }
    else {
        appearSoundEffect = false;
        a_fileName = QString::null;
    }
    e=element.namedItem("DISAPPEARSOUNDEFFECT").toElement();
    if(!e.isNull()) {
        if(e.hasAttribute("disappearSoundEffect"))
            disappearSoundEffect = static_cast<bool>(e.attribute("disappearSoundEffect").toInt());
        if(e.hasAttribute("disappearSoundFileName"))
            d_fileName = e.attribute("disappearSoundFileName");
    }
    else {
        disappearSoundEffect = false;
        d_fileName = QString::null;
    }
    return offset;
}

/*======================= get bounding rect ======================*/
KoRect KPObject::getBoundingRect( KoZoomHandler *_zoomHandler ) const
{
    KoRect r( orig.x() , orig.y(),
             ext.width(), ext.height() );

    if ( shadowDistance > 0 )
    {
        double sx = r.x(), sy = r.y();
        getShadowCoords( sx, sy, _zoomHandler );
        KoRect r2( sx, sy, r.width(), r.height() );
        r = r.unite( r2 );
    }

    if ( angle == 0.0 )
        return r;
    else
    {
        KoRect br = KoRect( 0, 0, ext.width(), ext.height() );
        double pw = br.width();
        double ph = br.height();
        KoRect rr = br;
        double yPos = -rr.y();
        double xPos = -rr.x();
        rr.moveTopLeft( KoPoint( -rr.width() / 2, -rr.height() / 2 ) );
        QWMatrix m;
        m.translate( pw / 2, ph / 2 );
        m.rotate( angle );
        m.translate( rr.left() + xPos, rr.top() + yPos );
        KoRect r = KoRect::fromQRect(m.mapRect( br.toQRect() )); // see above TODO
        r.moveBy( orig.x() , orig.y() );
        return r;
    }
}

/*======================== contain point ? =======================*/
bool KPObject::contains( const KoPoint &_point ) const
{
    if ( angle == 0.0 )
    {
        KoRect r( orig.x() , orig.y() ,
                 ext.width(), ext.height() );
        return r.contains( _point );
    }
    else
    {
        KoRect br = KoRect( 0, 0, ext.width(), ext.height() );
        double pw = br.width();
        double ph = br.height();
        KoRect rr = br;
        double yPos = -rr.y();
        double xPos = -rr.x();
        rr.moveTopLeft( KoPoint( -rr.width() / 2, -rr.height() / 2 ) );

        QWMatrix m;
        m.translate( pw / 2, ph / 2 );
        m.rotate( angle );
        m.translate( rr.left() + xPos, rr.top() + yPos );

        KoRect r = KoRect::fromQRect(m.mapRect( br.toQRect() )); // see above TODO
        r.moveBy( orig.x() , orig.y() );

        return r.contains( _point );
    }
}

/*================================================================*/
bool KPObject::intersects( const KoRect &_rect ) const
{
    if ( angle == 0.0 )
    {
        KoRect r( orig.x(), orig.y(),
                 ext.width(), ext.height() );
        return r.intersects( _rect );
    }
    else
    {
        KoRect br = KoRect( 0, 0, ext.width(), ext.height() );
        double pw = br.width();
        double ph = br.height();
        KoRect rr = br;
        double yPos = -rr.y();
        double xPos = -rr.x();
        rr.moveTopLeft( KoPoint( -rr.width() / 2, -rr.height() / 2 ) );

        QWMatrix m;
        m.translate( pw / 2, ph / 2 );
        m.rotate( angle );
        m.translate( rr.left() + xPos, rr.top() + yPos );

        KoRect r = KoRect::fromQRect(m.mapRect( br.toQRect() ));
        r.moveBy( orig.x(), orig.y() );

        return r.intersects( _rect );
    }
}

/*======================== get cursor ============================*/
QCursor KPObject::getCursor( const KoPoint &_point, ModifyType &_modType ) const
{
    double px = _point.x();
    double py = _point.y();

    double ox = orig.x() ;
    double oy = orig.y();
    double ow = ext.width();
    double oh = ext.height();

    KoRect r( ox, oy, ow, oh );

    if ( !r.contains( _point ) )
        return Qt::arrowCursor;

    if ( px >= ox && py >= oy && px <= ox + 6 && py <= oy + 6 )
    {
        _modType = MT_RESIZE_LU;
        return Qt::sizeFDiagCursor;
    }

    if ( px >= ox && py >= oy + oh / 2 - 3 && px <= ox + 6 && py <= oy + oh / 2 + 3 )
    {
        _modType = MT_RESIZE_LF;
        return Qt::sizeHorCursor;
    }

    if ( px >= ox && py >= oy + oh - 6 && px <= ox + 6 && py <= oy + oh )
    {
        _modType = MT_RESIZE_LD;
        return Qt::sizeBDiagCursor;
    }

    if ( px >= ox + ow / 2 - 3 && py >= oy && px <= ox + ow / 2 + 3 && py <= oy + 6 )
    {
        _modType = MT_RESIZE_UP;
        return Qt::sizeVerCursor;
    }

    if ( px >= ox + ow / 2 - 3 && py >= oy + oh - 6 && px <= ox + ow / 2 + 3 && py <= oy + oh )
    {
        _modType = MT_RESIZE_DN;
        return Qt::sizeVerCursor;
    }

    if ( px >= ox + ow - 6 && py >= oy && px <= ox + ow && py <= oy + 6 )
    {
        _modType = MT_RESIZE_RU;
        return Qt::sizeBDiagCursor;
    }

    if ( px >= ox + ow - 6 && py >= oy + oh / 2 - 3 && px <= ox + ow && py <= oy + oh / 2 + 3 )
    {
        _modType = MT_RESIZE_RT;
        return Qt::sizeHorCursor;
    }

    if ( px >= ox + ow - 6 && py >= oy + oh - 6 && px <= ox + ow && py <= oy + oh )
    {
        _modType = MT_RESIZE_RD;
        return Qt::sizeFDiagCursor;
    }

    _modType = MT_MOVE;
    return Qt::sizeAllCursor;
}

/*======================== draw ==================================*/
void KPObject::draw( QPainter *_painter, KoZoomHandler *_zoomHandler, bool drawSelection )
{
    if ( drawSelection )
        paintSelection( _painter, _zoomHandler );
}

/*====================== get shadow coordinates ==================*/
void KPObject::getShadowCoords( double& _x, double& _y ,KoZoomHandler *_zoomHandler) const
{
    double sx = 0, sy = 0;
    double shadowDir=_zoomHandler->zoomItX(shadowDistance); // ######## ????????
    // The above looks wrong (DF).
    // If shadowDistance is in pixels, it has to be unzoomed, not zoomed.
    // But that's not good anyway, we don't want a constant number of pixels.
    // I think shadowDistance should be a 'double' - then the parameter
    // _zoomHandler can be removed.

    switch ( shadowDirection )
    {
    case SD_LEFT_UP:
    {
        sx = _x - shadowDir;
        sy = _y - shadowDir;
    } break;
    case SD_UP:
    {
        sx = _x;
        sy = _y - shadowDir;
    } break;
    case SD_RIGHT_UP:
    {
        sx = _x + shadowDir;
        sy = _y - shadowDir;
    } break;
    case SD_RIGHT:
    {
        sx = _x + shadowDir;
        sy = _y;
    } break;
    case SD_RIGHT_BOTTOM:
    {
        sx = _x + shadowDir;
        sy = _y + shadowDir;
    } break;
    case SD_BOTTOM:
    {
        sx = _x;
        sy = _y + shadowDir;
    } break;
    case SD_LEFT_BOTTOM:
    {
        sx = _x - shadowDir;
        sy = _y + shadowDir;
    } break;
    case SD_LEFT:
    {
        sx = _x - shadowDir;
        sy = _y;
    } break;
    }

    _x = sx; _y = sy;
}

/*======================== paint selection =======================*/
void KPObject::paintSelection( QPainter *_painter, KoZoomHandler *_zoomHandler )
{
    _painter->save();
    _painter->translate( _zoomHandler->zoomItX(orig.x()), _zoomHandler->zoomItY(orig.y()) );
    Qt::RasterOp rop = _painter->rasterOp();

    _painter->setRasterOp( Qt::NotROP );

    if ( getType() == OT_TEXT && dynamic_cast<KPTextObject*>( this )->getDrawEditRect() )
    {
        _painter->save();

        if ( angle != 0 )
        {
            KoRect br = KoRect( 0, 0, ext.width(), ext.height() );
            double pw = br.width();
            double ph = br.height();
            KoRect rr = br;
            double yPos = -rr.y();
            double xPos = -rr.x();
            rr.moveTopLeft( KoPoint( -rr.width() / 2, -rr.height() / 2 ) );

            QWMatrix m;
            m.translate( pw / 2, ph / 2 );
            m.rotate( angle );
            m.translate( rr.left() + xPos, rr.top() + yPos );

            _painter->setWorldMatrix( m, true );
        }

        _painter->setPen( QPen( Qt::black, 1, Qt::DotLine ) );
        _painter->setBrush( Qt::NoBrush );
        _painter->drawRect( 0, 0, _zoomHandler->zoomItX(ext.width()), _zoomHandler->zoomItY( ext.height()) );

        _painter->restore();
    }

    _painter->setPen( QPen( Qt::black, 1, Qt::SolidLine ) );
    _painter->setBrush( Qt::black );

    if ( selected )
    {
        _painter->fillRect( 0, 0, _zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
        _painter->fillRect( 0, _zoomHandler->zoomItY(ext.height() / 2 - 3), _zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
        _painter->fillRect( 0, _zoomHandler->zoomItY(ext.height() - 6), _zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
        _painter->fillRect( _zoomHandler->zoomItX(ext.width() - 6), 0, _zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
        _painter->fillRect( _zoomHandler->zoomItX(ext.width() - 6), _zoomHandler->zoomItY(ext.height() / 2 - 3), _zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
        _painter->fillRect( _zoomHandler->zoomItX(ext.width() - 6), _zoomHandler->zoomItY(ext.height() - 6), _zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
        _painter->fillRect( _zoomHandler->zoomItX(ext.width() / 2 - 3), 0,_zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
        _painter->fillRect( _zoomHandler->zoomItX(ext.width() / 2 - 3), _zoomHandler->zoomItY(ext.height() - 6),_zoomHandler->zoomItX(6), _zoomHandler->zoomItY(6), Qt::black );
    }

    _painter->setRasterOp( rop );
    _painter->restore();
}

/*======================== do delete =============================*/
void KPObject::doDelete()
{
    if ( cmds == 0 && !inObjList )delete this;
}

/*=============================================================*/
DCOPObject* KPObject::dcopObject()
{
    if ( !dcop )
        dcop = new KPresenterObjectIface( this );

    return dcop;
}

void KPObject::setupClipRegion( QPainter *painter, const QRegion &clipRegion )
{
    QRegion region = painter->clipRegion( QPainter::CoordPainter );
    if ( region.isEmpty() )
        region = clipRegion;
    else
        region.unite( clipRegion );

    painter->setClipRegion( region, QPainter::CoordPainter );
}

QDomElement KPObject::createValueElement(const QString &tag, int value, QDomDocument &doc) {
    QDomElement elem=doc.createElement(tag);
    elem.setAttribute(attrValue, value);
    return elem;
}

QDomElement KPObject::createGradientElement(const QString &tag, const QColor &c1, const QColor &c2,
                                            int type, bool unbalanced, int xfactor, int yfactor, QDomDocument &doc) {
    QDomElement elem=doc.createElement(tag);
    elem.setAttribute(attrC1, c1.name());
    elem.setAttribute(attrC2, c2.name());
    elem.setAttribute(attrType, type);
    elem.setAttribute(attrUnbalanced, (uint)unbalanced);
    elem.setAttribute(attrXFactor, xfactor);
    elem.setAttribute(attrYFactor, yfactor);
    return elem;
}

void KPObject::toGradient(const QDomElement &element, QColor &c1, QColor &c2, BCType &type,
                          bool &unbalanced, int &xfactor, int &yfactor) const {
    c1=retrieveColor(element, attrC1, "red1", "green1", "blue1");
    c2=retrieveColor(element, attrC2, "red2", "green2", "blue2");
    if(element.hasAttribute(attrType))
        type=static_cast<BCType>(element.attribute(attrType).toInt());
    if(element.hasAttribute(attrUnbalanced))
        unbalanced=static_cast<bool>(element.attribute(attrUnbalanced).toInt());
    if(element.hasAttribute(attrXFactor))
        xfactor=element.attribute(attrXFactor).toInt();
    if(element.hasAttribute(attrYFactor))
        yfactor=element.attribute(attrYFactor).toInt();
}

QDomElement KPObject::createPenElement(const QString &tag, const QPen &pen, QDomDocument &doc) {

    QDomElement elem=doc.createElement(tag);
    elem.setAttribute(attrColor, pen.color().name());
    elem.setAttribute(attrWidth, pen.width());
    elem.setAttribute(attrStyle, static_cast<int>(pen.style()));
    return elem;
}

QPen KPObject::toPen(const QDomElement &element) const {

    QPen pen;
    pen.setColor(retrieveColor(element));
    if(element.hasAttribute(attrStyle))
        pen.setStyle(static_cast<Qt::PenStyle>(element.attribute(attrStyle).toInt()));
    if(element.hasAttribute(attrWidth))
        pen.setWidth(element.attribute(attrWidth).toInt());
    return pen;
}

QDomElement KPObject::createBrushElement(const QString &tag, const QBrush &brush, QDomDocument &doc) {

    QDomElement elem=doc.createElement(tag);
    elem.setAttribute(attrColor, brush.color().name());
    elem.setAttribute(attrStyle, static_cast<int>(brush.style()));
    return elem;
}

QBrush KPObject::toBrush(const QDomElement &element) const {

    QBrush brush;
    brush.setColor(retrieveColor(element));
    if(element.hasAttribute(attrStyle))
        brush.setStyle(static_cast<Qt::BrushStyle>(element.attribute(attrStyle).toInt()));
    return brush;
}

QColor KPObject::retrieveColor(const QDomElement &element, const QString &cattr,
                               const QString &rattr, const QString &gattr, const QString &battr) const {
    QColor ret;
    if(element.hasAttribute(cattr))
        ret.setNamedColor(element.attribute(cattr));
    else {
        int red=0, green=0, blue=0;
        if(element.hasAttribute(rattr))
            red=element.attribute(rattr).toInt();
        if(element.hasAttribute(gattr))
            green=element.attribute(gattr).toInt();
        if(element.hasAttribute(battr))
            blue=element.attribute(battr).toInt();
        ret.setRgb(red, green, blue);
    }
    return ret;
}

KP2DObject::KP2DObject()
    : KPObject(), pen(), brush(), gColor1( Qt::red ), gColor2( Qt::green )
{
    gradient = 0;
    fillType = FT_BRUSH;
    gType = BCT_GHORZ;
    unbalanced = false;
    xfactor = 100;
    yfactor = 100;
}

KP2DObject::KP2DObject( const QPen &_pen, const QBrush &_brush, FillType _fillType,
                        const QColor &_gColor1, const QColor &_gColor2, BCType _gType,
                        bool _unbalanced, int _xfactor, int _yfactor )
    : KPObject(), pen( _pen ), brush( _brush ), gColor1( _gColor1 ), gColor2( _gColor2 )
{
    gType = _gType;
    fillType = _fillType;
    unbalanced = _unbalanced;
    xfactor = _xfactor;
    yfactor = _yfactor;

    if ( fillType == FT_GRADIENT )
        gradient = new KPGradient( gColor1, gColor2, gType, unbalanced, xfactor, yfactor );
    else
        gradient = 0;
}

KP2DObject &KP2DObject::operator=( const KP2DObject & )
{
    return *this;
}

void KP2DObject::setFillType( FillType _fillType )
{
    fillType = _fillType;

    if ( fillType == FT_BRUSH && gradient )
    {
        delete gradient;
        gradient = 0;
    }
    if ( fillType == FT_GRADIENT && !gradient )
        gradient = new KPGradient( gColor1, gColor2, gType, unbalanced, xfactor, yfactor );
}

QDomDocumentFragment KP2DObject::save( QDomDocument& doc,int offset )
{
    QDomDocumentFragment fragment=KPObject::save(doc, offset);
    if(fillType!=FT_BRUSH)
        fragment.appendChild(KPObject::createValueElement(tagFILLTYPE, static_cast<int>(fillType), doc));
    if(gColor1!=Qt::red || gColor2!=Qt::green || gType!=BCT_GHORZ || unbalanced || xfactor!=100 || yfactor!=100)
        fragment.appendChild(KPObject::createGradientElement(tagGRADIENT, gColor1, gColor2, static_cast<int>(gType),
                                                             unbalanced, xfactor, yfactor, doc));
    if(pen.color()!=Qt::black || pen.width()!=1 || pen.style()!=Qt::SolidLine)
        fragment.appendChild(KPObject::createPenElement(tagPEN, pen, doc));
    if(brush.color()!=Qt::black || brush.style()!=Qt::NoBrush)
        fragment.appendChild(KPObject::createBrushElement(tagBRUSH, brush, doc));
    return fragment;
}

int KP2DObject::load(const QDomElement &element)
{
    int offset=KPObject::load(element);
    QDomElement e=element.namedItem(tagPEN).toElement();
    if(!e.isNull())
        setPen(KPObject::toPen(e));
    else
        pen=QPen();
    e=element.namedItem(tagBRUSH).toElement();
    if(!e.isNull())
        setBrush(KPObject::toBrush(e));
    else
        brush=QBrush();
    e=element.namedItem(tagFILLTYPE).toElement();
    if(!e.isNull()) {
        if(e.hasAttribute(attrValue))
            setFillType(static_cast<FillType>(e.attribute(attrValue).toInt()));
    }
    else
        setFillType(FT_BRUSH);
    e=element.namedItem(tagGRADIENT).toElement();
    if(!e.isNull()) {
        KPObject::toGradient(e, gColor1, gColor2, gType, unbalanced, xfactor, yfactor);
        if(gradient)
            gradient->setParameters(gColor1, gColor2, gType, unbalanced, xfactor, yfactor);
    }
    else {
        gColor1=Qt::red;
        gColor2=Qt::green;
        gType=BCT_GHORZ;
        unbalanced=false;
        xfactor=100;
        yfactor=100;
    }
    return offset;
}

void KP2DObject::draw( QPainter *_painter, KoZoomHandler*_zoomHandler, bool drawSelection )
{
    double ox = orig.x();
    double oy = orig.y();
    double ow = ext.width();
    double oh = ext.height();
    _painter->save();

    // Draw the shadow if any
    if ( shadowDistance > 0 )
    {
        QPen tmpPen( pen );
        pen.setColor( shadowColor );
        QBrush tmpBrush( brush );
        brush.setColor( shadowColor );

        if ( angle == 0 )
        {
            double sx = ox;
            double sy = oy;
            getShadowCoords( sx, sy, _zoomHandler );

            _painter->translate( _zoomHandler->zoomItX(sx), _zoomHandler->zoomItY( sy) );
            paint( _painter, _zoomHandler, true /* drawing shadow */ );
        }
        else
        {
            _painter->translate( _zoomHandler->zoomItX(ox), _zoomHandler->zoomItY(oy) );

            KoRect br = KoRect( 0, 0, ow, oh );
            double pw = br.width();
            double ph = br.height();
            KoRect rr = br;
            double yPos = -rr.y();
            double xPos = -rr.x();
            rr.moveTopLeft( KoPoint( -rr.width() / 2, -rr.height() / 2 ) );

            double sx = 0;
            double sy = 0;
            getShadowCoords( sx, sy, _zoomHandler );

            QWMatrix m;
            m.translate( _zoomHandler->zoomItX(pw / 2), _zoomHandler->zoomItY(ph / 2) );
            m.rotate( angle );
            m.translate( _zoomHandler->zoomItX( rr.left() + xPos + sx),_zoomHandler->zoomItY( rr.top() + yPos + sy) );

            _painter->setWorldMatrix( m, true );
            paint( _painter, _zoomHandler, true /* drawing shadow */ );
        }

        pen = tmpPen;
        brush = tmpBrush;
    }

    _painter->restore();

    _painter->save();
    _painter->translate( _zoomHandler->zoomItX(ox), _zoomHandler->zoomItY(oy) );

    if ( angle == 0 )
        paint( _painter,_zoomHandler, false );
    else
    {
        KoRect br = KoRect( 0, 0, ow, oh );
        double pw = br.width();
        double ph = br.height();
        KoRect rr = br;
        double yPos = -rr.y();
        double xPos = -rr.x();
        rr.moveTopLeft( KoPoint( -rr.width() / 2, -rr.height() / 2 ) );

        QWMatrix m;
        m.translate( _zoomHandler->zoomItX(pw / 2), _zoomHandler->zoomItY(ph / 2) );
        m.rotate( angle );
        m.translate( _zoomHandler->zoomItX(rr.left() + xPos), _zoomHandler->zoomItY(rr.top() + yPos) );

        _painter->setWorldMatrix( m, true );
        paint( _painter, _zoomHandler, false );
    }

    _painter->restore();

    KPObject::draw( _painter, _zoomHandler, drawSelection );
}
