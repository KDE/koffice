/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <wtrobin@mandrakesoft.com>

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

// This file has to be included, or kimageeffect will fail to compile!?!
#include <qimage.h>
#include <qdom.h>

#include <kdebug.h>
#include <kglobal.h>

#include <gobject.h>
#include <graphiteglobal.h>


const bool operator==(const Gradient &lhs, const Gradient &rhs) {

    return lhs.ca==rhs.ca && lhs.cb==rhs.cb && lhs.type==rhs.type &&
           lhs.xfactor==rhs.xfactor && lhs.yfactor==rhs.yfactor;
}

const bool operator!=(const Gradient &lhs, const Gradient &rhs) {
    return !operator==(lhs, rhs);
}

Gradient &Gradient::operator=(const Gradient &rhs) {

    ca=rhs.ca;
    cb=rhs.cb;
    type=rhs.type;
    xfactor=rhs.xfactor;
    yfactor=rhs.yfactor;
    return *this;
}

GraphiteGlobal *GraphiteGlobal::m_self=0;

GraphiteGlobal *GraphiteGlobal::self() {

    if(m_self==0L)
	m_self=new GraphiteGlobal();
    return m_self;	
}

void GraphiteGlobal::setHandleSize(const int &handleSize) {
    m_handleSize=handleSize;
    m_offset=Graphite::double2Int(static_cast<double>(handleSize)*0.5);
}

void GraphiteGlobal::setRotHandleSize(const int &rotHandleSize) {
    m_rotHandleSize=rotHandleSize;
    m_offset=Graphite::double2Int(static_cast<double>(rotHandleSize)*0.5);
}

void GraphiteGlobal::setUnit(const Unit &unit) {

    m_unit=unit;
    if(unit==MM)
	m_unitString=QString::fromLatin1("mm");
    else if(unit==Inch)
	m_unitString=QString::fromLatin1("inch");
    else
	m_unitString=QString::fromLatin1("pt");
}

void GraphiteGlobal::setZoom(const double &zoom) {
    m_zoom=zoom;
}

void GraphiteGlobal::setResoltuion(const int &resolution) {
    m_resolution=static_cast<double>(resolution)/25.399956;
}

QDomElement GraphiteGlobal::createElement(const QString &tagName, const QPen &pen, QDomDocument &doc) const {

    static const QString &attrPenColor=KGlobal::staticQString("color");
    static const QString &attrPenStyle=KGlobal::staticQString("style");
    static const QString &attrWidth=KGlobal::staticQString("width");
    static const QString &attrPenJoinStyle=KGlobal::staticQString("joinstyle");
    static const QString &attrPenCapStyle=KGlobal::staticQString("capstyle");

    QDomElement e=doc.createElement(tagName);
    e.setAttribute(attrPenColor, pen.color().name());
    e.setAttribute(attrPenStyle, static_cast<int>(pen.style()));
    e.setAttribute(attrWidth, pen.width());
    e.setAttribute(attrPenJoinStyle, pen.joinStyle());
    e.setAttribute(attrPenCapStyle, pen.capStyle());
    return e;
}

QPen GraphiteGlobal::toPen(const QDomElement &element) const {

    QPen pen;

    static const QString &attrPenColor=KGlobal::staticQString("color");
    static const QString &attrPenStyle=KGlobal::staticQString("style");
    static const QString &attrWidth=KGlobal::staticQString("width");
    static const QString &attrPenJoinStyle=KGlobal::staticQString("joinstyle");
    static const QString &attrPenCapStyle=KGlobal::staticQString("capstyle");

    if(element.hasAttribute(attrPenColor))
	pen.setColor(QColor(element.attribute(attrPenColor)));
    if(element.hasAttribute(attrPenStyle))
	pen.setStyle(static_cast<Qt::PenStyle>(element.attribute(attrPenStyle).toInt()));
    if(element.hasAttribute(attrWidth))
	pen.setWidth(element.attribute(attrWidth).toInt());
    if(element.hasAttribute(attrPenJoinStyle))
	pen.setJoinStyle(static_cast<Qt::PenJoinStyle>(element.attribute(attrPenJoinStyle).toInt()));
    if(element.hasAttribute(attrPenCapStyle))
	pen.setCapStyle(static_cast<Qt::PenCapStyle>(element.attribute(attrPenCapStyle).toInt()));
    return pen;
}

QDomElement GraphiteGlobal::createElement(const QString &tagName, const QRect &rect, QDomDocument &doc) const {

    static const QString &attrX=KGlobal::staticQString("x");
    static const QString &attrY=KGlobal::staticQString("y");
    static const QString &attrWidth=KGlobal::staticQString("width");
    static const QString &attrHeight=KGlobal::staticQString("height");

    QDomElement e=doc.createElement(tagName);
    e.setAttribute(attrX, rect.left());
    e.setAttribute(attrY, rect.top());
    e.setAttribute(attrWidth, rect.width());
    e.setAttribute(attrHeight, rect.height());
    return e;
}

QRect GraphiteGlobal::toRect(const QDomElement &element) const {

    QRect rect;

    static const QString &attrX=KGlobal::staticQString("x");
    static const QString &attrY=KGlobal::staticQString("y");
    static const QString &attrWidth=KGlobal::staticQString("width");
    static const QString &attrHeight=KGlobal::staticQString("height");

    if(element.hasAttribute(attrX))
	rect.setTop(element.attribute(attrX).toInt());
    if(element.hasAttribute(attrY))
	rect.setTop(element.attribute(attrY).toInt());
    if(element.hasAttribute(attrWidth))
	rect.setTop(element.attribute(attrWidth).toInt());
    if(element.hasAttribute(attrHeight))
	rect.setTop(element.attribute(attrHeight).toInt());
    return rect;
}

GraphiteGlobal::GraphiteGlobal() : m_fuzzyBorder(3), m_handleSize(4),
				   m_rotHandleSize(4), m_thirdHandleTrigger(20),
				   m_offset(2), m_unit(MM), m_zoom(1.0),
				   m_resolution(2.8346457) {
    m_unitString=QString::fromLatin1("mm");
}


FxValue::FxValue() : m_value(0.0), m_pixel(0) {
}

FxValue::FxValue(const int &pixel) {
    setPxValue(pixel);
}

FxValue::FxValue(const FxValue &v) : m_value(v.value()), m_pixel(v.pxValue()) {
}

FxValue &FxValue::operator=(const FxValue &rhs) {

    m_value=rhs.value();
    m_pixel=rhs.pxValue();
    return *this;
}

const bool FxValue::operator==(const FxValue &rhs) {
    return m_pixel==rhs.pxValue();
}

const bool FxValue::operator!=(const FxValue &rhs) {
    return m_pixel!=rhs.pxValue();
}

void FxValue::setValue(const double &value) {
    m_value=value;
    recalculate();
}

void FxValue::setPxValue(const int &/*pixel*/) {
    // TODO
}

const double FxValue::valueUnit() const {

    if(GraphiteGlobal::self()->unit()==GraphiteGlobal::MM)
	return valueMM();
    else if(GraphiteGlobal::self()->unit()==GraphiteGlobal::Inch)
	return valueInch();
    else
	return valuePt();
}

const double FxValue::valueMM() const {
    return 0.0;
}

const double FxValue::valueInch() const {
    return 0.0;
}

const double FxValue::valuePt() const {
    return 0.0;
}

void FxValue::recalculate() {
}
