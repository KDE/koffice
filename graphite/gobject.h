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

#ifndef gobject_h
#define gobject_h

#include <qlist.h>
#include <qbrush.h>
#include <qpen.h>
#include <qrect.h>
#include <qpoint.h>
#include <qwidget.h>

#include <kdialogbase.h>

#include <math.h>

#include <graphiteglobal.h>

class QDomElement;
class QDomDocument;
class QPoint;
class QRect;
class QPainter;
class QMouseEvent;
class QKeyEvent;
class QResizeEvent;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QVButtonGroup;
class QWidgetStack;
class QCheckBox;
class QSlider;
class QSizePolicy;

class KDialogBase;
class KColorButton;

class GObject;
class GraphiteView;
class GraphitePart;
class PWidget;


// This is the manipulator class for GObject. Manipulators (M9r's)
// are used to handle the creation, selection, movement, rotation,...
// of objects. They also provide a property dialog (lazy creation)
// The pure virtual GObject::createM9r() factory method ensures that
// the correct manipulator is created :) (factory method pattern)
// The M9r is used every time a user wants to create or change an object
// interactively.
// First the object is "hit" - then a M9r is created and this M9r is used as
// a kind of EventFilter. Every Event is forwarded to the M9r. If the M9r
// decides to handle the event, it returns true afterwards. If the Event
// remains unhandled, the M9r returns false and the Event has to be processed
// by the calling method.
// Note: The M9r is bound to a specific view and it won't work (correctly)
// if you use one M9r for more than one view.
// Whenever a repaint is needed (movement,...), the dirty rect has to be
// set (i.e. something different to (0, 0, 0, 0)).
// Some of the M9rs can be in two different "modes": Create and Manipulate
// General rule: simple M9rs support Create, complex ones do not :)
class GObjectM9r : public KDialogBase {

    Q_OBJECT
public:
    enum Mode { Create, Manipulate };

    virtual ~GObjectM9r();

    const Mode &mode() const { return m_mode; }
    void setMode(const Mode &mode) { m_mode=mode; }

    const GraphiteView *view() const { return m_view; }

    // calls drawHandles
    virtual void draw(QPainter &p) = 0;

    // return false when you couldn't handle the event
    virtual const bool mouseMoveEvent(QMouseEvent */*e*/, QRect &/*dirty*/) { return false; }
    virtual const bool mousePressEvent(QMouseEvent */*e*/, QRect &/*dirty*/) { return false; }
    virtual const bool mouseReleaseEvent(QMouseEvent */*e*/, QRect &/*dirty*/) { return false; }
    virtual const bool mouseDoubleClickEvent(QMouseEvent */*e*/, QRect &/*dirty*/) { return false; }

    virtual const bool keyPressEvent(QKeyEvent */*e*/, QRect &/*dirty*/) { return false; }
    virtual const bool keyReleaseEvent(QKeyEvent */*e*/, QRect &/*dirty*/) { return false; }

    virtual GObject *gobject() = 0;

protected slots:
    // All these slots just tell us that something has been changed
    // Yes, I know that this is an ugly hack :(
    // It was necessary to either have this or to store the temporary
    // values (because of the Ok/Apply/Cancel stuff).
    virtual void slotChanged(const QString &);
    virtual void slotChanged(int);
    virtual void slotChanged(const QColor &);

    virtual void slotOk();
    virtual void slotApply();
    virtual void slotCancel();

protected:
    GObjectM9r(GObject *object, const Mode &mode, GraphitePart *part,
               GraphiteView *view, const QString &type);

    // This menthod returns a property dialog for the object. It
    // creates a dialog (i.e. adds a few pages to *this). The
    // dialog is cached for further use and destroyed on destrucion
    // of this object.
    // If you decide to override this method make sure that the first
    // thing you do in your implementation is checking whether the dialog
    // has been created already. Then call the method of your parent.
    // Then add your pages to the dialog and initialize the contents.
    // Note: This dialog is modal and it has an "Apply" button. The
    // user is able to change the properties and see the result after
    // clicking 'Apply'.
    virtual void createPropertyDialog();

    GObject *m_object;
    Mode m_mode;
    bool first_call; // Whether this is the first call for this M9r (no hit test!)
    GraphitePart *m_part;     // we need that for the history
    QList<QRect> *m_handles;  // contains all the handle rects
    bool m_pressed;           // mouse button pressed?
    bool m_changed;           // true, if the Apply button is "active"
    bool m_created;           // dia created?

private:
    QString m_type;         // Type of object (e.g. "Line", "Rectangle")
    QLineEdit *m_line;      // line ed. for the name field
    GraphiteView *m_view;   // "our" parent view
};


