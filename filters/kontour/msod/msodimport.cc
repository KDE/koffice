/*
    Copyright (C) 2000, S.R.Haque <shaheedhaque@hotmail.com>.
    This file is part of the KDE project

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

DESCRIPTION
*/

#include <kdebug.h>
#include <koFilterManager.h>
#include <koQueryTrader.h>
#include <koStore.h>
#include <ktempfile.h>
#include <msodimport.h>
#include <msodimport.moc>
#include <qpointarray.h>
#include <qfile.h>
#include <unistd.h>

MSODImport::MSODImport(
    KoFilter *parent,
    const char *name) :
        KoFilter(parent, name), Msod(100)
{
}

MSODImport::~MSODImport()
{
}

bool MSODImport::filter(
    const QString &fileIn,
    const QString &fileOut,
    const QString &prefixOut,
    const QString &from,
    const QString &to,
    const QString &config)
{
    if (to != "application/x-kontour" || from != "image/x-msod")
        return false;

    // Get configuration data: the shape id, and any delay stream that we were given.

    unsigned shapeId = (unsigned)-1;
    const char *delayStream = 0L;
    QStringList args = QStringList::split(";", config);
    unsigned i;

    kdDebug(s_area) << "MSODImport::filter: config: " << config << endl;
    for (i = 0; i < args.count(); i++)
    {
        if (args[i].startsWith("shape-id="))
        {
            shapeId = args[i].mid(9).toUInt();
        }
        else
        if (args[i].startsWith("delay-stream="))
        {
            delayStream = (const char *)args[i].mid(13).toULong();
        }
        else
        {
            kdError(s_area) << "Invalid argument: " << args[i] << endl;
            return false;
        }
    }
    m_prefixOut = prefixOut;
    m_nextPart = 0;

    m_text = "";
    m_text += "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    m_text += "<!DOCTYPE killustrator>\n";
    m_text += "<killustrator mime=\"application/x-killustrator\" version=\"3\" editor=\"MSOD import filter\">\n";
    m_text += " <head currentpagenum=\"2\">\n";
    m_text += "  <grid dx=\"50\" dy=\"50\" align=\"0\">\n";
    m_text += "   <helplines align=\"0\"/>\n";
    m_text += "  </grid>\n";
    m_text += " </head>\n";
    m_text += " <page id=\"Page 1\">\n";
    m_text += "  <layout width=\"210\" lmargin=\"0\" format=\"a4\" bmargin=\"0\" height=\"297\" rmargin=\"0\" tmargin=\"0\" orientation=\"portrait\"/>\n";
    m_text += "  <layer>\n";

    if (!parse(shapeId, fileIn, delayStream))
        return false;
    m_text += "  </layer>\n";
    m_text += " </page>\n";
    m_text += "</killustrator>\n";

    emit sigProgress(100);

    KoStore out = KoStore(fileOut, KoStore::Write);
    if (!out.open("root"))
    {
        kdError(s_area) << "Cannot open output file " << fileOut << endl;
        return false;
    }
    QCString cstring = m_text.utf8();
    out.write((const char*)cstring, cstring.length());
    out.close();

    // Now add in the data for all embedded parts.

    QMap<unsigned, Part>::Iterator it;

    for (it = m_parts.begin(); it != m_parts.end(); ++it)
    {
        // Now fetch out the elements from the resulting KoStore and embed them in our KoStore.

        KoStore storedPart(it.data().file, KoStore::Read);
        if (!out.embed(it.data().storageName, storedPart))
            kdError(s_area) << "Could not embed in KoStore!" << endl;
        unlink(it.data().file.local8Bit());
    }
    return true;
}

void MSODImport::gotEllipse(
    const DrawContext &dc,
    QString type,
    QPoint topLeft,
    QSize halfAxes,
    unsigned startAngle,
    unsigned stopAngle)
{
    m_text += "<ellipse angle1=\"" + QString::number(startAngle) +
                "\" angle2=\"" + QString::number(stopAngle) +
                "\" x=\"" + QString::number(topLeft.x()) +
                "\" y=\"" + QString::number(topLeft.y()) +
                "\" kind=\"" + type +
                "\" rx=\"" + QString::number(halfAxes.width()) +
                "\" ry=\"" + QString::number(halfAxes.height()) +
                "\">\n";
    m_text += " <gobject fillcolor=\"#" + QString::number(dc.m_brushColour, 16) +
                "\" fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</ellipse>\n";
}

