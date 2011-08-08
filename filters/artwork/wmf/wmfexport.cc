/* This file is part of the KDE project
 * Copyright (c) 2003 thierry lorthiois (lorthioist@wanadoo.fr)
 * Copyright (c) 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "wmfexport.h"
#include <kowmfwrite.h>

#include <ArtworkDocument.h>
#include <ArtworkPart.h>

#include <kdebug.h>
#include <kpluginfactory.h>
#include <KoFilterChain.h>
#include <KLineBorder.h>
#include <KShape.h>
#include <KShapeContainer.h>
#include <KColorBackground.h>
#include <KGradientBackground.h>
#include <KPatternBackground.h>

/*
TODO: bs.wmf stroke in red with MSword and in brown with Kword ??
*/

K_PLUGIN_FACTORY(WmfExportFactory, registerPlugin<WmfExport>();)
K_EXPORT_PLUGIN(WmfExportFactory("kofficefilters"))


WmfExport::WmfExport(QObject*parent, const QVariantList&) :
        KoFilter(parent)
{
}

WmfExport::~WmfExport()
{
}

KoFilter::ConversionStatus WmfExport::convert(const QByteArray& from, const QByteArray& to)
{
    if (to != "image/x-wmf" || from != "application/vnd.oasis.opendocument.graphics")
        return KoFilter::NotImplemented;

    KoDocument * doc = m_chain->inputDocument();
    if (! doc)
        return KoFilter::ParsingError;

    ArtworkPart * artworkPart = dynamic_cast<ArtworkPart*>(doc);
    if (! artworkPart)
        return KoFilter::WrongFormat;

    // open Placeable Wmf file
    mWmf = new KoWmfWrite(m_chain->outputFile());
    if (!mWmf->begin()) {
        delete mWmf;
        return KoFilter::WrongFormat;
    }

    paintDocument(artworkPart->document());

    mWmf->end();

    delete mWmf;

    return KoFilter::OK;
}

void WmfExport::paintDocument(ArtworkDocument& document)
{

    // resolution
    mDpi = 1000;

    QSizeF pageSize = document.pageSize();
    int width = static_cast<int>(POINT_TO_INCH(pageSize.width()) * mDpi);
    int height = static_cast<int>(POINT_TO_INCH(pageSize.height()) * mDpi);

    mWmf->setDefaultDpi(mDpi);
    mWmf->setWindow(0, 0, width, height);

    if ((pageSize.width() != 0) && (pageSize.height() != 0)) {
        mScaleX = static_cast<double>(width) / pageSize.width();
        mScaleY = static_cast<double>(height) / pageSize.height();
    }

    QList<KShape*> shapes = document.shapes();
    qSort(shapes.begin(), shapes.end(), KShape::compareShapeZIndex);

    // Export layers.
    foreach(KShape * shape, shapes) {
        if (dynamic_cast<KShapeContainer*>(shape))
            continue;
        paintShape(shape);
    }
}

void WmfExport::paintShape(KShape * shape)
{
    QList<QPolygonF> subpaths = shape->outline().toFillPolygons(shape->absoluteTransformation(0));

    if (! subpaths.count())
        return;

    QList<QPolygon> polygons;
    foreach(const QPolygonF & subpath, subpaths) {
        QPolygon p;
        uint pointCount = subpath.count();
        for (uint i = 0; i < pointCount; ++i)
            p.append(QPoint(coordX(subpath[i].x()), coordY(subpath[i].y())));

        polygons.append(p);
    }
    mWmf->setPen(shape->border()->pen());

    if (polygons.count() == 1 && ! shape->background())
        mWmf->drawPolyline(polygons.first());
    else {
        QBrush fill(Qt::NoBrush);
        KColorBackground * cbg = dynamic_cast<KColorBackground*>(shape->background());
        if (cbg)
            fill = QBrush(cbg->color(), cbg->style());
        KGradientBackground * gbg = dynamic_cast<KGradientBackground*>(shape->background());
        if (gbg) {
            fill = QBrush(*gbg->gradient());
            fill.setTransform(gbg->transform());
        }
        KPatternBackground * pbg = dynamic_cast<KPatternBackground*>(shape->background());
        if (pbg) {
            fill.setTextureImage(pbg->pattern());
            fill.setTransform(pbg->transform());
        }
        mWmf->setBrush(fill);
        if (polygons.count() == 1)
            mWmf->drawPolygon(polygons.first());
        else
            mWmf->drawPolyPolygon(polygons);
    }
}

int WmfExport::coordX(double left)
{
    return (int)(left * mScaleX);
}

int WmfExport::coordY(double top)
{
    return (int)(top * mScaleY);
}

#include <wmfexport.moc>

