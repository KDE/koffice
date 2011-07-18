/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KARBON_1X_IMPORT_H
#define KARBON_1X_IMPORT_H

#include <KarbonDocument.h>

#include <KoFilter.h>
#include <KXmlReader.h>

#include <QtCore/QVariant>
#include <QtGui/QMatrix>

class KShape;
class KShapeContainer;

class KarbonImport : public KoFilter
{
    Q_OBJECT

public:
    KarbonImport(QObject* parent, const QVariantList&);
    virtual ~KarbonImport();

    virtual KoFilter::ConversionStatus convert(const QByteArray& from, const QByteArray& to);

protected:
    bool parseRoot(QIODevice* io);

    bool convert(const KXmlDocument &);
    bool loadXML(const KXmlElement& doc);

    void loadGroup(KShapeContainer * parent, const KXmlElement &element);
    void loadStyle(KShape * shape, const KXmlElement &element);
    void loadCommon(KShape * shape, const KXmlElement &element);
    void loadStroke(KShape * shape, const KXmlElement &element);
    void loadFill(KShape * shape, const KXmlElement &element);
    QColor loadColor(const KXmlElement &element);
    QVector<qreal> loadDashes(const KXmlElement& element);
    QBrush loadGradient(KShape * shape, const KXmlElement &element);
    void loadPattern(KShape * shape, const KXmlElement &element);

    KShape * loadPath(const KXmlElement &element);
    KShape * loadEllipse(const KXmlElement &element);
    KShape * loadRect(const KXmlElement &element);
    KShape * loadPolyline(const KXmlElement &element);
    KShape * loadPolygon(const KXmlElement &element);
    KShape * loadSinus(const KXmlElement &element);
    KShape * loadSpiral(const KXmlElement &element);
    KShape * loadStar(const KXmlElement &element);
    KShape * loadImage(const KXmlElement &element);
    KShape * loadText(const KXmlElement &element);

    double getAttribute(KXmlElement &element, const char *attributeName, double defaultValue);
    int getAttribute(KXmlElement &element, const char *attributeName, int defaultValue);
    int nextZIndex();
    KShape * createShape(const QString &shapeID) const;

private:
    KarbonDocument * m_document;
    QTransform m_mirrorMatrix;
};

#endif // KARBON_IMPORT_H
