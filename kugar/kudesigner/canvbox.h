/***************************************************************************
                          canvbox.h  -  description
                             -------------------
    begin                : 07.06.2002
    copyright            : (C) 2002 by Alexander Dymo
    email                : cloudtemple@mksat.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/
#ifndef CANVBOX_H
#define CANVBOX_H

#include <map>
#include <qcanvas.h>
#include "canvdefs.h"
#include "property.h"

class QString;
class QStringList;
class CanvasDetailHeader;
class CanvasDetailFooter;
class CanvasDetail;
class CanvasReportHeader;
class CanvasReportFooter;
class CanvasPageHeader;
class CanvasPageFooter;

const int HolderSize = 6;

class CanvasBox: public QCanvasRectangle{
public:
    CanvasBox(int x, int y, int width, int height, QCanvas * canvas):
        QCanvasRectangle(x, y, width, height, canvas)
    {
        setSelected(false);
    }
    virtual ~CanvasBox();

    virtual int rtti() const { return KuDesignerRttiCanvasBox; }
    virtual void draw(QPainter &painter);
    void scale(int scale);
    virtual QString getXml() { return ""; }

    enum ResizeEnum {ResizeNothing=0,ResizeLeft=1,ResizeTop=2,ResizeRight=4,ResizeBottom=8};
    virtual int isInHolder(const QPoint ) {return ResizeNothing;}
    virtual void drawHolders(QPainter &) {}

    virtual void updateGeomProps(){;}

  
//    std::map<QString, std::pair<QString, QStringList> > props;
    /**
      NEW property format
      map<QString, PropPtr >
    */
    std::map<QString, PropPtr > props;
    protected:
    void registerAs(int type);
};

class CanvasSection: public CanvasBox{
public:
    CanvasSection(int x, int y, int width, int height, QCanvas * canvas):
	    CanvasBox(x, y, width, height, canvas) { }
    virtual QString getXml() { return ""; }
    virtual void draw(QPainter &painter);
    virtual int rtti() const { return KuDesignerRttiCanvasSection; }
};

class CanvasBand: public CanvasSection{
public:
    CanvasBand(int x, int y, int width, int height, QCanvas * canvas):
	    CanvasSection(x, y, width, height, canvas)
    {
	setZ(10);
    }
    ~CanvasBand();
    virtual void draw(QPainter &painter);
    virtual int rtti() const { return KuDesignerRttiCanvasBand; }
    virtual QString getXml();
    virtual int isInHolder(const QPoint );
    virtual void drawHolders(QPainter &);
    int minHeight();
    QRect bottomMiddleResizableRect();
    void arrange(int base, bool destructive = TRUE);
    virtual void updateGeomProps();

    QCanvasItemList items;
};

typedef std::pair< std::pair<CanvasDetailHeader*, CanvasDetailFooter*>, CanvasDetail*>  DetailBand;

class CanvasKugarTemplate: public CanvasSection{
public:
    CanvasKugarTemplate(int x, int y, int width, int height, QCanvas * canvas);
    ~CanvasKugarTemplate();
    virtual int rtti() const { return KuDesignerRttiKugarTemplate; }
    virtual void draw(QPainter &painter);
    void arrangeSections(bool destructive=TRUE);
    void updatePaperProps();

    QString fileName() const { return reportFileName; }
    void setFileName(const QString &fName) { reportFileName = fName; }

    virtual QString getXml();

    void removeSection(CanvasBand *section, CanvasDetailHeader **header,
		       CanvasDetailFooter **footer);

    CanvasReportHeader *reportHeader;
    CanvasReportFooter *reportFooter;
    CanvasPageHeader *pageHeader;
    CanvasPageFooter *pageFooter;
    std::map<int, DetailBand> details;
    unsigned int detailsCount;
private:
    QString reportFileName;
};

class CanvasReportHeader: public CanvasBand{
public:
    CanvasReportHeader(int x, int y, int width, int height, QCanvas * canvas);
    virtual int rtti() const { return KuDesignerRttiReportHeader; }
    virtual void draw(QPainter &painter);
    virtual QString getXml();
};

class CanvasReportFooter: public CanvasBand{
public:
    CanvasReportFooter(int x, int y, int width, int height, QCanvas * canvas);
    virtual int rtti() const { return KuDesignerRttiReportFooter; }
    virtual void draw(QPainter &painter);
    virtual QString getXml();
};

class CanvasPageHeader: public CanvasBand{
public:
    CanvasPageHeader(int x, int y, int width, int height, QCanvas * canvas);
    virtual void draw(QPainter &painter);
    virtual int rtti() const { return KuDesignerRttiPageHeader; }
    virtual QString getXml();
};

class CanvasPageFooter: public CanvasBand{
public:
    CanvasPageFooter(int x, int y, int width, int height, QCanvas * canvas);
    virtual int rtti() const { return KuDesignerRttiPageFooter; }
    virtual void draw(QPainter &painter);
    virtual QString getXml();
};

class CanvasDetailHeader: public CanvasBand{
public:
    CanvasDetailHeader(int x, int y, int width, int height, QCanvas * canvas);
    virtual int rtti() const { return KuDesignerRttiDetailHeader; }
    virtual void draw(QPainter &painter);
    virtual QString getXml();
};

class CanvasDetail: public CanvasBand{
public:
    CanvasDetail(int x, int y, int width, int height, QCanvas * canvas);
    virtual int rtti() const { return KuDesignerRttiDetail; }
    virtual void draw(QPainter &painter);
    virtual QString getXml();
};

class CanvasDetailFooter: public CanvasBand{
public:
    CanvasDetailFooter(int x, int y, int width, int height, QCanvas * canvas);
    virtual int rtti() const { return KuDesignerRttiDetailFooter; }
    virtual void draw(QPainter &painter);
    virtual QString getXml();
};

#endif
