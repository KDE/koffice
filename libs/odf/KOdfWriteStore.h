/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KODFWRITESTORE_H
#define KODFWRITESTORE_H

class QIODevice;
class KXmlWriter;
class KOdfStore;

#include "kodf_export.h"

/**
 * Helper class around KOdfStore for writing out ODF files.
 * This class helps solving the problem that automatic styles must be before
 * the body, but it's easier to iterate over the application's objects only
 * once. So we open a KXmlWriter into a memory buffer, write the body into it,
 * collect automatic styles while doing that, write out automatic styles,
 * and then copy the body XML from the buffer into the real KXmlWriter.
 *
 * The typical use of this class is therefore:
 *   - write body into bodyWriter() and collect auto styles
 *   - write auto styles into contentWriter()
 *   - call closeContentWriter()
 *   - write other files into the store (styles.xml, settings.xml etc.)
 *
 *
 * TODO: maybe we could encapsulate a bit more things, to e.g. handle
 * adding manifest entries automatically.
 *
 * @author: David Faure <faure@kde.org>
 */
class KODF_EXPORT KOdfWriteStore
{
public:
    /// @param store recontents the property of the caller
    explicit KOdfWriteStore(KOdfStore *store);

    ~KOdfWriteStore();

    /**
     * Return an XML writer for saving Oasis XML into the device @p dev,
     * including the XML processing instruction,
     * and the root element with all its namespaces.
     * You can add more namespaces afterwards with addAttribute.
     *
     * @param dev the device into which the XML will be written.
     * @param rootElementName the tag name of the root element.
     *    This is either office:document, office:document-content,
     *    office:document-styles, office:document-meta or office:document-settings
     * @return the KXmlWriter instance. It becomes owned by the caller, which
     * must delete it at some point.
     *
     * Once done with writing the contents of the root element, you
     * will need to call endElement(); endDocument(); before destroying the KXmlWriter.
     */
    static KXmlWriter *createOasisXmlWriter(QIODevice *dev, const char *rootElementName);

    KOdfStore *store() const;

    /**
     * Open contents.xml for writing and return the KXmlWriter
     */
    KXmlWriter *contentWriter();

    /**
     * Open another KXmlWriter for writing out the contents
     * into a temporary file, to collect automatic styles while doing that.
     */
    KXmlWriter *bodyWriter();

    /**
     * This will copy the body into the content writer,
     * delete the bodyWriter and the contentWriter, and then
     * close contents.xml.
     */
    bool closeContentWriter();

    // For other files in the store, use open/addManifestEntry/KOdfStorageDevice/createOasisXmlWriter/close

    /**
     * Create and return a manifest writer. It will write to a memory buffer.
     */
    KXmlWriter *manifestWriter(const char *mimeType);

    /**
     * Return the manifest writer. It has to be created by manifestWriter( mimeType ) before you can use
     * this function.
     */
    KXmlWriter *manifestWriter();

    /**
     * Close the manifest writer, writing its contents to manifest.xml
     */
    bool closeManifestWriter();

private:
    struct Private;
    Private * const d;
};

#endif /* KOODFWRITESTORE_H */
