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

#include <qdom.h>
#include <qapp.h>
#include <qpainter.h>
#include <klocale.h>

#include <gline.h>


GLine::GLine(const QPoint &a, const QPoint &b, const QString &name) : GObject(name),
								      m_a(a), m_b(b) {
}

GLine::GLine(const QString &name) : GObject(name) {
}

GLine::GLine(const GLine &rhs) : GObject(rhs), m_a(rhs.a()), m_b(rhs.b()) {
}

GLine::GLine(const QDomElement &element) :
    GObject(element.namedItem(QString::fromLatin1("gobject")).toElement()) {

    if(!m_ok)
	return;

    bool ok;
    static QString tagGLine=QString::fromLatin1("gline");
    static QString tagStart=QString::fromLatin1("start");
    static QString tagEnd=QString::fromLatin1("end");
    static QString attrX=QString::fromLatin1("x");
    static QString attrY=QString::fromLatin1("y");

    if(element.tagName()!=tagGLine) {
	m_ok=false;
	return;
    }

    QDomElement point=element.namedItem(tagStart).toElement();
    if(!point.isNull()) {
	m_a.setX(point.attribute(attrX).toInt(&ok));
	if(!ok)
	    m_a.setX(0);
	
	m_a.setY(point.attribute(attrY).toInt(&ok));
	if(!ok)
	    m_a.setX(0);
    }
    else {
	m_a.setX(0);
	m_a.setY(0);
    }

    point=element.namedItem(tagEnd).toElement();
    if(!point.isNull()) {
	m_b.setX(point.attribute(attrX).toInt(&ok));
	if(!ok)
	    m_b.setX(0);
	
	m_b.setY(point.attribute(attrY).toInt(&ok));
	if(!ok)
	    m_b.setX(0);
    }
    else {
	m_b.setX(0);
	m_b.setY(0);
    }
}

GLine *GLine::clone() const {
    return new GLine(*this);
}

GLine *GLine::instantiate(const QDomElement &element) const {
    return new GLine(element);
}

QDomElement GLine::save(QDomDocument &doc) const {

    static QString tagGLine=QString::fromLatin1("gline");
    static QString tagStart=QString::fromLatin1("start");
    static QString tagEnd=QString::fromLatin1("end");
    static QString attrX=QString::fromLatin1("x");
    static QString attrY=QString::fromLatin1("y");
    QDomElement e=doc.createElement(tagGLine);
    QDomElement point=doc.createElement(tagStart);
    point.setAttribute(attrX, m_a.x());
    point.setAttribute(attrY, m_a.y());
    e.appendChild(point);
    point=doc.createElement(tagEnd);
    point.setAttribute(attrX, m_b.x());
    point.setAttribute(attrY, m_b.y());
    e.appendChild(point);
    e.appendChild(GObject::save(doc));
    return e;
}

void GLine::draw(QPainter &p, QRegion &reg, const bool toPrinter) {

    if(m_state==GObject::Deleted || (toPrinter && m_state==GObject::Invisible))
	return;

    if(!reg.contains(boundingRect()))
	return;

    p.save();
    p.setPen(m_pen);
    p.drawLine(m_a, m_b);
    p.restore();

    // resize the region
    reg+=boundingRect();
}

void GLine::drawHandles(QPainter &p, QList<QRect> *handles) {

    p.save();
    p.setPen(Qt::black);
    p.setBrush(Qt::black);

    int size;
    if(m_state==Handles)
	size=GraphiteGlobal::self()->handleSize();
    else if(m_state==Rot_Handles)
	size=GraphiteGlobal::self()->rotHandleSize();
    else
	return; // no need to draw handles - shouldn't happen

    int offset=Graphite::double2Int(static_cast<double>(size)*0.5);

    QRect *r1=new QRect(m_a.x()-offset, m_a.y()-offset, size, size);
    QRect *r2=new QRect(m_b.x()-offset, m_b.y()-offset, size, size);
    QRect *r3=0L;

    if(boundingRect().width()>GraphiteGlobal::self()->thirdHandleTrigger() ||
       boundingRect().height()>GraphiteGlobal::self()->thirdHandleTrigger())
	r3=new QRect(m_a.x()+Graphite::double2Int(static_cast<double>(m_b.x()-m_a.x())*0.5)-offset,
		     m_a.y()+Graphite::double2Int(static_cast<double>(m_b.y()-m_a.y())*0.5)-offset,
		     size, size);

    if(m_state==Handles) {
	p.drawRect(*r1);
	p.drawRect(*r2);
	if(r3)
	    p.drawRect(*r3);
    }
    else {
	p.drawEllipse(*r1);
	p.drawEllipse(*r2);
	if(r3)
	    p.drawEllipse(*r3);
    }

    if(handles) {
	handles->append(r1);
	handles->append(r2);
	if(r3)
	    handles->append(r3);
    }
    else {
	delete r1;
	delete r2;
	delete r3;
    }
    p.restore();
}