// This class adds a "Pen" dialog page to the empty dialog
class G1DObjectM9r : public GObjectM9r {

    Q_OBJECT
public:
    virtual ~G1DObjectM9r() {}

protected slots:
    virtual void slotApply();

protected:
    G1DObjectM9r(GObject *object, const Mode &mode, GraphitePart *part,
                 GraphiteView *view, const QString &type) :
        GObjectM9r(object, mode, part, view, type) {}
    virtual void createPropertyDialog();

private:
    QSpinBox *m_width;
    KColorButton *m_color;
    QComboBox *m_style;
};


// This class adds a fill style (none/brush/gradient) page to the dialog
class G2DObjectM9r : public G1DObjectM9r {

    Q_OBJECT
public:
    virtual ~G2DObjectM9r() {}

protected slots:
    virtual void slotChanged(int x);
    virtual void slotChanged(const QColor &x);

    virtual void slotApply();
    virtual void resizeEvent(QResizeEvent *e);  // update the preview on resize

protected:
    G2DObjectM9r(GObject *object, const Mode &mode, GraphitePart *part,
                 GraphiteView *view, const QString &type) :
        G1DObjectM9r(object, mode, part, view, type) {}
    virtual void createPropertyDialog();

private slots:
    void slotBalance();   // activate/deactivate the sliders (xfactor/yfactor)

private:
    void updatePage();
    void updatePreview(int btn);

    QVButtonGroup *m_style;
    PWidget *m_preview;
    QWidgetStack *m_stack;
    KColorButton *m_brushColor;
    QComboBox *m_brushStyle;
    KColorButton *m_gradientCA, *m_gradientCB;
    QComboBox *m_gradientStyle;
    QCheckBox *m_unbalanced;
    QSlider *m_xfactor, *m_yfactor;
};


// The abstract base classes for all graphic objects. This class is
// implemented as a composite (pattern) - sort of :)
// There are complex classes (classes which are composed of many
// objects, like a group) and leaf classes which don't have any
// children (e.g. line, rect,...).
// The resulting tree represents the Z-Order of the document. (First
// the object draws "itself" and then its children)
class GObject {

public:
    enum State { Visible, Handles, Rot_Handles, Invisible, Deleted }; // all possible states
    enum FillStyle { Brush, GradientFilled };  // all possible fill styles
    enum Position { First, Last, Current }; // where to insert the new child object

    virtual ~GObject() {}

    virtual const bool isOk() const { return m_ok; }
    virtual void setOk(const bool &ok=true) { m_ok=ok; }

    virtual GObject *clone() const = 0;           // exact copy of "this" (calls the Copy-CTOR)
    // create an object and initialize it with the given XML (calls the XML-CTOR)
    virtual GObject *instantiate(const QDomElement &element) const = 0;

    const GObject *parent() const { return m_parent; }
    void setParent(GObject *parent) const;   // parent==0L - no parent, parent==this - illegal

    // These two methods are only implemented for "complex" objetcs!
    // The child is inserted at GObject::Position
    virtual const bool plugChild(GObject */*child*/, const Position &/*pos*/=Current) { return false; }
    virtual const bool unplugChild(GObject */*child*/) { return false; }

    // These methods are used to access the object's children
    // Implemented via QListIterator - Leaf classes don't override
    // that default behavior...
    virtual const GObject *firstChild() const { return 0L; }
    virtual const GObject *nextChild() const { return 0L; }
    virtual const GObject *lastChild() const { return 0L; }
    virtual const GObject *prevChild() const { return 0L; }
    virtual const GObject *current() const { return 0L; }

    virtual QDomElement save(QDomDocument &doc) const; // save the object (and all its children) to xml

    // toPrinter is set when we print the document - this means we don't
    // have to paint "invisible" (normally they are colored gray) objects
    // Whenever you paint an object, check if it is within the requested
    // area (region). Add the area of "your" object (boundingRect) to the
    // region to maintain a correct Z order!
    virtual void draw(QPainter &p, QRegion &reg, const bool toPrinter=false) = 0;
    // Used to draw the handles/rot-handles when selected
    // All handles which are drawn are added to the list if the list
    // is != 0L. Use this list to check "mouseOver" stuff
    virtual void drawHandles(QPainter &p, QList<QRect> *handles=0L);

    // This one is called by the document whenever the zoom of the view changes
    virtual void recalculate() = 0;  // don't forget to call it for all coords!

