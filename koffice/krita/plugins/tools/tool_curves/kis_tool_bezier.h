/*
 *  kis_tool_bezier.h -- part of Krita
 *
 *  Copyright (c) 2006 Emanuele Tamponi <emanuele@valinor.it>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KIS_TOOL_BEZIER_H_
#define KIS_TOOL_BEZIER_H_

#include "kis_tool_factory.h"
#include "kis_tool_curve_paint.h"
#include "kis_point.h"

class CurvePoint;
class KisPoint;
class KisCanvas;
class KisCurve;
class KisPainter;
class KisPoint;

const int BEZIERENDHINT = 0x0010;
const int BEZIERPREVCONTROLHINT = 0x0020;
const int BEZIERNEXTCONTROLHINT = 0x0040;

const int SYMMETRICALCONTROLSOPTION = ALTOPTION;
const int PREFERCONTROLSOPTION = SHIFTOPTION;

class KisCurveBezier : public KisCurve {

    typedef KisCurve super;

    void recursiveCurve (const KisPoint& P1, const KisPoint& P2, const KisPoint& P3,
                         const KisPoint& P4, int level, iterator it);
    KisPoint midpoint (const KisPoint&, const KisPoint&);

    int m_maxLevel;
    
public:

    KisCurveBezier() : super() {m_maxLevel = 5;}

    ~KisCurveBezier() {}

    virtual void calculateCurve(iterator, iterator, iterator);
    virtual iterator pushPivot(const KisPoint&);
    virtual iterator movePivot(iterator, const KisPoint&);
    virtual void deletePivot(iterator);

public:

    iterator groupEndpoint (iterator) const;
    iterator groupPrevControl (iterator) const;
    iterator groupNextControl (iterator) const;

    bool groupSelected (iterator) const;

    iterator nextGroupEndpoint (iterator) const;
    iterator prevGroupEndpoint (iterator) const;

};

class KisToolBezier : public KisToolCurvePaint {

    typedef KisToolCurvePaint super;
    Q_OBJECT

public:
    KisToolBezier();
    virtual ~KisToolBezier();

    virtual void setup(KActionCollection *collection);
    virtual enumToolType toolType() { return TOOL_SHAPE; }

protected:

    virtual KisCurve::iterator selectByHandle(const QPoint& pos);

    virtual KisCurve::iterator paintPoint(KisPainter& painter, KisCurve::iterator point);
    virtual void drawPivotHandle(KisCanvasPainter& gc, KisCurve::iterator point);
    virtual KisCurve::iterator drawPoint(KisCanvasPainter& gc, KisCurve::iterator point);

protected:

    KisCurveBezier *m_derivated;

};

class KisToolBezierFactory : public KisToolFactory {
    typedef KisToolFactory super;
public:
    KisToolBezierFactory() : super() {};
    virtual ~KisToolBezierFactory(){};

    virtual KisTool * createTool(KActionCollection * ac) {
        KisTool * t =  new KisToolBezier();
        Q_CHECK_PTR(t);
        t->setup(ac);
        return t;
    }
    virtual KisID id() { return KisID("beziershape", i18n("Bezier Tool")); }
};


#endif //__KIS_TOOL_BEZIER_H__
