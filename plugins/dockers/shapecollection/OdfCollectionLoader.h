/* This file is part of the KDE project
 * Copyright (C) 2008 Peter Simonsson <peter.simonsson@gmail.com>
 *
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
#ifndef KOODFCOLLECTIONLOADER_H
#define KOODFCOLLECTIONLOADER_H

#include <kurl.h>

#include <KXmlReader.h>

#include <QObject>
#include <QList>
#include <QStringList>

class KOdfStoreReader;
class KOdfLoadingContext;
class KShapeLoadingContext;
class QTimer;
class KShape;
class KoFilterManager;

class OdfCollectionLoader : public QObject
{
    Q_OBJECT
    public:
        explicit OdfCollectionLoader(const QString& path, QObject* parent = 0);
        ~OdfCollectionLoader();

        void load();

        QList<KShape*> shapeList() const { return m_shapeList; }
        QString collectionPath() const { return m_path; }

    protected:
        void nextFile();
        void loadNativeFile(const QString& path);
        QString findMimeTypeByUrl(const KUrl& url);

    protected slots:
        void loadShape();

    private:
        KOdfStoreReader* m_odfStore;
        QTimer* m_loadingTimer;
        KOdfLoadingContext* m_loadingContext;
        KShapeLoadingContext* m_shapeLoadingContext;
        KXmlElement m_body;
        KXmlElement m_page;
        KXmlElement m_shape;
        QList<KShape*> m_shapeList;
        QString m_path;
        QStringList m_fileList;
        KoFilterManager* m_filterManager;

    signals:
        /**
         * Emitted when the loading failed
         * @param reason Reason the loading failed.
         */
        void loadingFailed(const QString& reason);

        void loadingFinished();
};

#endif //KOODFCOLLECTIONLOADER_H