    // does the object contain this point? (Note: finds the most nested child which is hit!)
    virtual const GObject *hit(const QPoint &p) const = 0;
    virtual const bool intersects(const QRect &r) const = 0;  // does the object intersect the rectangle?
    virtual const QRect &boundingRect() const = 0;            // the bounding rectangle of this object

    virtual GObjectM9r *createM9r(GraphitePart *part, GraphiteView *view,
                                  const GObjectM9r::Mode &mode=GObjectM9r::Manipulate) = 0;

    QString name() const { return m_name; }       // name of the object (e.g. "Line001")
    void setName(const QString &name) { m_name=name; }   // set the name

    virtual const QPoint origin() const = 0;             // the origin coordinate of the obj
    virtual void setOrigin(const QPoint &origin) = 0;
    virtual void moveX(const int &dx) = 0;
    virtual void moveY(const int &dy) = 0;
    virtual void move(const int &dx, const int &dy) = 0;

    // Note: radians!
    virtual void rotate(const QPoint &center, const double &angle) = 0;
    virtual const double &angle() const { return m_angle; }

    virtual void scale(const QPoint &origin, const double &xfactor, const double &yfactor) = 0;
    virtual void resize(const QRect &boundingRect) = 0;  // resize, that it fits in this rect

    const State &state() const { return m_state; }              // what's the current state?
    virtual void setState(const State state) { m_state=state; } // set the state

    const FillStyle &fillStyle() const { return m_fillStyle; }
    virtual void setFillStyle(const FillStyle &fillStyle) { m_fillStyle=fillStyle; }
    const QBrush &brush() const { return m_brush; }         // Fill style (brush)
    virtual void setBrush(const QBrush &brush) { m_brush=brush; }
    const Gradient &gradient() const { return m_gradient; } // Gradient filled
    virtual void setGradient(const Gradient &gradient) { m_gradient=gradient; }
    const QPen &pen() const { return m_pen; }               // Pen for the lines
    virtual void setPen(const QPen &pen) { m_pen=pen; }

protected:
    GObject(const QString &name=QString::null);
    GObject(const GObject &rhs);
    GObject(const QDomElement &element);

    // rotatePoint rotates a given point. The "point" (x, y, or QPoint) passed as
    // function argument is changed! (we're using radians!)
    void rotatePoint(int &x, int &y, const double &angle, const QPoint &center) const;
    void rotatePoint(unsigned int &x, unsigned int &y, const double &angle, const QPoint &center) const;
    void rotatePoint(double &x, double &y, const double &angle, const QPoint &center) const;
    void rotatePoint(QPoint &p, const double &angle, const QPoint &center) const;

    // scalePoint scales a given point. The "point" (x, y, or QPoint) passed as
    // function argument is changed!
    void scalePoint(int &x, int &y, const double &xfactor, const double &yfactor,
                    const QPoint &center) const;
    void scalePoint(unsigned int &x, unsigned int &y, const double &xfactor,
                    const double &yfactor, const QPoint &center) const;
    void scalePoint(double &x, double &y, const double &xfactor, const double &yfactor,
                    const QPoint &center) const;
    void scalePoint(QPoint &p, const double &xfactor, const double &yfactor,
                    const QPoint &center) const;

    QString m_name;                    // name of the object
    State m_state;                     // are there handles to draw or not?
    mutable GObject *m_parent;
    mutable double m_angle;            // angle (radians!)

    mutable bool m_boundingRectDirty;  // is the cached bounding rect still correct?
    mutable QRect m_boundingRect;      // bounding rect (cache) - don't use directly!

    FillStyle m_fillStyle;
    QBrush m_brush;
    Gradient m_gradient;
    QPen m_pen;

    bool m_ok;      // used to express errors (e.g. during loading)

private:
    GObject &operator=(const GObject &rhs); // don't assign the objects, clone them
};


// Some helper functions
namespace Graphite {

inline const int double2Int(const double &value) {

    if( static_cast<double>((value-static_cast<int>(value)))>=0.5 )
        return static_cast<int>(value)+1;
    else if( static_cast<double>((value-static_cast<int>(value)))<=-0.5 )
        return static_cast<int>(value)-1;
    else
        return static_cast<int>(value);
}

inline const double rad2deg(const double &rad) {
    return rad*180.0*M_1_PI;   // M_1_PI = 1/M_PI :)
}

inline const double deg2rad(const double &deg) {
    return deg*M_PI/180.0;
}

inline const double normalizeRad(const double &rad) {

    double nRad=rad;
    while(nRad>2*M_PI)
        nRad-=2*M_PI;
    while(nRad<0)
        nRad+=2*M_PI;
    return nRad;
}

inline const double normalizeDeg(const double &deg) {

    double nDeg=deg;
    while(nDeg>360)
        nDeg-=360;
    while(nDeg<0)
        nDeg+=360;
    return nDeg;
}

}; // namespace Graphite

