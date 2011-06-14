/* This file is part of the KDE project
 * Copyright (C) 2002-2003,2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005,2007-2009 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SVGPARSER_H
#define SVGPARSER_H

#include "SvgGradientHelper.h"
#include "SvgPatternHelper.h"
#include "SvgFilterHelper.h"
#include "SvgGraphicContext.h"
#include "SvgCssHelper.h"

#include <KXmlReader.h>

#include <QtGui/QGradient>
#include <QtCore/QMap>
#include <QtCore/QStack>

class KShape;
class KShapeContainer;
class KoShapeGroup;
class KResourceManager;

class SvgParser
{
public:
    SvgParser(KResourceManager *documentResourceManager);
    virtual ~SvgParser();

    /// Parses a svg fragment, returning the list of top level child shapes
    QList<KShape*> parseSvg(const KXmlElement &e, QSizeF * fragmentSize = 0);

    /// Sets the initial xml base directory (the directory form where the file is read)
    void setXmlBaseDir(const QString &baseDir);

    /// Returns the list of all shapes of the svg document
    QList<KShape*> shapes() const;

protected:

    typedef QMap<QString, QString> SvgStyles;

    /// Parses a container element, returning a list of child shapes
    QList<KShape*> parseContainer(const KXmlElement &);
    /// Parses a use element, returning a list of child shapes
    QList<KShape*> parseUse(const KXmlElement &);
    /// Parses definitions for later use
    void parseDefs(const KXmlElement &);
    /// Parses style attributes, applying them to the given shape
    void parseStyle(KShape *, const KXmlElement &);
    void parseStyle(KShape *, const SvgStyles &);
    /// Parses a single style attribute
    void parsePA(SvgGraphicsContext *, const QString &, const QString &);
    /// Parses a gradient element
    bool parseGradient(const KXmlElement &, const KXmlElement &referencedBy = KXmlElement());
    /// Parses gradient color stops
    void parseColorStops(QGradient *, const KXmlElement &);
    /// Parses a pattern element
    void parsePattern(SvgPatternHelper &pattern, const KXmlElement &);
    /// Parses a filter element
    bool parseFilter(const KXmlElement &, const KXmlElement &referencedBy = KXmlElement());
    /// Parses a length attribute
    double parseUnit(const QString &, bool horiz = false, bool vert = false, QRectF bbox = QRectF());
    /// parses a length attribute in x-direction
    double parseUnitX(const QString &unit);
    /// parses a length attribute in y-direction
    double parseUnitY(const QString &unit);
    /// parses a length attribute in xy-direction
    double parseUnitXY(const QString &unit);
    /// Parses a color attribute
    bool parseColor(QColor &, const QString &);
    /// Parse a image
    bool parseImage(const QString &imageAttribute, QImage &image);
    /// Parses a viewbox attribute into an rectangle
    QRectF parseViewBox(QString viewbox);

    void setupTransform(const KXmlElement &);
    void updateContext(const KXmlElement &);
    void addGraphicContext();
    void removeGraphicContext();

    /// Creates an object from the given xml element
    KShape * createObject(const KXmlElement &, const SvgStyles &style = SvgStyles());
    /// Create text object from the given xml element
    KShape * createText(const KXmlElement &, const QList<KShape*> & shapes);
    /// Parses font attributes
    void parseFont(const SvgStyles &styles);
    /// find object with given id in document
    KShape * findObject(const QString &name);
    /// find object with given id in given group
    KShape * findObject(const QString &name, KShapeContainer *);
    /// find object with given if in given shape list
    KShape * findObject(const QString &name, const QList<KShape*> & shapes);
    /// find gradient with given id in gradient map
    SvgGradientHelper* findGradient(const QString &id, const QString &href = 0);
    /// find pattern with given id in pattern map
    SvgPatternHelper* findPattern(const QString &id);
    /// find filter with given id in filter map
    SvgFilterHelper* findFilter(const QString &id, const QString &href = 0);

    /// Creates style map from given xml element
    SvgStyles collectStyles(const KXmlElement &);
    /// Merges two style elements, returning the merged style
    SvgStyles mergeStyles(const SvgStyles &, const SvgStyles &);

    /// Adds list of shapes to the given group shape
    void addToGroup(QList<KShape*> shapes, KoShapeGroup * group);

    /// Returns the next z-index
    int nextZIndex();

    /// Constructs an absolute file path from the fiven href and base directory
    QString absoluteFilePath(const QString &href, const QString &xmlBase);

    /// creates a shape from the given shape id
    KShape * createShape(const QString &shapeID);

    /// Builds the document from the given shapes list
    void buildDocument(QList<KShape*> shapes);

    /// Applies the current fill style to the object
    void applyFillStyle(KShape * shape);

    /// Applies the current stroke style to the object
    void applyStrokeStyle(KShape * shape);

    /// Applies the current filter to the object
    void applyFilter(KShape * shape);

    /// Returns inherited attribute value for specified element
    QString inheritedAttribute(const QString &attributeName, const KXmlElement &e);

private:
    QSizeF m_documentSize;
    QStack<SvgGraphicsContext*>    m_gc;
    QMap<QString, SvgGradientHelper>  m_gradients;
    QMap<QString, SvgPatternHelper> m_patterns;
    QMap<QString, SvgFilterHelper> m_filters;
    QMap<QString, KXmlElement>     m_defs;
    QStringList m_fontAttributes; ///< font related attributes
    QStringList m_styleAttributes; ///< style related attributes
    KResourceManager *m_documentResourceManager;
    QList<KShape*> m_shapes;
    QList<KShape*> m_toplevelShapes;
    QString m_xmlBaseDir;
    SvgCssHelper m_cssStyles;
};

#endif
