/***************************************************************************
                          canvbox.cpp  -  description
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
#include <klocale.h>

#include <qcanvas.h>
#include <qapplication.h>
#include <qstringlist.h>
#include <qstring.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include "canvbox.h"
#include "mycanvas.h"
#include "creportitem.h"
#include "property.h"

//CanvasBox class

CanvasBox::~CanvasBox()
{
}


void CanvasBox::scale(int scale)
{
    setSize(width()*scale, height()*scale);
}

void CanvasBox::draw(QPainter &painter)
{
    QCanvasRectangle::draw(painter);
}


//CanvasSection class

void CanvasSection::draw(QPainter &painter)
{
    CanvasBox::draw(painter);
}


//CanvasKugarTemplate class

CanvasKugarTemplate::CanvasKugarTemplate(int x, int y, int width, int height, QCanvas * canvas):
    CanvasSection(x, y, width, height, canvas)
{
    detailsCount = 0;

    setZ(1);
    ((MyCanvas *)canvas)->templ = this;
    reportHeader = 0;
    reportFooter = 0;
    pageHeader = 0;
    pageFooter = 0;

    std::map<QString, QString> m;
    m["A4"] = "0";
    m["B5"] = "1";
    m["Letter"] = "2";
    m["Legal"] = "3";
    m["Executive"] = "4";
    m["A0"] = "5";
    m["A1"] = "6";
    m["A2"] = "7";
    m["A3"] = "8";
    m["A5"] = "9";
    m["A6"] = "10";
    m["A7"] = "11";
    m["A8"] = "12";
    m["A9"] = "13";
    m["B0"] = "14";
    m["B1"] = "15";
    m["B10"] = "16";
    m["B2"] = "17";
    m["B3"] = "18";
    m["B4"] = "19";
    m["B6"] = "20";
    m["B7"] = "21";
    m["B8"] = "22";
    m["B9"] = "23";
    m["C5E"] = "24";
    m["Comm10E"] = "25";
    m["DLE"] = "26";
    m["Folio"] = "27";
    m["Ledger"] = "28";
    m["Tabloid"] = "29";
    m["NPageSize"] = "30";
    props["PageSize"] = *(new PropPtr(new DescriptionProperty("PageSize", m, i18n("Page size"), "0")));
    m.clear();

    m["Portrait"] = "0";
    m["Landscape"] = "1";
    props["PageOrientation"] = *(new PropPtr(new DescriptionProperty("PageOrientation",
        m, i18n("Page orientation"), "0")));
    m.clear();

    props["TopMargin"] = *(new PropPtr(new Property(IntegerValue, "TopMargin", i18n("Top margin"), "0")));

    props["BottomMargin"] = *(new PropPtr(new Property(IntegerValue, "BottomMargin", i18n("Bottom margin"), "0")));

    props["LeftMargin"] = *(new PropPtr(new Property(IntegerValue, "LeftMargin", i18n("Left margin"), "0")));

    props["RightMargin"] = *(new PropPtr(new Property(IntegerValue, "RightMargin", i18n("Right margin"), "0")));
}

CanvasKugarTemplate::~CanvasKugarTemplate()
{
    if (reportHeader)
        delete reportHeader;
    if (pageHeader)
        delete pageHeader;

    std::map<int, DetailBand>::const_iterator it;
    for (it = details.begin(); it != details.end(); ++it)
    {
        if (it->second.first.first)
            delete it->second.first.first;
        if (it->second.second)
            delete it->second.second;
        if (it->second.first.second)
            delete it->second.first.second;
    }
    if (pageFooter)
        delete pageFooter;
    if (reportFooter)
        delete reportFooter;
}

void CanvasKugarTemplate::draw(QPainter &painter)
{
    painter.setPen(QPen(QColor(0, 0, 0), 0, Qt::DashLine));
    QPoint p1((int)(x()+props["LeftMargin"]->value().toInt()),
        (int)(y()+props["TopMargin"]->value().toInt()));
    QPoint p2((int)(x()+props["LeftMargin"]->value().toInt()),
        (int)y() + height() - props["BottomMargin"]->value().toInt());
    QPoint p3((int)x() + width() - props["RightMargin"]->value().toInt(),
        (int)y() + height() - props["BottomMargin"]->value().toInt());
    QPoint p4((int)x() + width() - props["RightMargin"]->value().toInt(),
        (int)(y()+props["TopMargin"]->value().toInt()));
    painter.moveTo(p1);
    painter.lineTo(p2);
    painter.lineTo(p3);
    painter.lineTo(p4);
    painter.lineTo(p1);
/*    painter.drawRect((int)(x()+props["LeftMargin"].first.toInt()),
        (int)(y()+props["TopMargin"].first.toInt()),
        width() - props["LeftMargin"].first.toInt() - props["RightMargin"].first.toInt(),
        height() - props["TopMargin"].first.toInt() - props["BottomMargin"].first.toInt());*/
    CanvasSection::draw(painter);
}

