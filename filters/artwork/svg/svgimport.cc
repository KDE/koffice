/* This file is part of the KDE project
 * Copyright (C) 2002-2005,2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2002-2004 Nicolas Goutte <nicolasg@snafu.de>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2005,2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2007 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2007-2008 Thorsten Zachmann <t.zachmann@zagge.de>

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include "svgimport.h"
#include "SvgParser.h"

#include <ArtworkPart.h>
#include <ArtworkDocument.h>

#include <KShape.h>
#include <KShapeLayer.h>
#include <KShapeGroup.h>
#include <KoFilterChain.h>
#include <commands/KShapeUngroupCommand.h>
#include <KXmlReader.h>

#include <kpluginfactory.h>
#include <KDebug>
#include <kfilterdev.h>

#include <QtCore/QFileInfo>

K_PLUGIN_FACTORY(SvgImportFactory, registerPlugin<SvgImport>();)
K_EXPORT_PLUGIN(SvgImportFactory("kofficefilters"))


SvgImport::SvgImport(QObject*parent, const QVariantList&)
        : KoFilter(parent), m_document(0)
{
}

SvgImport::~SvgImport()
{
}

KoFilter::ConversionStatus SvgImport::convert(const QByteArray& from, const QByteArray& to)
{
    // check for proper conversion
    if (to != "application/vnd.oasis.opendocument.graphics")
        return KoFilter::NotImplemented;
    if (from != "image/svg+xml" && from != "image/svg+xml-compressed")
        return KoFilter::NotImplemented;

    //Find the last extension
    QString strExt;
    QString fileIn(m_chain->inputFile());
    const int result = fileIn.lastIndexOf('.');
    if (result >= 0)
        strExt = fileIn.mid(result).toLower();

    QString strMime; // Mime type of the compressor
    if ((strExt == ".gz")    //in case of .svg.gz (logical extension)
            || (strExt == ".svgz")) //in case of .svgz (extension used prioritary)
        strMime = "application/x-gzip"; // Compressed with gzip
    else if (strExt == ".bz2") //in case of .svg.bz2 (logical extension)
        strMime = "application/x-bzip2"; // Compressed with bzip2
    else
        strMime = "text/plain";

    /*kDebug(30514) <<"File extension: -" << strExt <<"- Compression:" << strMime;*/

    QIODevice* in = KFilterDev::deviceForFile(fileIn, strMime);

    if (!in->open(QIODevice::ReadOnly)) {
        kError(30514) << "Cannot open file! Aborting!" << endl;
        delete in;
        return KoFilter::FileNotFound;
    }

    int line, col;
    QString errormessage;

    KXmlDocument inputDoc;

    const bool parsed = inputDoc.setContent(in, &errormessage, &line, &col);

    in->close();
    delete in;

    if (! parsed) {
        kError(30514) << "Error while parsing file: "
        << "at line " << line << " column: " << col
        << " message: " << errormessage << endl;
        // ### TODO: feedback to the user
        return KoFilter::ParsingError;
    }

    ArtworkPart * part = dynamic_cast<ArtworkPart*>(m_chain->outputDocument());
    if (! part)
        return KoFilter::CreationError;

    m_document = &part->document();

    // Do the conversion!
    convert(inputDoc.documentElement());

    return KoFilter::OK;
}

void SvgImport::convert(const KXmlElement &rootElement)
{
    if (! m_document)
        return;

    // set default page size to A4
    QSizeF pageSize(550.0, 841.0);

    SvgParser parser(m_document->resourceManager());

    parser.setXmlBaseDir(QFileInfo(m_chain->inputFile()).filePath());

    QList<KShape*> toplevelShapes = parser.parseSvg(rootElement, &pageSize);
    // parse the root svg element
    buildDocument(toplevelShapes, parser.shapes());

    // set the page size
    m_document->setPageSize(pageSize);
}

void SvgImport::buildDocument(const QList<KShape*> &toplevelShapes, const QList<KShape*> &shapes)
{
    // if we have only top level groups, make them layers
    bool onlyTopLevelGroups = true;
    foreach(KShape * shape, toplevelShapes) {
        if (! dynamic_cast<KShapeGroup*>(shape) || shape->filterEffectStack()) {
            onlyTopLevelGroups = false;
            break;
        }
    }

    // add all shapes to the document
    foreach(KShape * shape, shapes) {
        m_document->add(shape);
    }

    KShapeLayer * oldLayer = 0;
    if (m_document->layers().count())
        oldLayer = m_document->layers().first();

    if (onlyTopLevelGroups) {
        foreach(KShape * shape, toplevelShapes) {
            // ungroup toplevel groups
            KShapeGroup * group = dynamic_cast<KShapeGroup*>(shape);
            QList<KShape*> children = group->shapes();
            KShapeUngroupCommand cmd(group, children, QList<KShape*>() << group);
            cmd.redo();

            KShapeLayer * layer = new KShapeLayer();
            foreach(KShape * child, children) {
                m_document->add(child);
                layer->addShape(child);
            }
            if (! group->name().isEmpty())
                layer->setName(group->name());
            layer->setVisible(group->isVisible());
            layer->setZIndex(group->zIndex());
            m_document->insertLayer(layer);
            delete group;
        }
    } else {
        KShapeLayer * layer = new KShapeLayer();
        foreach(KShape * shape, toplevelShapes) {
            m_document->add(shape);
            layer->addShape(shape);
        }
        m_document->insertLayer(layer);
    }

    if (oldLayer) {
        m_document->removeLayer(oldLayer);
        delete oldLayer;
    }
}

#include <svgimport.moc>