const GLine *GLine::hit(const QPoint &p) const {

    if(p==m_a)
	return this;
    else if(p==m_b)
	return this;
    else {
	double dx=static_cast<double>(m_b.x()-m_a.x());
	double dy=static_cast<double>(m_b.y()-m_a.y());
	double r=std::sqrt( dx*dx + dy*dy );
	int ir=Graphite::double2Int(r);
	double alpha1=std::asin( QABS(dy)/r );
	double alpha;
	
	if(dx>=0 && dy>=0)
	    alpha=-alpha1;
	else if(dx<0 && dy>=0)
	    alpha=alpha1+M_PI;
	else if(dx<0 && dy<0)
	    alpha=M_PI-alpha1;
	else // dx>=0 && dy<0
	    alpha=alpha1;
	
	// make it easier for the user to select something by
	// adding a (configurable) "fuzzy zone" :)
	int w=Graphite::double2Int(static_cast<double>(m_pen.width())/2.0);
	int fb=GraphiteGlobal::self()->fuzzyBorder();
        QRect fuzzyZone=QRect( QMIN( m_a.x(), m_a.x() + ir ) - fb - w,
			       m_a.y() - fb - w,
			       ir + 2*fb + w + 1, 2*fb + w + 1 );
	// Don't change the original point!
	QPoint tmp=p;
	rotatePoint(tmp, -alpha, m_a);

	if(fuzzyZone.contains(tmp))
	    return this;
    }
    return 0L;
}

const bool GLine::intersects(const QRect &r) const {

    if(r.contains(m_a) || r.contains(m_b))
	return true;
    else if(r.intersects(boundingRect()))
	return true;
    else {
	// f(x)=mx+d
	double m=static_cast<double>(m_b.y()-m_a.y())/static_cast<double>(m_b.x()-m_a.x());
	double d=m_a.y()-m*m_a.x();
	
	// top
	double i=(static_cast<double>(r.top())-d)/m;
	if(i>=r.left() && i<=r.right())
	    return true;
	
	// bottom
	i=(static_cast<double>(r.bottom())-d)/m;
	if(i>=r.left() && i<=r.right())
	    return true;
	
	// left
	i=m*static_cast<double>(r.left())+d;
	if(i>=r.top() && i<=r.bottom())
	    return true;
	
	// right
	i=m*static_cast<double>(r.right())+d;
	if(i>=r.top() && i<=r.bottom())
	    return true;	
    }
    return false;
}

const QRect &GLine::boundingRect() const {

    if(!m_boundingRectDirty)
	return m_boundingRect;
    m_boundingRect=QRect( QMIN(m_a.x(), m_b.x()), QMIN(m_a.y(), m_b.y()),
			  QABS(m_a.x()-m_b.x()), QABS(m_a.y()-m_b.y()));
    m_boundingRectDirty=false;
    return m_boundingRect;				
}

GObjectM9r *GLine::createM9r(GraphitePart *part, GraphiteView *view,
			     const GObjectM9r::Mode &mode) {
    return new GLineM9r(this, mode, part, view, i18n("Line"));
}

void GLine::setOrigin(const QPoint &origin) {

    m_b.setX(m_b.x()-m_a.x()+origin.x());
    m_b.setY(m_b.y()-m_a.y()+origin.y());
    m_a=origin;
    m_boundingRectDirty=true;
}

void GLine::moveX(const int &dx) {

    m_a.setX(m_a.x()+dx);
    m_b.setX(m_b.x()+dx);
    m_boundingRectDirty=true;
}

void GLine::moveY(const int &dy) {

    m_a.setY(m_a.y()+dy);
    m_b.setY(m_b.y()+dy);
    m_boundingRectDirty=true;
}

void GLine::move(const int &dx, const int &dy) {

    moveX(dx);
    moveY(dy);
}

void GLine::rotate(const QPoint &center, const double &angle) {

    rotatePoint(m_a, angle, center);
    rotatePoint(m_b, angle, center);
    m_boundingRectDirty=true;
}

const double &GLine::angle() const {

    m_angle=std::atan( static_cast<double>(m_b.y()-m_a.y()) /
		       static_cast<double>(m_b.x()-m_a.x()) );
    return m_angle;
}

void GLine::scale(const QPoint &origin, const double &xfactor, const double &yfactor) {

    scalePoint(m_a, xfactor, yfactor, origin);
    scalePoint(m_b, xfactor, yfactor, origin);
    m_boundingRectDirty=true;
}

void GLine::resize(const QRect &boundingRect) {

    m_boundingRect=boundingRect;
    m_boundingRectDirty=false;
    m_a=boundingRect.topLeft();
    m_b=boundingRect.bottomRight();
}


GLineM9r::GLineM9r(GLine *line, const Mode &mode, GraphitePart *part,
		   GraphiteView *view, const QString &type) :
    G1DObjectM9r(line, mode, part, view, type), m_line(line) {
    m_line->setState(GObject::Handles);
}

GLineM9r::~GLineM9r() {
    if(m_line->state()==GObject::Handles || m_line->state()==GObject::Rot_Handles)
	m_line->setState(GObject::Visible);
}

void GLineM9r::draw(QPainter &p) {
    m_line->drawHandles(p, m_handles);
}

const bool GLineM9r::mouseMoveEvent(QMouseEvent */*e*/, QRect &/*dirty*/) {
    // TODO
    return false;
}

const bool GLineM9r::mousePressEvent(QMouseEvent */*e*/, QRect &/*dirty*/) {
    // TODO
    return false;
}

const bool GLineM9r::mouseReleaseEvent(QMouseEvent */*e*/, QRect &/*dirty*/) {
    // TODO
    return false;
}

const bool GLineM9r::mouseDoubleClickEvent(QMouseEvent */*e*/, QRect &/*dirty*/) {
    // TODO
    return false;
}

const bool GLineM9r::keyPressEvent(QKeyEvent */*e*/, QRect &/*dirty*/) {
    // TODO
    return false;
}

const bool GLineM9r::keyReleaseEvent(QKeyEvent */*e*/, QRect &/*dirty*/) {
    // We don't need that one for lines... hmmm ...
    return false;
}
#include <gline.moc>