void CanvasKugarTemplate::updatePaperProps()
{
    QPrinter* printer;

    // Set the page size
    printer = new QPrinter();
    printer->setFullPage(true);
    printer->setPageSize((QPrinter::PageSize)props["PageSize"]->value().toInt());
    printer->setOrientation((QPrinter::Orientation)props["PageOrientation"]->value().toInt());

    // Get the page metrics and set appropriate wigth and height
    QPaintDeviceMetrics pdm(printer);
    canvas()->resize(pdm.width(), pdm.height());
    setSize(pdm.width(), pdm.height());

    //this is not needed anymore
    delete printer;
}

/*arrange sections on page automatically*/
void CanvasKugarTemplate::arrangeSections(bool destructive)
{
    int base = props["TopMargin"]->value().toInt();
    if (reportHeader)
    {
        reportHeader->arrange(base, destructive);
        base += reportHeader->props["Height"]->value().toInt();
        reportHeader->show();
    }
    if (pageHeader)
    {
        pageHeader->arrange(base, destructive);
        base += pageHeader->props["Height"]->value().toInt();
        pageHeader->show();
    }

    std::map<int, DetailBand>::const_iterator it;
    for (it = details.begin(); it != details.end(); ++it)
    {
        //arranging detail header
        if (it->second.first.first)
        {
            it->second.first.first->arrange(base, destructive);
            base += it->second.first.first->props["Height"]->value().toInt();
            it->second.first.first->show();
        }
        //arranging detail
        if (it->second.second)
        {
            it->second.second->arrange(base, destructive);
            base += it->second.second->props["Height"]->value().toInt();
            it->second.second->show();
        }
    }
    std::map<int, DetailBand>::reverse_iterator itr;
    for (itr = details.rbegin(); itr != details.rend(); ++itr)
    {
        //arranging detail footer
        if (itr->second.first.second)
        {
            itr->second.first.second->arrange(base, destructive);
            base += itr->second.first.second->props["Height"]->value().toInt();
            itr->second.first.second->show();
        }
    }

    if (pageFooter)
    {
        pageFooter->arrange(base, destructive);
        base += pageFooter->props["Height"]->value().toInt();
        pageFooter->show();
    }
    if (reportFooter)
    {
        reportFooter->arrange(base, destructive);
        base += reportFooter->props["Height"]->value().toInt();
        reportFooter->show();
    }
}

QString CanvasKugarTemplate::getXml()
{
    QString result = "";
    result += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n";
    result += "<!DOCTYPE KugarTemplate SYSTEM \"kugartemplate.dtd\">\n\n";
    result += "<KugarTemplate";
    for (std::map<QString, PropPtr >::const_iterator it = props.begin();
        it != props.end(); it++ )
    {
        result += " " + it->first + "=" + "\"" + it->second->value() + "\"";
    }
    result += ">\n";

    if (reportHeader)
        result += reportHeader->getXml();
    if (pageHeader)
        result += pageHeader->getXml();

    std::map<int, DetailBand>::const_iterator it;
    for (it = details.begin(); it != details.end(); ++it)
    {
        //getting xml from detail header
        if (it->second.first.first)
            result += it->second.first.first->getXml();
        //getting xml from detail
        if (it->second.second)
            result += it->second.second->getXml();
        //getting xml from detail footer
        if (it->second.first.second)
            result += it->second.first.second->getXml();
    }
    if (pageFooter)
        result += pageFooter->getXml();
    if (reportFooter)
        result += reportFooter->getXml();

    result += "</KugarTemplate>\n";

    return result;
}

