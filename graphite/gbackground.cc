/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>

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

#include <qpopupmenu.h>
#include <qpainter.h>

#include <klocale.h>
#include <kglobal.h>
#include <kaction.h>

#include <gbackground.h>
#include <graphitepart.h>

GBackgroundM9r::GBackgroundM9r(GBackground *background, const Mode &/*mode*/, GraphitePart *part,
                     GraphiteView *view, const QString &type) :
    G2DObjectM9r(background, GObjectM9r::Manipulate, part, view, type, false),
    m_background(background), m_popup(0) {
}

GBackgroundM9r::~GBackgroundM9r() {
    delete m_popup;
}

bool GBackgroundM9r::gmousePressEvent(QMouseEvent *e, QRect &/*dirty*/) {

    if(e->button()==Qt::RightButton) {
        if(m_popup==0) {
            m_popup=new QPopupMenu();
            KActionCollection *collection=document()->actionCollection();
            if(collection!=0) {
                KAction *action=collection->action("edit_undo");
                if(action)
                    action->plug(m_popup);
                action=collection->action("edit_redo");
                if(action)
                    action->plug(m_popup);
                m_popup->insertSeparator();
            }
            m_popup->insertItem(i18n("&Properties..."), this, SLOT(showPropertyDialog()));
        }
        m_popup->popup(e->globalPos());
        return true;
    }
    else
        return false;
}

bool GBackgroundM9r::gmouseDoubleClickEvent(QMouseEvent */*e*/, QRect &/*dirty*/) {
    showPropertyDialog();
    // ### repaint, calculate a dirty area
    return true;
}


GBackground::GBackground(const QDomElement &element) :
    GAbstractGroup(element.namedItem(QString::fromLatin1("gabstractgroup")).toElement()) {

    if(!isOk())
        return;

    static const QString &tagGBackground=KGlobal::staticQString("gbackground");
    static const QString &attrTransparent=KGlobal::staticQString("transparent");

    if(element.tagName()!=tagGBackground) {
        setOk(false);
        return;
    }

    bool ok=true;
    m_transparent=static_cast<bool>(element.attribute(attrTransparent).toInt(&ok));
    if(!ok)
        m_transparent=false;
}

GBackground *GBackground::clone() const {
    return new GBackground(*this);
}

GBackground *GBackground::instantiate(const QDomElement &element) const {
    return new GBackground(element);
}

QDomElement GBackground::save(QDomDocument &doc) const {

    static const QString &tagGBackground=KGlobal::staticQString("gbackground");
    static const QString &attrTransparent=KGlobal::staticQString("transparent");
    QDomElement element=doc.createElement(tagGBackground);
    element.setAttribute(attrTransparent, static_cast<int>(m_transparent));
    element.appendChild(GAbstractGroup::save(doc));
    return element;
}

void GBackground::draw(QPainter &p, const QRect &rect, bool toPrinter) const {

    if(dirty())
        recalculate();

    // do we really have to draw ourselves?
    if(!rect.intersects(boundingRect()))
        return;

    // okay, let's draw a nice background
    if(!m_transparent) {
        p.save();
        if(fillStyle()==GObject::Brush) {
            p.setBrush(brush());
            p.setPen(pen());
            p.drawRect(m_rect.pxRect());
        }
        //else {
            // we have a bg gradient...
            // let's wait for Dirk's answer
            // Update: Seems that gradients won't be supported as bg (too slow and mem-hungry)
        //}
        p.restore();
    }
    GAbstractGroup::draw(p, rect, toPrinter);
}

const QRect &GBackground::boundingRect() const {

    if(!boundingRectDirty())
        return GObject::boundingRect();

    setBoundingRect(GAbstractGroup::boundingRect().unite(m_rect.pxRect()));
    setBoundingRectDirty(false);
    return GObject::boundingRect();
}

GBackgroundM9r *GBackground::createM9r(GraphitePart *part, GraphiteView *view,
                                       const GObjectM9r::Mode &mode) const {
    // Yes, this const_cast is ugly, I know
    return new GBackgroundM9r(const_cast<GBackground*>(this), mode, part, view, i18n("Background"));
}

const FxPoint GBackground::origin() const {
    return m_rect.topLeft();
}

void GBackground::setOrigin(const FxPoint &origin) {
    m_rect.moveTopLeft(origin);
    setBoundingRectDirty(true);
}

void GBackground::resize(const FxRect &boundingRect) {
    m_rect=boundingRect;
    setBoundingRectDirty(true);
}

void GBackground::recalculate() const {
    m_rect.recalculate();
    GAbstractGroup::recalculate();
}

#include <gbackground.moc>