inline void GObject::rotatePoint(int &x, int &y, const double &angle, const QPoint &center) const {

    double alpha=angle/2.0;
    double dx=static_cast<double>(x-center.x());
    double dy=static_cast<double>(y-center.y());
    double r=std::sqrt(dx*dx+dy*dy);
    double s=QABS(2*r*std::sin(alpha));
    double gamma=std::acos( QABS(dy)/r );
    double beta;

    if(dx>=0 && dy>=0)
        beta=-gamma-alpha;
    else if(dx<0 && dy>=0)
        beta=-(M_PI-gamma-QABS(alpha));
    else if(dx<0 && dy<0)
        beta=M_PI-gamma+M_PI-alpha;
    else // dx>=0 && dy<0
        beta=QABS(gamma)+QABS(alpha);

    y+=Graphite::double2Int(s*std::sin(beta));
    x+=Graphite::double2Int(s*std::cos(beta));
}

inline void GObject::rotatePoint(unsigned int &x, unsigned int &y, const double &angle, const QPoint &center) const {

    // This awkward stuff with the tmp variables is a workaround for
    // "old" compilers (egcs-1.1.2 :)
    int _x=static_cast<int>(x);
    int _y=static_cast<int>(y);
    rotatePoint(_x, _y, angle, center);
    x=static_cast<unsigned int>(_x);
    y=static_cast<unsigned int>(_y);
}

inline void GObject::rotatePoint(double &x, double &y, const double &angle, const QPoint &center) const {

    double alpha=angle/2.0;
    double dx=x-static_cast<double>(center.x());
    double dy=y-static_cast<double>(center.y());
    double r=std::sqrt(dx*dx+dy*dy);
    double s=QABS(2*r*std::sin(alpha));
    double gamma=std::acos( QABS(dy)/r );
    double beta;

    if(dx>=0 && dy>=0)
        beta=-gamma-alpha;
    else if(dx<0 && dy>=0)
        beta=-(M_PI-gamma-QABS(alpha));
    else if(dx<0 && dy<0)
        beta=M_PI-gamma+M_PI-alpha;
    else // dx>=0 && dy<0
        beta=QABS(gamma)+QABS(alpha);

    y+=s*std::sin(beta);
    x+=s*std::cos(beta);
}

inline void GObject::rotatePoint(QPoint &p, const double &angle, const QPoint &center) const {
    rotatePoint(p.rx(), p.ry(), angle, center);
}

inline void GObject::scalePoint(int &x, int &y, const double &xfactor, const double &yfactor,
                         const QPoint &center) const {
    if(xfactor<=0 || yfactor<=0)
        return;
    x=Graphite::double2Int( static_cast<double>(center.x()) + static_cast<double>(x-center.x())*xfactor );
    y=Graphite::double2Int( static_cast<double>(center.y()) + static_cast<double>(y-center.y())*yfactor );
}

inline void GObject::scalePoint(unsigned int &x, unsigned int &y, const double &xfactor,
                         const double &yfactor, const QPoint &center) const {
    // This awkward stuff with the tmp variables is a workaround for
    // "old" compilers (egcs-1.1.2 :)
    int _x=static_cast<int>(x);
    int _y=static_cast<int>(y);
    scalePoint(_x, _y, xfactor, yfactor, center);
    x=static_cast<unsigned int>(_x);
    y=static_cast<unsigned int>(_y);
}

inline void GObject::scalePoint(double &x, double &y, const double &xfactor, const double &yfactor,
                         const QPoint &center) const {
    if(xfactor<=0 || yfactor<=0)
        return;
    x=static_cast<double>(center.x()) + static_cast<double>(x-center.x())*xfactor;
    y=static_cast<double>(center.y()) + static_cast<double>(y-center.y())*yfactor;
}

inline void GObject::scalePoint(QPoint &p, const double &xfactor, const double &yfactor,
                         const QPoint &center) const {
    scalePoint(p.rx(), p.ry(), xfactor, yfactor, center);
}


// This *huge* class is needed to present the preview pixmap.
// It is simply a plain Widget which tries to get all the free
// space it can get (in x and y direction).
class PWidget : public QWidget {

public:
    PWidget(QWidget *w) : QWidget(w) {}
    virtual QSizePolicy sizePolicy() const;
};
#endif