void CanvasKugarTemplate::removeSection(CanvasBand *section,
    CanvasDetailHeader **header, CanvasDetailFooter **footer)
{
    *header = 0;
    *footer = 0;
    if (section == reportHeader)
        reportHeader = 0;
    if (section == reportFooter)
        reportFooter = 0;
    if (section == pageHeader)
        pageHeader = 0;
    if (section == pageFooter)
        pageFooter = 0;
    for (std::map<int, DetailBand>::iterator i = details.begin(); i != details.end(); i++)
    {
        if (i->second.second == section)
        {
            //delete not only detail but it's header and footer
            i->second.second = 0;
            *header = i->second.first.first;
            i->second.first.first = 0;
            *footer = i->second.first.second;
            i->second.first.second = 0;
            detailsCount--;
        }
        if (i->second.first.first == section)
            i->second.first.first = 0;
        if (i->second.first.second == section)
            i->second.first.second = 0;
    }
}


//CanvasBand class

void CanvasBand::draw(QPainter &painter)
{
    setX(((MyCanvas*)canvas())->templ->props["LeftMargin"]->value().toInt());
    setSize(((MyCanvas*)canvas())->templ->width()
        - ((MyCanvas*)canvas())->templ->props["RightMargin"]->value().toInt()
        - ((MyCanvas*)canvas())->templ->props["LeftMargin"]->value().toInt(),
        props["Height"]->value().toInt());
    CanvasSection::draw(painter);
}

//arrange band and all sublings (items)
void CanvasBand::arrange(int base, bool destructive)
{
    int diff = base - (int)y();
    setY(base);
    if (!destructive)
        return;
    for (QCanvasItemList::Iterator it=items.begin(); it!=items.end(); ++it)
    {
        (*it)->moveBy(0, diff);
    //  ( (CanvasReportItem *)(*it) )->updateGeomProps();
        canvas()->update();
        (*it)->hide();
        (*it)->show();
    }
}

QString CanvasBand::getXml()
{
    QString result = "";
    std::map<QString, PropPtr >::const_iterator it;
    for (it = props.begin(); it != props.end(); ++it)
    {
        result += " " + it->first + "=" + "\"" + it->second->value() + "\"";
    }
    result += ">\n";
    for (QCanvasItemList::Iterator it=items.begin(); it!=items.end(); ++it)
    {
        result += ((CanvasReportItem *)(*it))->getXml();
    }
    return result;
}

CanvasBand::~CanvasBand()
{
    for (QCanvasItemList::Iterator it = items.begin(); it != items.end(); ++it)
    {
//  (*it)->hide();
        delete (*it);
    }
    items.clear();
}

//CanvasReportHeader class

CanvasReportHeader::CanvasReportHeader(int x, int y, int width, int height, QCanvas * canvas):
    CanvasBand(x, y, width, height, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Report header's height"), "50")));
};

void CanvasReportHeader::draw(QPainter &painter)
{
    painter.drawText(rect(), AlignVCenter | AlignLeft,
        i18n("Report Header"));
    CanvasBand::draw(painter);
}

QString CanvasReportHeader::getXml()
{
    return "\t<ReportHeader PrintFrequency=\"0\"" +
        CanvasBand::getXml() + "\t</ReportHeader>\n\n";
}


//CanvasReportFooter class

CanvasReportFooter::CanvasReportFooter(int x, int y, int width, int height, QCanvas * canvas):
    CanvasBand(x, y, width, height, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Report footer's height"), "50")));
}

