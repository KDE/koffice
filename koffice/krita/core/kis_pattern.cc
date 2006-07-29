/*
 *  kis_pattern.cc - part of Krayon
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
 *                2001 John Califf
 *                2004 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_pattern.h"

#include <sys/types.h>
#include <netinet/in.h>

#include <limits.h>
#include <stdlib.h>

#include <qpoint.h>
#include <qsize.h>
#include <qimage.h>
#include <qvaluevector.h>
#include <qmap.h>
#include <qfile.h>

#include <kdebug.h>
#include <klocale.h>

#include "kis_color.h"
#include "kis_layer.h"
#include "kis_paint_device.h"

namespace {
    struct GimpPatternHeader {
        Q_UINT32 header_size;  /*  header_size = sizeof (PatternHeader) + brush name  */
        Q_UINT32 version;      /*  pattern file version #  */
        Q_UINT32 width;        /*  width of pattern */
        Q_UINT32 height;       /*  height of pattern  */
        Q_UINT32 bytes;        /*  depth of pattern in bytes : 1, 2, 3 or 4*/
        Q_UINT32 magic_number; /*  GIMP brush magic number  */
    };

    // Yes! This is _NOT_ what my pat.txt file says. It's really not 'GIMP', but 'GPAT'
    Q_UINT32 const GimpPatternMagic = (('G' << 24) + ('P' << 16) + ('A' << 8) + ('T' << 0));
}

KisPattern::KisPattern(const QString& file) : super(file), m_hasFile(true)
{
}

KisPattern::KisPattern(KisPaintDevice* image, int x, int y, int w, int h)
    : super(""), m_hasFile(false)
{
    // Forcefully convert to RGBA8
    // XXX profile and exposure?
    setImage(image->convertToQImage(0, x, y, w, h));
    setName(image->name());
}

KisPattern::~KisPattern()
{
}

bool KisPattern::load()
{
    if (!m_hasFile)
        return true;

    QFile file(filename());
    file.open(IO_ReadOnly);
    QByteArray data = file.readAll();
    if (!data.isEmpty()) {
        Q_INT32 startPos = m_data.size();

        m_data.resize(m_data.size() + data.count());
        memcpy(&m_data[startPos], data.data(), data.count());
    }
    file.close();
    return init();
}

bool KisPattern::save()
{
    QFile file(filename());
    file.open(IO_WriteOnly | IO_Truncate);

    QTextStream stream(&file);
    // Header: header_size (24+name length),version,width,height,colourdepth of brush,magic,name
    // depth: 1 = greyscale, 2 = greyscale + A, 3 = RGB, 4 = RGBA
    // magic = "GPAT", as a single uint32, the docs are wrong here!
    // name is UTF-8 (\0-terminated! The docs say nothing about this!)
    // _All_ data in network order, it seems! (not mentioned in gimp-2.2.8/devel-docs/pat.txt!!)
    // We only save RGBA at the moment
    // Version is 1 for now...

    GimpPatternHeader ph;
    QCString utf8Name = name().utf8();
    char const* name = utf8Name.data();
    int nameLength = qstrlen(name);

    ph.header_size = htonl(sizeof(GimpPatternHeader) + nameLength + 1); // trailing 0
    ph.version = htonl(1);
    ph.width = htonl(width());
    ph.height = htonl(height());
    ph.bytes = htonl(4);
    ph.magic_number = htonl(GimpPatternMagic);

    QByteArray bytes;
    bytes.setRawData(reinterpret_cast<char*>(&ph), sizeof(GimpPatternHeader));
    int wrote = file.writeBlock(bytes);
    bytes.resetRawData(reinterpret_cast<char*>(&ph), sizeof(GimpPatternHeader));

    if (wrote == -1)
        return false;

    wrote = file.writeBlock(name, nameLength + 1); // Trailing 0 apparantly!
    if (wrote == -1)
        return false;

    int k = 0;
    bytes.resize(width() * height() * 4);
    for (Q_INT32 y = 0; y < height(); y++) {
        for (Q_INT32 x = 0; x < width(); x++) {
            // RGBA only
            QRgb pixel = m_img.pixel(x,y);
            bytes[k++] = static_cast<char>(qRed(pixel));
            bytes[k++] = static_cast<char>(qGreen(pixel));
            bytes[k++] = static_cast<char>(qBlue(pixel));
            bytes[k++] = static_cast<char>(qAlpha(pixel));
        }
    }

    wrote = file.writeBlock(bytes);
    if (wrote == -1)
        return false;

    file.close();

    return true;
}

QImage KisPattern::img()
{
    return m_img;
}

