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

#ifndef gprimitivecmds_h
#define gprimitivecmds_h

#include <gcommand.h>
#include <gobject.h>

class QPoint;
class QRect;
class QBrush;
class QPen;


class GMoveCmd : public GCommand {

public:
    GMoveCmd(GObject *object, const QString &name);
    GMoveCmd(GObject *object, const QString &name, const QPoint &from,
             const QPoint &to);
    virtual ~GMoveCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setFrom(const QPoint &from) { m_from=from; }
    const QPoint &from() const { return m_from; }
    void setTo(const QPoint &to) { m_to=to; }
    const QPoint &to() const { return m_to; }

private:
    GObject *m_object;
    QPoint m_from, m_to;
};


class GResizeCmd : public GCommand {

public:
    GResizeCmd(GObject *object, const QString &name);
    GResizeCmd(GObject *object, const QString &name,
               const QRect &oldSize, const QRect &newSize);
    virtual ~GResizeCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setOldSize(const QRect &oldSize) { m_old=oldSize; }
    const QRect &oldSize() const { return m_old; }
    void setNewSize(const QRect &newSize) { m_new=newSize; }
    const QRect &newSize() const { return m_new; }

private:
    GObject *m_object;
    QRect m_old, m_new;
};


class GRotateCmd : public GCommand {

public:
    GRotateCmd(GObject *object, const QString &name);
    GRotateCmd(GObject *object, const QString &name, const QPoint &center,
               const double &angle);
    virtual ~GRotateCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setCenter(const QPoint &center) { m_center=center; }
    const QPoint &center() const { return m_center; }
    void setAngle(const double &angle) { m_angle=angle; }
    const double &angle() const { return m_angle; }

private:
    GObject *m_object;
    QPoint m_center;
    double m_angle;
};


class GScaleCmd : public GCommand {

public:
    GScaleCmd(GObject *object, const QString &name);
    GScaleCmd(GObject *object, const QString &name, const QPoint &origin,
              const double &xfactor, const double &yfactor);
    virtual ~GScaleCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setOrigin(const QPoint &origin) { m_origin=origin; }
    const QPoint &origin() const { return m_origin; }
    void setXFactor(const double &xfactor) { m_xfactor=xfactor; }
    const double &xfactor() const { return m_xfactor; }
    void setYFactor(const double &yfactor) { m_yfactor=yfactor; }
    const double &yfactor() const { return m_yfactor; }

private:
    GObject *m_object;
    QPoint m_origin;
    double m_xfactor, m_yfactor;
};


class GSetBrushCmd : public GCommand {

public:
    GSetBrushCmd(GObject *object, const QString &name);
    GSetBrushCmd(GObject *object, const QString &name, const QBrush &oldBrush,
                 const QBrush &newBrush);
    virtual ~GSetBrushCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setOldBrush(const QBrush &oldBrush) { m_old=oldBrush; }
    const QBrush &oldBrush() const { return m_old; }
    void setNewBrush(const QBrush &newBrush) { m_new=newBrush; }
    const QBrush &newBrush() const { return m_new; }

private:
    GObject *m_object;
    QBrush m_old, m_new;
};


class GSetPenCmd : public GCommand {

public:
    GSetPenCmd(GObject *object, const QString &name);
    GSetPenCmd(GObject *object, const QString &name, const QPen &oldPen,
               const QPen &newPen);
    virtual ~GSetPenCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setOldPen(const QPen &oldPen) { m_old=oldPen; }
    const QPen &oldPen() const { return m_old; }
    void setNewPen(const QPen &newPen) { m_new=newPen; }
    const QPen &newPen() const { return m_new; }

private:
    GObject *m_object;
    QPen m_old, m_new;
};


class GSetNameCmd : public GCommand {

public:
    GSetNameCmd(GObject *object, const QString &name);
    GSetNameCmd(GObject *object, const QString &name, const QString &oldName,
                const QString &newName);
    virtual ~GSetNameCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setOldName(const QString &oldName) { m_old=oldName; }
    QString oldName() const { return m_old; }
    void setNewName(const QString &newName) { m_new=newName; }
    QString newName() const { return m_new; }

private:
    GObject *m_object;
    QString m_old, m_new;
};


class GSetFillStyleCmd : public GCommand {

public:
    GSetFillStyleCmd(GObject *object, const QString &name);
    GSetFillStyleCmd(GObject *object, const QString &name,
                     const GObject::FillStyle &oldFillStyle,
                     const GObject::FillStyle &newFillStyle);
    virtual ~GSetFillStyleCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setOldFillStyle(const GObject::FillStyle &oldFillStyle) { m_old=oldFillStyle; }
    const GObject::FillStyle &oldFillStyle() const { return m_old; }
    void setNewFillStyle(const GObject::FillStyle &newFillStyle) { m_new=newFillStyle; }
    const GObject::FillStyle &newFillStyle() const { return m_new; }

private:
    GObject *m_object;
    GObject::FillStyle m_old, m_new;
};


class GSetGradientCmd : public GCommand {

public:
    GSetGradientCmd(GObject *object, const QString &name);
    GSetGradientCmd(GObject *object, const QString &name,
                    const Gradient &oldGradient, const Gradient &newGradient);
    virtual ~GSetGradientCmd() {}

    virtual void execute();
    virtual void unexecute();

    void setOldGradient(const Gradient &oldGradient) { m_old=oldGradient; }
    const Gradient &oldGradient() const { return m_old; }
    void setNewGradient(const Gradient &newGradient) { m_new=newGradient; }
    const Gradient &newGradient() const { return m_new; }

private:
    GObject *m_object;
    Gradient m_old, m_new;
};
#endif // gprimitivecmds_h
