/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Dymo <cloudtemple@mksat.net>

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
#include <map>

#ifndef PURE_QT
#include <klocale.h>
#else
#include "qlocale.h"
#endif

#include <qpainter.h>

#include "canvdetail.h"
#include "property.h"

CanvasDetail::CanvasDetail(int x, int y, int width, int height, int level, QCanvas * canvas):
    CanvasDetailBase(x, y, width, height, level, canvas)
{
    props["Height"] = *(new PropPtr(new Property(IntegerValue, "Height", i18n("Detail height"), "50")));
    props["Level"] = *(new PropPtr(new Property(IntegerValue, "Level", i18n("Detail level"), "0")));
    registerAs(KuDesignerRttiDetail);
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