bool KisPattern::init()
{
    // load Gimp patterns
    GimpPatternHeader bh;
    Q_INT32 k;
    QValueVector<char> name;

    if (sizeof(GimpPatternHeader) > m_data.size()) {
        return false;
    }

    memcpy(&bh, &m_data[0], sizeof(GimpPatternHeader));
    bh.header_size = ntohl(bh.header_size);
    bh.version = ntohl(bh.version);
    bh.width = ntohl(bh.width);
    bh.height = ntohl(bh.height);
    bh.bytes = ntohl(bh.bytes);
    bh.magic_number = ntohl(bh.magic_number);

    if (bh.header_size > m_data.size() || bh.header_size == 0) {
        return false;
    }

    name.resize(bh.header_size - sizeof(GimpPatternHeader));
    memcpy(&name[0], &m_data[sizeof(GimpPatternHeader)], name.size());

    if (name[name.size() - 1]) {
        return false;
    }

    setName(i18n(&name[0]));

    if (bh.width == 0 || bh.height == 0 || !m_img.create(bh.width, bh.height, 32)) {
        return false;
    }

    k = bh.header_size;

    if (bh.bytes == 1) {
        // Grayscale
        Q_INT32 val;

        for (Q_UINT32 y = 0; y < bh.height; y++) {
            for (Q_UINT32 x = 0; x < bh.width; x++, k++) {
                if (static_cast<Q_UINT32>(k) > m_data.size()) {
                    kdDebug(DBG_AREA_FILE) << "failed in gray\n";
                    return false;
                }

                val = m_data[k];
                m_img.setPixel(x, y, qRgb(val, val, val));
                m_img.setAlphaBuffer(false);
            }
        }
    } else if (bh.bytes == 2) {
        // Grayscale + A
        Q_INT32 val;
        Q_INT32 alpha;
        for (Q_UINT32 y = 0; y < bh.height; y++) {
            for (Q_UINT32 x = 0; x < bh.width; x++, k++) {
                if (static_cast<Q_UINT32>(k + 2) > m_data.size()) {
                    kdDebug(DBG_AREA_FILE) << "failed in grayA\n";
                    return false;
                }

                val = m_data[k];
                alpha = m_data[k++];
                m_img.setPixel(x, y, qRgba(val, val, val, alpha));
                m_img.setAlphaBuffer(true);
            }
        }
    } else if (bh.bytes == 3) {
        // RGB without alpha
        for (Q_UINT32 y = 0; y < bh.height; y++) {
            for (Q_UINT32 x = 0; x < bh.width; x++) {
                if (static_cast<Q_UINT32>(k + 3) > m_data.size()) {
                    kdDebug(DBG_AREA_FILE) << "failed in RGB\n";
                    return false;
                }

                m_img.setPixel(x, y, qRgb(m_data[k],
                              m_data[k + 1],
                              m_data[k + 2]));
                k += 3;
                m_img.setAlphaBuffer(false);
            }
        }
    } else if (bh.bytes == 4) {
        // Has alpha
        for (Q_UINT32 y = 0; y < bh.height; y++) {
            for (Q_UINT32 x = 0; x < bh.width; x++) {
                if (static_cast<Q_UINT32>(k + 4) > m_data.size()) {
                    kdDebug(DBG_AREA_FILE) << "failed in RGBA\n";
                    return false;
                }

                m_img.setPixel(x, y, qRgba(m_data[k],
                               m_data[k + 1],
                               m_data[k + 2],
                               m_data[k + 3]));
                k += 4;
                m_img.setAlphaBuffer(true);
            }
        }
    } else {
        return false;
    }

    if (m_img.isNull()) {
        return false;
    }

    setWidth(m_img.width());
    setHeight(m_img.height());

    setValid(true);

    return true;
}

KisPaintDeviceSP KisPattern::image(KisColorSpace * colorSpace) {
    // Check if there's already a pattern prepared for this colorspace
    QMap<QString, KisPaintDeviceSP>::const_iterator it = m_colorspaces.find(colorSpace->id().id());
    if (it != m_colorspaces.end())
        return (*it);

    // If not, create one
    KisPaintDeviceSP layer = new KisPaintDevice(colorSpace, "pattern");

    Q_CHECK_PTR(layer);

    layer->convertFromQImage(m_img,"");

    m_colorspaces[colorSpace->id().id()] = layer;
    return layer;
}

Q_INT32 KisPattern::width() const
{
    return m_width;
}

void KisPattern::setWidth(Q_INT32 w)
{
    m_width = w;
}

Q_INT32 KisPattern::height() const
{
    return m_height;
}

void KisPattern::setHeight(Q_INT32 h)
{
    m_height = h;
}

void KisPattern::setImage(const QImage& img)
{
    m_hasFile = false;
    m_img = img;
    m_img.detach();

    setWidth(img.width());
    setHeight(img.height());

    setValid(true);
}

KisPattern* KisPattern::clone() const
{
    KisPattern* pattern = new KisPattern("");
    pattern->setImage(m_img);
    pattern->setName(name());
    return pattern;
}

#include "kis_pattern.moc"
