/*
 *  Copyright (c) 2005 Adrian Page <adrian@pagenet.plus.com>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <qfile.h>

#include <kmessagebox.h>

#include <half.h>
#include <ImfRgbaFile.h>

#include <kgenericfactory.h>
#include <KoDocument.h>
#include <KoFilterChain.h>

#include "kis_doc.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_annotation.h"
#include "kis_types.h"
#include "kis_iterators_pixel.h"
#include "kis_abstract_colorspace.h"
#include "kis_paint_device.h"
#include "kis_rgb_f32_colorspace.h"
#include "kis_rgb_f16half_colorspace.h"

#include "kis_openexr_export.h"

using namespace std;
using namespace Imf;
using namespace Imath;

typedef KGenericFactory<KisOpenEXRExport, KoFilter> KisOpenEXRExportFactory;
K_EXPORT_COMPONENT_FACTORY(libkrita_openexr_export, KisOpenEXRExportFactory("kofficefilters"))

KisOpenEXRExport::KisOpenEXRExport(KoFilter *, const char *, const QStringList&) : KoFilter()
{
}

KisOpenEXRExport::~KisOpenEXRExport()
{
}

KoFilter::ConversionStatus KisOpenEXRExport::convert(const QCString& from, const QCString& to)
{
    if (to != "image/x-exr" || from != "application/x-krita") {
        return KoFilter::NotImplemented;
    }

    kdDebug(41008) << "Krita exporting to OpenEXR\n";

    // XXX: Add dialog about flattening layers here

    KisDoc *doc = dynamic_cast<KisDoc*>(m_chain -> inputDocument());
    QString filename = m_chain -> outputFile();
    
    if (!doc) {
        return KoFilter::CreationError;
    }
    
    if (filename.isEmpty()) {
        return KoFilter::FileNotFound;
    }

    KisImageSP img = new KisImage(*doc -> currentImage());
    Q_CHECK_PTR(img);

    // Don't store this information in the document's undo adapter
    bool undo = doc -> undoAdapter() -> undo();
    doc -> undoAdapter() -> setUndo(false);

    img -> flatten();

    KisPaintLayerSP layer = dynamic_cast<KisPaintLayer*>(img->activeLayer().data());
    Q_ASSERT(layer);
    
    doc -> undoAdapter() -> setUndo(undo);

    //KisF32RgbColorSpace * cs = static_cast<KisF32RgbColorSpace *>((KisColorSpaceRegistry::instance() -> get(KisID("RGBAF32", ""))));
    KisRgbF16HalfColorSpace *cs = dynamic_cast<KisRgbF16HalfColorSpace *>(layer->paintDevice()->colorSpace());

    if (cs == 0) {
        // We could convert automatically, but the conversion wants to be done with
        // selectable profiles and rendering intent.
        KMessageBox::information(0, i18n("The image is using an unsupported color space. "
                                         "Please convert to 16-bit floating point RGB/Alpha "
                                         "before saving in the OpenEXR format."));

        // Don't show the couldn't save error dialog.
        doc -> setErrorMessage("USER_CANCELED");

        return KoFilter::WrongFormat;
    }

    Box2i displayWindow(V2i(0, 0), V2i(img -> width() - 1, img -> height() - 1));

    QRect dataExtent = layer -> exactBounds();
    int dataWidth = dataExtent.width();
    int dataHeight = dataExtent.height();

    Box2i dataWindow(V2i(dataExtent.left(), dataExtent.top()), V2i(dataExtent.right(), dataExtent.bottom()));

    RgbaOutputFile file(QFile::encodeName(filename), displayWindow, dataWindow, WRITE_RGBA);

    QMemArray<Rgba> pixels(dataWidth);

    for (int y = 0; y < dataHeight; ++y) {

        file.setFrameBuffer(pixels.data() - dataWindow.min.x - (dataWindow.min.y + y) * dataWidth, 1, dataWidth);

        KisHLineIterator it = layer->paintDevice()->createHLineIterator(dataWindow.min.x, dataWindow.min.y + y, dataWidth, false);
        Rgba *rgba = pixels.data();

        while (!it.isDone()) {

            // XXX: Currently we use unmultiplied data so premult it.
            half unmultipliedRed;
            half unmultipliedGreen;
            half unmultipliedBlue;
            half alpha;

            cs -> getPixel(it.rawData(), &unmultipliedRed, &unmultipliedGreen, &unmultipliedBlue, &alpha);
            rgba -> r = unmultipliedRed * alpha;
            rgba -> g = unmultipliedGreen * alpha;
            rgba -> b = unmultipliedBlue * alpha;
            rgba -> a = alpha;
            ++it;
            ++rgba;
        }
        file.writePixels();
    }

    //vKisAnnotationSP_it beginIt = img -> beginAnnotations();
    //vKisAnnotationSP_it endIt = img -> endAnnotations();
    return KoFilter::OK;
}

#include "kis_openexr_export.moc"