void MSODImport::gotPicture(
    unsigned key,
    QString extension,
    unsigned length,
    const char *data)
{
    KTempFile tempFile(QString::null, "." + extension);

    tempFile.file()->writeBlock(data, length);
    tempFile.close();
    if ((extension == "wmf") ||
        (extension == "emf") ||
        (extension == "pict"))
    {
        Part part;
        QMap<unsigned, Part>::Iterator it = m_parts.find(key);

        if (it != m_parts.end())
        {
            // This part is already known! Extract the duplicate part.

            part = m_parts[key];
        }
        else
        {
            KoFilterManager *mgr = KoFilterManager::self();

            // It's not here, so let's generate one.

            part.fullName = m_prefixOut + '/' + QString::number(m_nextPart);
            part.storageName = "tar:/" + QString::number(m_nextPart);

            // Save the data supplied into a temporary file, then run the filter
            // on it.

            part.file = mgr->import(tempFile.name(), part.mimeType, "", part.fullName.mid(sizeof("tar:") - 1));
            if (part.file != QString::null)
            {
                m_parts.insert(key, part);
                m_nextPart++;
            }
            else
            {
                kdError(s_area) << "could not create part" << endl;
            }
        }
        unlink(tempFile.name().local8Bit());
        m_text += "<object url=\"" + part.fullName +
                    "\" mime=\"" + part.mimeType +
                    "\" x=\"0\" y=\"0\" width=\"100\" height=\"200\"/>\n";
    }
    else
    {
        // We could not import it as a part. Try as an image.

        m_text += "<pixmap src=\"" + tempFile.name() + "\">\n"
                    " <gobject fillstyle=\"0\" linewidth=\"1\" strokecolor=\"#000000\" strokestyle=\"1\">\n"
                    "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n"
                    " </gobject>\n"
                    "</pixmap>\n";

        // Note that we cannot delete the file...
    }
}

void MSODImport::gotPolygon(
    const DrawContext &dc,
    const QPointArray &points)
{
    QRect bounds = points.boundingRect();

    m_text += "<polygon width=\"" + QString::number(bounds.width()) +
                "\" x=\"" + QString::number(bounds.x()) +
                "\" y=\"" + QString::number(bounds.y()) +
                "\" height=\"" + QString::number(bounds.height()) +
                "\" rounding=\"0\">\n";
    m_text += "<polyline arrow1=\"0\" arrow2=\"0\">\n";
    pointArray(points);
    m_text += " <gobject fillcolor=\"#" + QString::number(dc.m_brushColour, 16) +
                "\" fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</polyline>\n";
    m_text += "</polygon>\n";
}


void MSODImport::gotPolyline(
    const DrawContext &dc,
    const QPointArray &points)
{
    m_text += "<polyline arrow1=\"0\" arrow2=\"0\">\n";
    pointArray(points);
    m_text += " <gobject fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</polyline>\n";
}

void MSODImport::gotRectangle(
    const DrawContext &dc,
    const QPointArray &points)
{
    QRect bounds = points.boundingRect();

    m_text += "<rectangle width=\"" + QString::number(bounds.width()) +
                "\" x=\"" + QString::number(bounds.x()) +
                "\" y=\"" + QString::number(bounds.y()) +
                "\" height=\"" + QString::number(bounds.height()) +
                "\" rounding=\"0\">\n";
    m_text += "<polyline arrow1=\"0\" arrow2=\"0\">\n";
    pointArray(points);
    m_text += " <gobject fillcolor=\"#" + QString::number(dc.m_brushColour, 16) +
                "\" fillstyle=\"" + QString::number(1 /*m_winding*/) +
                "\" linewidth=\"" + QString::number(dc.m_penWidth) +
                "\" strokecolor=\"#" + QString::number(dc.m_penColour, 16) +
                "\" strokestyle=\"" + QString::number(dc.m_penStyle) +
                "\">\n";
    m_text += "  <matrix dx=\"0\" dy=\"0\" m21=\"0\" m22=\"1\" m11=\"1\" m12=\"0\"/>\n";
    m_text += " </gobject>\n";
    m_text += "</polyline>\n";
    m_text += "</rectangle>\n";
}

void MSODImport::pointArray(
    const QPointArray &points)
{

    for (unsigned i = 0; i < points.count(); i++)
    {
        m_text += "<point x=\"" + QString::number(points.point(i).x()) +
                    "\" y=\"" + QString::number(points.point(i).y()) +
                     "\"/>\n";
    }
}
