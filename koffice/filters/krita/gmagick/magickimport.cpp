/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <qstring.h>
 
#include <magickimport.h>
#include <kgenericfactory.h>
#include <KoDocument.h>
#include <KoFilterChain.h>

#include <kis_doc.h>
#include <kis_view.h>
#include <kis_image_magick_converter.h>
#include <kis_progress_display_interface.h>
#include <kis_image.h>
#include <kis_layer.h>

typedef KGenericFactory<MagickImport, KoFilter> MagickImportFactory;
K_EXPORT_COMPONENT_FACTORY(libkritagmagickimport, MagickImportFactory("kofficefilters"))

MagickImport::MagickImport(KoFilter *, const char *, const QStringList&) : KoFilter()
{
}

MagickImport::~MagickImport()
{
}

KoFilter::ConversionStatus MagickImport::convert(const QCString&, const QCString& to)
{
    kdDebug(41008) << "Importing using MagickImport!\n";

    if (to != "application/x-krita")
        return KoFilter::BadMimeType;

    KisDoc * doc = dynamic_cast<KisDoc*>(m_chain -> outputDocument());
    KisView * view = static_cast<KisView*>(doc -> views().getFirst());
    
    QString filename = m_chain -> inputFile();
    
    if (!doc)
        return KoFilter::CreationError;

    doc -> prepareForImport();
        

    if (!filename.isEmpty()) {
    
        KURL url;
        url.setPath(filename);

        if (url.isEmpty())
            return KoFilter::FileNotFound;
            
        KisImageMagickConverter ib(doc, doc -> undoAdapter());

        if (view != 0)
            view -> canvasSubject() ->  progressDisplay() -> setSubject(&ib, false, true);

        switch (ib.buildImage(url)) {
            case KisImageBuilder_RESULT_UNSUPPORTED:
                return KoFilter::NotImplemented;
                break;
            case KisImageBuilder_RESULT_INVALID_ARG:
                return KoFilter::BadMimeType;
                break;
            case KisImageBuilder_RESULT_NO_URI:
            case KisImageBuilder_RESULT_NOT_LOCAL:
                return KoFilter::FileNotFound;
                break;
            case KisImageBuilder_RESULT_BAD_FETCH:
            case KisImageBuilder_RESULT_EMPTY:
                return KoFilter::ParsingError;                
                break;
            case KisImageBuilder_RESULT_FAILURE:
                return KoFilter::InternalError;
                break;
            case KisImageBuilder_RESULT_OK:
                doc -> setCurrentImage( ib.image());
                return KoFilter::OK;
            default:
                break;
        }

    }
    return KoFilter::StorageCreationError;
}

#include <magickimport.moc>

