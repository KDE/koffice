#ifndef CLINE_H
#define CLINE_H

#include "canvdefs.h"
#include "creportitem.h"

class CanvasLine: public CanvasReportItem{
public:
    CanvasLine(int x, int y, int width, int height, QCanvas * canvas);
    virtual int rtti() const { return RttiCanvasLine; }
    virtual QString getXml();
    
    virtual void updateGeomProps()
    {
	props["X"].first = QString("%1").arg((int)x());
	props["Y"].first = QString("%1").arg((int)y());
	props["X2"].first = QString("%1").arg((int)(x() + width()));
	props["Y2"].first = QString("%1").arg((int)(y() + height()));
    }
};

#endif