void CanvasReportFooter::draw(QPainter &painter)
{
    painter.drawText(rect(), AlignVCenter | AlignLeft,
        i18n("Report Footer"));
    CanvasBand::draw(painter);
}

QString CanvasReportFooter::getXml()
{
    return "\t<ReportFooter PrintFrequency=\"0\"" + CanvasBand::getXml() + "\t</ReportFooter>\n\n";
}


//CanvasPageHeader class

CanvasPageHeader::CanvasPageHeader(int x, int y, int width, int height, QCanvas * canvas):
    CanvasBand(x, y, width, height, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Page header's height"), "50")));
}

void CanvasPageHeader::draw(QPainter &painter)
{
    painter.drawText(rect(), AlignVCenter | AlignLeft,
        i18n("Page Header"));
    CanvasBand::draw(painter);
}

QString CanvasPageHeader::getXml()
{
    return "\t<PageHeader PrintFrequency=\"1\"" + CanvasBand::getXml() + "\t</PageHeader>\n\n";
}


//CanvasReportFooter class

CanvasPageFooter::CanvasPageFooter(int x, int y, int width, int height, QCanvas * canvas):
    CanvasBand(x, y, width, height, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Page footer's height"), "50")));
}

void CanvasPageFooter::draw(QPainter &painter)
{
    painter.drawText(rect(), AlignVCenter | AlignLeft,
        i18n("Page Footer"));
    CanvasBand::draw(painter);
}

QString CanvasPageFooter::getXml()
{
    return "\t<PageFooter PrintFrequency=\"1\"" + CanvasBand::getXml() + "\t</PageFooter>\n\n";
}


//CanvasDetailHeader class

CanvasDetailHeader::CanvasDetailHeader(int x, int y, int width, int height, QCanvas * canvas):
    CanvasBand(x, y, width, height, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Detail header's height"), "50")));

    props["Level"] = *(new PropPtr(new Property(IntegerValue, "Level", i18n("Detail header's level"), "0")));
}

void CanvasDetailHeader::draw(QPainter &painter)
{
    QString str = QString("%1 %2").arg(i18n("Detail header")).arg(props["Level"]->value().toInt());
    painter.drawText(rect(), AlignVCenter | AlignLeft, str);
    CanvasBand::draw(painter);
}

QString CanvasDetailHeader::getXml()
{
    return "\t<DetailHeader" + CanvasBand::getXml() + "\t</DetailHeader>\n\n";
}


//CanvasDetail class

CanvasDetail::CanvasDetail(int x, int y, int width, int height, QCanvas * canvas):
    CanvasBand(x, y, width, height, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Detail height"), "50")));
    props["Level"] = *(new PropPtr(new Property(IntegerValue, "Level", i18n("Detail level"), "0")));
}

void CanvasDetail::draw(QPainter &painter)
{
    QString str = QString("%1 %2").arg(i18n("Detail")).arg(props["Level"]->value().toInt());
    painter.drawText(rect(), AlignVCenter | AlignLeft, str);
    CanvasBand::draw(painter);
}

QString CanvasDetail::getXml()
{
    return "\t<Detail" + CanvasBand::getXml() + "\t</Detail>\n\n";
}


//CanvasDetailFooter class

CanvasDetailFooter::CanvasDetailFooter(int x, int y, int width, int height, QCanvas * canvas):
    CanvasBand(x, y, width, height, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Detail footer's height"), "50")));
    props["Level"] = *(new PropPtr(new Property(IntegerValue, "Level", i18n("Detail footer's level"), "0")));
}

void CanvasDetailFooter::draw(QPainter &painter)
{
    QString str = QString("%1 %2").arg(i18n("Detail footer")).arg(props["Level"]->value().toInt());
    painter.drawText(rect(), AlignVCenter | AlignLeft, str);
    CanvasBand::draw(painter);
}

QString CanvasDetailFooter::getXml()
{
    return "\t<DetailFooter" + CanvasBand::getXml() + "\t</DetailFooter>\n\n";
}
