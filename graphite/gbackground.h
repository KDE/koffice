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

#ifndef gbackground_h
#define gbackground_h

#include <ggroup.h>

class GBackground;
class QPopupMenu;

class GBackgroundM9r : public G2DObjectM9r {

    Q_OBJECT
public:
    GBackgroundM9r(GBackground *background, const Mode &mode, GraphitePart *part,
              GraphiteView *view, const QString &type);
    virtual ~GBackgroundM9r();

    // never draw any handles for the background
    virtual void draw(QPainter &) {}

    // RMB popup on press
    virtual bool mousePressEvent(QMouseEvent *e, QRect &dirty);
    // Property dia on DblClick
    virtual bool mouseDoubleClickEvent(QMouseEvent *e, QRect &dirty);

private:
    GBackgroundM9r(const GBackgroundM9r &rhs);
    GBackgroundM9r &operator=(GBackgroundM9r &rhs);

    GBackground *m_background;
    QPopupMenu *m_popup;
};


class GBackground : public GAbstractGroup {

public:
    GBackground(const QString &name=QString::null) : GAbstractGroup(name), m_transparent(false) {}
    GBackground(const GBackground &rhs) : GAbstractGroup(rhs), m_transparent(false) {}
    GBackground(const QDomElement &element);

    virtual ~GBackground() {}

    // relaxed return type (Stroustrup, p.425)
    virtual GBackground *clone() const;
    virtual GBackground *instantiate(const QDomElement &element) const;

    virtual QDomElement save(QDomDocument &doc) const;

    void setTransparent(bool transparent=true) { m_transparent=transparent; }
    bool transparent() const { return m_transparent; }

    virtual void draw(QPainter &p, const QRect &rect, bool toPrinter=false) const;

    virtual const QRect &boundingRect() const;

    virtual GBackgroundM9r *createM9r(GraphitePart *part, GraphiteView *view,
                                      const GObjectM9r::Mode &mode=GObjectM9r::Manipulate) const;

    virtual const FxPoint origin() const;
    virtual void setOrigin(const FxPoint &origin);

    virtual void resize(const FxRect &boundingRect);

protected:
    virtual void recalculate() const;

private:
    GBackground &operator=(const GBackground &rhs);

    FxRect m_rect;
    bool m_transparent;
};

#endif // gbackground_h
