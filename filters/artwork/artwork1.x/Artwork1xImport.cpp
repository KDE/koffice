/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2011 Thomas Zander <zander@kde.org>
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

#include "Artwork1xImport.h"

#include <ArtworkGlobal.h>
#include <ArtworkPart.h>

#include <KoFilterChain.h>
#include <KOdfStorageDevice.h>
#include <KOdf.h>
#include <KOdfPageLayoutData.h>
#include <KShape.h>
#include <KShapeContainer.h>
#include <KShapeLayer.h>
#include <KPathShape.h>
#include <KResourceManager.h>
#include <KPathShapeLoader.h>
#include <KShapeGroup.h>
#include <commands/KShapeGroupCommand.h>
#include <KLineBorder.h>
#include <pathshapes/ellipse/EllipseShape.h>
#include <pathshapes/rectangle/RectangleShape.h>
#include <pathshapes/star/StarShape.h>
#include <artistictextshape/ArtisticTextShape.h>
#include <pictureshape/PictureShape.h>
#include <KImageData.h>
#include <KImageCollection.h>
#include <KPathPoint.h>
#include <KoZoomHandler.h>
#include <KPatternBackground.h>
#include <KColorBackground.h>
#include <KGradientBackground.h>
#include <KShapeFactoryBase.h>
#include <KShapeRegistry.h>

#include <KDebug>
#include <KPluginFactory>

#include <QtGui/QTextCursor>
#include <QtCore/QFile>

#include <math.h>

K_PLUGIN_FACTORY(ArtworkImportFactory, registerPlugin<ArtworkImport>();)
K_EXPORT_PLUGIN(ArtworkImportFactory("kofficefilters"))

ArtworkImport::ArtworkImport(QObject*parent, const QVariantList&)
        : KoFilter(parent), m_document(0)
{
}

ArtworkImport::~ArtworkImport()
{
}

KoFilter::ConversionStatus ArtworkImport::convert(const QByteArray& from, const QByteArray& to)
{
    // check for proper conversion
    if (to != "application/vnd.oasis.opendocument.graphics" || from != "application/x-artwork")
        return KoFilter::NotImplemented;

    const QString fileName(m_chain->inputFile());
    if (fileName.isEmpty()) {
        kError() << "No input file name!";
        return KoFilter::StupidError;
    }

    ArtworkPart * part = dynamic_cast<ArtworkPart*>(m_chain->outputDocument());
    if (! part)
        return KoFilter::CreationError;

    m_document = &part->document();

    KOdfStore* store = KOdfStore::createStore(fileName, KOdfStore::Read);
    if (store && store->hasFile("maindoc.xml")) {

        if (! store->open("maindoc.xml")) {
            kError() << "Opening root has failed";
            delete store;
            return KoFilter::StupidError;
        }
        KOdfStorageDevice ioMain(store);
        ioMain.open(QIODevice::ReadOnly);
        if (! parseRoot(&ioMain)) {
            kWarning() << "Parsing maindoc.xml has failed! Aborting!";
            delete store;
            return KoFilter::StupidError;
        }
        ioMain.close();
        store->close();
    } else {
        kWarning() << "Opening store has failed. Trying raw XML file!";
        // Be sure to undefine store
        delete store;
        store = 0;

        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        if (! parseRoot(&file)) {
            kError() << "Could not process document! Aborting!";
            file.close();
            return KoFilter::StupidError;
        }
        file.close();
    }

    // We have finished with the input store/file, so close the store (already done for a raw XML file)
    delete store;
    store = 0;

    return KoFilter::OK;
}

bool ArtworkImport::parseRoot(QIODevice* io)
{
    int line, col;
    QString errormessage;

    KXmlDocument inputDoc;
    const bool parsed = inputDoc.setContent(io, &errormessage, &line, &col);

    if (! parsed) {
        kError() << "Error while parsing file: "
        << "at line " << line << " column: " << col
        << " message: " << errormessage;
        // ### TODO: feedback to the user
        return false;
    }

    // Do the conversion!
    convert(inputDoc);

    return true;
}

bool ArtworkImport::convert(const KXmlDocument &document)
{
    KXmlElement doc = document.documentElement();

    bool success = loadXML(doc);

    KOdfPageLayoutData pageLayout;

    // <PAPER>
    KXmlElement paper = doc.namedItem("PAPER").toElement();
    if (!paper.isNull()) {
        pageLayout.format = static_cast<KOdfPageFormat::Format>(getAttribute(paper, "format", 0));
        pageLayout.orientation = static_cast<KOdfPageFormat::Orientation>(getAttribute(paper, "orientation", 0));

        if (pageLayout.format == KOdfPageFormat::CustomSize) {
            pageLayout.width = m_document->pageSize().width();
            pageLayout.height = m_document->pageSize().height();
        } else {
            pageLayout.width = getAttribute(paper, "width", 0.0);
            pageLayout.height = getAttribute(paper, "height", 0.0);
        }
    } else {
        pageLayout.width = getAttribute(doc, "width", 595.277);
        pageLayout.height = getAttribute(doc, "height", 841.891);
    }

    KXmlElement borders = paper.namedItem("PAPERBORDERS").toElement();
    if (!borders.isNull()) {
        if (borders.hasAttribute("left"))
            pageLayout.leftMargin = borders.attribute("left").toDouble();
        if (borders.hasAttribute("top"))
            pageLayout.topMargin = borders.attribute("top").toDouble();
        if (borders.hasAttribute("right"))
            pageLayout.rightMargin = borders.attribute("right").toDouble();
        if (borders.hasAttribute("bottom"))
            pageLayout.bottomMargin = borders.attribute("bottom").toDouble();
    }

    // TODO set page layout to the output document

    return success;
}

double ArtworkImport::getAttribute(KXmlElement &element, const char *attributeName, double defaultValue)
{
    QString value = element.attribute(attributeName);
    if (! value.isEmpty())
        return value.toDouble();
    else
        return defaultValue;
}

int ArtworkImport::getAttribute(KXmlElement &element, const char *attributeName, int defaultValue)
{
    QString value = element.attribute(attributeName);
    if (! value.isEmpty())
        return value.toInt();
    else
        return defaultValue;
}

bool ArtworkImport::loadXML(const KXmlElement& doc)
{
    if (doc.attribute("mime") != "application/x-artwork" || doc.attribute("syntaxVersion") != "0.1")
        return false;

    double width = doc.attribute("width", "800.0").toDouble();
    double height = doc.attribute("height", "550.0").toDouble();

    m_document->setPageSize(QSizeF(width, height));
    //m_document->setUnit(KUnit::unit(doc.attribute("unit", KUnit::unitName(m_document->unit()))));

    m_mirrorMatrix.scale(1.0, -1.0);
    m_mirrorMatrix.translate(0, -m_document->pageSize().height());

    KShapeLayer * defaulLayer = m_document->layers().first();

    KXmlElement e;
    forEachElement(e, doc) {
        if (e.tagName() == "LAYER") {
            KShapeLayer * layer = new KShapeLayer();
            layer->setZIndex(nextZIndex());
            layer->setVisible(e.attribute("visible") == 0 ? false : true);
            loadGroup(layer, e);

            m_document->insertLayer(layer);
        }
    }

    if (defaulLayer && m_document->layers().count() > 1)
        m_document->removeLayer(defaulLayer);

    return true;
}

void ArtworkImport::loadGroup(KShapeContainer * parent, const KXmlElement &element)
{
    QList<KShape*> shapes;

    KXmlElement e;
    forEachElement(e, element) {
        KShape * shape = 0;
        if (e.tagName() == "COMPOSITE" || e.tagName() == "PATH") {
            shape = loadPath(e);
        } else if (e.tagName() == "ELLIPSE") {
            shape = loadEllipse(e);
        } else if (e.tagName() == "RECT") {
            shape = loadRect(e);
        } else if (e.tagName() == "POLYLINE") {
            shape = loadPolyline(e);
        } else if (e.tagName() == "POLYGON") {
            shape = loadPolygon(e);
        } else if (e.tagName() == "SINUS") {
            shape = loadSinus(e);
        } else if (e.tagName() == "SPIRAL") {
            shape = loadSpiral(e);
        } else if (e.tagName() == "STAR") {
            shape = loadStar(e);
        } else if (e.tagName() == "GROUP") {
            KShapeGroup * group = new KShapeGroup();
            group->setZIndex(nextZIndex());
            loadGroup(group, e);
            shape = group;
        }
        /* TODO
        else if( e.tagName() == "CLIP" )
        {
            VClipGroup* grp = new VClipGroup( this );
            grp->load( e );
            append( grp );
        }
        */
        else if (e.tagName() == "IMAGE") {
            shape = loadImage(e);
        } else if (e.tagName() == "TEXT") {
            shape = loadText(e);
        }
        if (shape)
            shapes.append(shape);
    }

    foreach(KShape * shape, shapes) {
        m_document->add(shape);
    }

    KShapeGroup * g = dynamic_cast<KShapeGroup*>(parent);
    if (g) {
        KShapeGroupCommand cmd(g, shapes);
        cmd.redo();
    } else {
        foreach(KShape * shape, shapes) {
            parent->addShape(shape);
        }
    }

    loadCommon(parent, element);
}

void ArtworkImport::loadStyle(KShape * shape, const KXmlElement &element)
{
    // reset fill and stroke first
    shape->setBorder(0);
    shape->setBackground(0);

    KXmlElement e;
    forEachElement(e, element) {
        if (e.tagName() == "STROKE") {
            loadStroke(shape, e);
        } else if (e.tagName() == "FILL") {
            loadFill(shape, e);
        }
    }
}

QColor ArtworkImport::loadColor(const KXmlElement &element)
{
    enum ColorSpace {
        rgb  = 0,  // the RGB colorspace (red, green and blue components)
        cmyk = 1,  // the CMYK colorspace (cyan, magenta, yellow and black components)
        hsb  = 2,  // the HSB colorspace (hue, saturation and brightnes components)
        gray = 3   // the Gray colorspace (gray from black to white)
    };

    ushort colorSpace = element.attribute("colorSpace").toUShort();

    double opacity = element.attribute("opacity", "1.0").toDouble();

    double value[4] = { 0 };

    if (colorSpace == gray)
        value[0] = element.attribute("v", "0.0").toDouble();
    else {
        value[0] = element.attribute("v1", "0.0").toDouble();
        value[1] = element.attribute("v2", "0.0").toDouble();
        value[2] = element.attribute("v3", "0.0").toDouble();

        if (colorSpace == cmyk)
            value[3] = element.attribute("v4", "0.0").toDouble();
    }

    if (value[0] < 0.0 || value[0] > 1.0)
        value[0] = 0.0;
    if (value[1] < 0.0 || value[1] > 1.0)
        value[1] = 0.0;
    if (value[2] < 0.0 || value[2] > 1.0)
        value[2] = 0.0;
    if (value[3] < 0.0 || value[3] > 1.0)
        value[3] = 0.0;

    QColor color;

    if (colorSpace == hsb)
        color.setHsvF(value[0], value[1], value[2], opacity);
    else if (colorSpace == gray)
        color.setRgbF(value[0], value[0], value[0], opacity);
    else if (colorSpace == cmyk)
        color.setCmykF(value[0], value[1], value[2], value[3], opacity);
    else
        color.setRgbF(value[0], value[1], value[2], opacity);

    return color;
}

QBrush ArtworkImport::loadGradient(KShape * shape, const KXmlElement &element)
{
    enum GradientType { linear = 0, radial = 1, conic  = 2 };
    enum GradientSpread { none = 0, reflect = 1, repeat  = 2 };


    QPointF origin;
    origin.setX(element.attribute("originX", "0.0").toDouble());
    origin.setY(element.attribute("originY", "0.0").toDouble());

    QPointF focal;
    focal.setX(element.attribute("focalX", "0.0").toDouble());
    focal.setY(element.attribute("focalY", "0.0").toDouble());

    QPointF vector;
    vector.setX(element.attribute("vectorX", "0.0").toDouble());
    vector.setY(element.attribute("vectorY", "0.0").toDouble());

    QSizeF shapeSize = shape->size();
    QTransform shapeMatrix = m_mirrorMatrix * shape->absoluteTransformation(0).inverted();
    origin = KoFlake::toRelative(shapeMatrix.map(origin), shapeSize);
    focal = KoFlake::toRelative(shapeMatrix.map(focal), shapeSize);
    vector = KoFlake::toRelative(shapeMatrix.map(vector), shapeSize);

    int type = element.attribute("type", 0).toInt();
    int spread = element.attribute("repeatMethod", 0).toInt();

    QGradient * gradient = 0;

    switch (type) {
    case linear: {
        QLinearGradient * g = new QLinearGradient();
        g->setStart(origin);
        g->setFinalStop(vector);
        gradient = g;
        break;
    }
    case radial: {
        QPointF diffVec = origin - vector;
        double radius = sqrt(diffVec.x() * diffVec.x() + diffVec.y() * diffVec.y());

        QRadialGradient * g = new QRadialGradient();
        g->setCenter(origin);
        g->setRadius(radius);
        g->setFocalPoint(focal);
        gradient = g;
        break;
    }
    case conic: {
        QPointF dirVec = vector - origin;
        double angle = atan2(dirVec.y(), dirVec.x()) * 180.0 / M_PI;
        QConicalGradient * g = new QConicalGradient();
        g->setCenter(origin);
        g->setAngle(angle);
        gradient = g;
        break;
    }
    }
    if (! gradient)
        return QBrush();

    QGradientStops stops;

    // load stops
    KXmlElement colorstop;
    forEachElement(colorstop, element) {
        if (colorstop.tagName() == "COLORSTOP") {
            QColor color = loadColor(colorstop.firstChild().toElement());
            double stop = colorstop.attribute("ramppoint", "0.0").toDouble();
            stops.append(QGradientStop(stop, color));
        }
    }
    gradient->setStops(stops);
    gradient->setCoordinateMode(QGradient::ObjectBoundingMode);
    switch (spread) {
    case reflect:
        gradient->setSpread(QGradient::ReflectSpread);
        break;
    case repeat:
        gradient->setSpread(QGradient::RepeatSpread);
        break;
    default:
        gradient->setSpread(QGradient::PadSpread);
        break;
    }

    QBrush gradientBrush(*gradient);
    delete gradient;

    return gradientBrush;
}

void ArtworkImport::loadPattern(KShape * shape, const KXmlElement &element)
{
    QPointF origin;
    origin.setX(element.attribute("originX", "0.0").toDouble());
    origin.setY(element.attribute("originY", "0.0").toDouble());
    origin = m_mirrorMatrix.map(origin) - shape->position();

    QPointF vector;
    vector.setX(element.attribute("vectorX", "0.0").toDouble());
    vector.setY(element.attribute("vectorY", "0.0").toDouble());
    vector = m_mirrorMatrix.map(vector) - shape->position();

    QPointF dirVec = vector - origin;
    double angle = atan2(dirVec.y(), dirVec.x()) * 180.0 / M_PI;

    QTransform m;
    m.translate(origin.x(), origin.y());
    m.rotate(angle);

    QString fname = element.attribute("tilename");

    QImage img;
    if (! img.load(fname)) {
        kWarning() << "Failed to load pattern image" << fname;
        return;
    }

    KImageCollection *imageCollection = m_document->resourceManager()->imageCollection();
    if (imageCollection) {
        KPatternBackground * newFill = new KPatternBackground(imageCollection);
        newFill->setPattern(img.mirrored(false, true));
        newFill->setTransform(m);
        shape->setBackground(newFill);
    }
}

QVector<qreal> ArtworkImport::loadDashes(const KXmlElement& element)
{
    QVector<qreal> dashes;

    KXmlElement e;
    forEachElement(e, element) {
        if (e.tagName() == "DASH") {
            double value = qMax(qreal(0.0), e.attribute("l", "0.0").toDouble());
            dashes.append(value);
        }
    }
    return dashes;
}

void ArtworkImport::loadStroke(KShape * shape, const KXmlElement &element)
{
    KLineBorder * border = new KLineBorder();

    QPen pen;
    switch (element.attribute("lineCap", "0").toUShort()) {
    case 1:
        pen.setCapStyle(Qt::RoundCap); break;
    case 2:
        pen.setCapStyle(Qt::SquareCap); break;
    default:
        pen.setCapStyle(Qt::FlatCap);
    }

    switch (element.attribute("lineJoin", "0").toUShort()) {
    case 1:
        pen.setJoinStyle(Qt::RoundJoin);; break;
    case 2:
        pen.setJoinStyle(Qt::BevelJoin); break;
    default:
        pen.setJoinStyle(Qt::MiterJoin);
    }

    pen.setWidth(qMax(qreal(0.0), element.attribute("lineWidth", "1.0").toDouble()));
    pen.setMiterLimit(qMax(qreal(0.0), element.attribute("miterLimit", "10.0").toDouble()));

    bool hasStroke = false;

    KXmlElement e;
    forEachElement(e, element) {
        if (e.tagName() == "COLOR") {
            pen.setColor(loadColor(e));
            hasStroke = true;
        } else if (e.tagName() == "DASHPATTERN") {
            double dashOffset = element.attribute("offset", "0.0").toDouble();
            pen.setDashPattern(loadDashes(e));
            pen.setStyle(Qt::CustomDashLine);
            pen.setDashOffset(dashOffset);
            hasStroke = true;
        } else if (e.tagName() == "GRADIENT") {
            pen.setBrush(loadGradient(shape, e));
            hasStroke = true;
        }
        /* TODO gradient and pattern on stroke not yet implemented in flake
        else if( e.tagName() == "PATTERN" )
        {
            m_type = patt;
            m_pattern.load( e );
        }
        */
    }
    border->setPen(pen);

    if (hasStroke)
        shape->setBorder(border);
    else
        delete border;
}

void ArtworkImport::loadFill(KShape * shape, const KXmlElement &element)
{
    QBrush fill;

    KXmlElement e;
    forEachElement(e, element) {
        if (e.tagName() == "COLOR") {
            KColorBackground * newFill = new KColorBackground(loadColor(e));
            shape->setBackground(newFill);
        }
        if (e.tagName() == "GRADIENT") {
            QBrush brush = loadGradient(shape, e);
            KGradientBackground * newFill = new KGradientBackground(*brush.gradient());
            newFill->setTransform(brush.transform());
            shape->setBackground(newFill);
        } else if (e.tagName() == "PATTERN") {
            loadPattern(shape, e);
        }
    }
}

void ArtworkImport::loadCommon(KShape * shape, const KXmlElement &element)
{
    if (! element.attribute("ID").isEmpty())
        shape->setName(element.attribute("ID"));

    QString trafo = element.attribute("transform");

    if (!trafo.isEmpty())
        shape->applyAbsoluteTransformation(KOdf::loadTransformation(trafo));

    if (dynamic_cast<KShapeContainer*>(shape))
        return;

    // apply mirroring
    shape->applyAbsoluteTransformation(m_mirrorMatrix);
}

KShape * ArtworkImport::loadPath(const KXmlElement &element)
{
    KPathShape * path = new KPathShape();

    QString data = element.attribute("d");
    if (data.length() > 0) {
        KPathShapeLoader loader(path);
        loader.parseSvg(data, true);
        path->normalize();
    }

    path->setFillRule(element.attribute("fillRule") == 0 ? Qt::OddEvenFill : Qt::WindingFill);

    KXmlElement child;
    forEachElement(child, element) {
        // backward compatibility for artwork before koffice 1.3.x
        if (child.tagName() == "PATH") {
            KPathShape * subpath = new KPathShape();

            KXmlElement segment;
            forEachElement(segment, child) {
                if (segment.tagName() == "MOVE") {
                    subpath->moveTo(QPointF(segment.attribute("x").toDouble(), segment.attribute("y").toDouble()));
                } else if (segment.tagName() == "LINE") {
                    subpath->lineTo(QPointF(segment.attribute("x").toDouble(), segment.attribute("y").toDouble()));
                } else if (segment.tagName() == "CURVE") {
                    QPointF p0(segment.attribute("x1").toDouble(), segment.attribute("y1").toDouble());
                    QPointF p1(segment.attribute("x2").toDouble(), segment.attribute("y2").toDouble());
                    QPointF p2(segment.attribute("x3").toDouble(), segment.attribute("y3").toDouble());
                    subpath->curveTo(p0, p1, p2);
                }
            }

            if (child.attribute("isClosed") == 0 ? false : true)
                path->close();

            path->combine(subpath);
        }
    }

    loadCommon(path, element);
    loadStyle(path, element);
    path->setZIndex(nextZIndex());

    return path;
}

KShape * ArtworkImport::loadEllipse(const KXmlElement &element)
{
    EllipseShape * ellipse = new EllipseShape();

    double rx = KUnit::parseValue(element.attribute("rx"));
    double ry = KUnit::parseValue(element.attribute("ry"));
    ellipse->setSize(QSizeF(2*rx, 2*ry));

    ellipse->setStartAngle(element.attribute("start-angle").toDouble());
    ellipse->setEndAngle(element.attribute("end-angle").toDouble());

    if (element.attribute("kind") == "cut")
        ellipse->setType(EllipseShape::Chord);
    else if (element.attribute("kind") == "section")
        ellipse->setType(EllipseShape::Pie);
    else if (element.attribute("kind") == "arc")
        ellipse->setType(EllipseShape::Arc);

    QPointF center(KUnit::parseValue(element.attribute("cx")), KUnit::parseValue(element.attribute("cy")));
    ellipse->setAbsolutePosition(center);

    loadCommon(ellipse, element);
    loadStyle(ellipse, element);
    ellipse->setZIndex(nextZIndex());

    return ellipse;
}

KShape * ArtworkImport::loadRect(const KXmlElement &element)
{
    RectangleShape * rect = new RectangleShape();

    double w  = KUnit::parseValue(element.attribute("width"), 10.0);
    double h = KUnit::parseValue(element.attribute("height"), 10.0);
    rect->setSize(QSizeF(w, h));

    double x = KUnit::parseValue(element.attribute("x"));
    double y = KUnit::parseValue(element.attribute("y"));
    rect->setAbsolutePosition(QPointF(x, y), KoFlake::BottomLeftCorner);

    double rx  = KUnit::parseValue(element.attribute("rx"));
    double ry  = KUnit::parseValue(element.attribute("ry"));
    rect->setCornerRadiusX(rx / (0.5 * w) * 100.0);
    rect->setCornerRadiusY(ry / (0.5 * h) * 100.0);

    loadCommon(rect, element);
    loadStyle(rect, element);
    rect->setZIndex(nextZIndex());

    return rect;
}

KShape * ArtworkImport::loadPolyline(const KXmlElement &element)
{
    KPathShape * polyline = new KPathShape();

    QString points = element.attribute("points").simplified();

    bool bFirst = true;

    points.replace(',', ' ');
    points.remove('\r');
    points.remove('\n');
    QStringList pointList = points.split(' ');
    QStringList::Iterator end(pointList.end());
    for (QStringList::Iterator it = pointList.begin(); it != end; ++it) {
        QPointF point;
        point.setX((*it).toDouble());
        point.setY((*++it).toDouble());
        if (bFirst) {
            polyline->moveTo(point);
            bFirst = false;
        } else
            polyline->lineTo(point);
    }

    loadCommon(polyline, element);
    loadStyle(polyline, element);
    polyline->setZIndex(nextZIndex());

    return polyline;
}

KShape * ArtworkImport::loadPolygon(const KXmlElement &element)
{
    KPathShape * polygon = new KPathShape();

    QString points = element.attribute("points").simplified();

    bool bFirst = true;

    points.replace(',', ' ');
    points.remove('\r');
    points.remove('\n');
    QStringList pointList = points.split(' ');
    QStringList::Iterator end(pointList.end());
    for (QStringList::Iterator it = pointList.begin(); it != end; ++it) {
        QPointF point;
        point.setX((*it).toDouble());
        point.setY((*++it).toDouble());
        if (bFirst) {
            polygon->moveTo(point);
            bFirst = false;
        } else
            polygon->lineTo(point);
    }
    polygon->close();

    double x = KUnit::parseValue(element.attribute("x"));
    double y = KUnit::parseValue(element.attribute("y"));
    polygon->setAbsolutePosition(QPointF(x, y), KoFlake::TopLeftCorner);

    loadCommon(polygon, element);
    loadStyle(polygon, element);
    polygon->setZIndex(nextZIndex());

    return polygon;
}

KShape * ArtworkImport::loadSinus(const KXmlElement &element)
{
    KPathShape * sinus = new KPathShape();

    uint periods = element.attribute("periods").toUInt();

    QPointF p1, p2, p3;
    sinus->moveTo(QPointF(0, 0));

    for (uint i = 0; i < periods; ++i) {
        p1.setX(i + 1.0 / 24.0);
        p1.setY((2.0 * ArtworkGlobal::sqrt2 - 1.0) * ArtworkGlobal::one_7);
        p2.setX(i + 1.0 / 12.0);
        p2.setY((4.0 * ArtworkGlobal::sqrt2 - 2.0) * ArtworkGlobal::one_7);
        p3.setX(i + 1.0 / 8.0);
        p3.setY(ArtworkGlobal::sqrt2 * 0.5);
        sinus->curveTo(p1, p2, p3);

        p1.setX(i + 1.0 / 6.0);
        p1.setY((3.0 * ArtworkGlobal::sqrt2 + 2.0) * ArtworkGlobal::one_7);
        p2.setX(i + 5.0 / 24.0);
        p2.setY(1.0);
        p3.setX(i + 1.0 / 4.0);
        p3.setY(1.0);
        sinus->curveTo(p1, p2, p3);

        p1.setX(i + 7.0 / 24.0);
        p1.setY(1.0);
        p2.setX(i + 1.0 / 3.0);
        p2.setY((3.0 * ArtworkGlobal::sqrt2 + 2.0) * ArtworkGlobal::one_7);
        p3.setX(i + 3.0 / 8.0);
        p3.setY(ArtworkGlobal::sqrt2 * 0.5);
        sinus->curveTo(p1, p2, p3);

        p1.setX(i + 5.0 / 12.0);
        p1.setY((4.0 * ArtworkGlobal::sqrt2 - 2.0) * ArtworkGlobal::one_7);
        p2.setX(i + 11.0 / 24.0);
        p2.setY((2.0 * ArtworkGlobal::sqrt2 - 1.0) * ArtworkGlobal::one_7);
        p3.setX(i + 1.0 / 2.0);
        p3.setY(0.0);
        sinus->curveTo(p1, p2, p3);

        p1.setX(i + 13.0 / 24.0);
        p1.setY(-(2.0 * ArtworkGlobal::sqrt2 - 1.0) * ArtworkGlobal::one_7);
        p2.setX(i + 7.0 / 12.0);
        p2.setY(-(4.0 * ArtworkGlobal::sqrt2 - 2.0) * ArtworkGlobal::one_7);
        p3.setX(i + 5.0 / 8.0);
        p3.setY(-ArtworkGlobal::sqrt2 * 0.5);
        sinus->curveTo(p1, p2, p3);

        p1.setX(i + 2.0 / 3.0);
        p1.setY(-(3.0 * ArtworkGlobal::sqrt2 + 2.0) * ArtworkGlobal::one_7);
        p2.setX(i + 17.0 / 24.0);
        p2.setY(-1.0);
        p3.setX(i + 3.0 / 4.0);
        p3.setY(-1.0);
        sinus->curveTo(p1, p2, p3);

        p1.setX(i + 19.0 / 24.0);
        p1.setY(-1.0);
        p2.setX(i + 5.0 / 6.0);
        p2.setY(-(3.0 * ArtworkGlobal::sqrt2 + 2.0) * ArtworkGlobal::one_7);
        p3.setX(i + 7.0 / 8.0);
        p3.setY(-ArtworkGlobal::sqrt2 * 0.5);
        sinus->curveTo(p1, p2, p3);

        p1.setX(i + 11.0 / 12.0);
        p1.setY(-(4.0 * ArtworkGlobal::sqrt2 - 2.0) * ArtworkGlobal::one_7);
        p2.setX(i + 23.0 / 24.0);
        p2.setY(-(2.0 * ArtworkGlobal::sqrt2 - 1.0) * ArtworkGlobal::one_7);
        p3.setX(i + 1.0);
        p3.setY(0.0);
        sinus->curveTo(p1, p2, p3);
    }

    sinus->normalize();

    double x = KUnit::parseValue(element.attribute("x"));
    double y = KUnit::parseValue(element.attribute("y"));

    double w  = KUnit::parseValue(element.attribute("width"), 10.0);
    double h = KUnit::parseValue(element.attribute("height"), 10.0);

    sinus->setAbsolutePosition(QPointF(x, y - h)/*, KoFlake::TopLeftCorner*/);
    sinus->setSize(QSizeF(w / periods, h));

    loadCommon(sinus, element);
    loadStyle(sinus, element);
    sinus->setZIndex(nextZIndex());

    return sinus;
}

KShape * ArtworkImport::loadSpiral(const KXmlElement &element)
{
    enum SpiralType { round, rectangular };

    KPathShape * spiral = new KPathShape();

    double radius  = qAbs(KUnit::parseValue(element.attribute("radius")));
    double angle = element.attribute("angle").toDouble();
    double fade = element.attribute("fade").toDouble();

    double cx = KUnit::parseValue(element.attribute("cx"));
    double cy = KUnit::parseValue(element.attribute("cy"));

    uint segments  = element.attribute("segments").toUInt();
    int clockwise = element.attribute("clockwise").toInt();
    int type = element.attribute("type").toInt();


    // It makes sense to have at least one segment:
    if (segments < 1)
        segments = 1;

    // Fall back, when fade is out of range:
    if (fade <= 0.0 || fade >= 1.0)
        fade = 0.5;

    spiral->setFillRule(Qt::WindingFill);

    // advance by pi/2 clockwise or cclockwise?
    double adv_ang = (clockwise ? 1.0 : -1.0) * 90.0;
    double adv_rad = (clockwise ? -1.0 : 1.0) * ArtworkGlobal::pi_2;
    // radius of first segment is non-faded radius:
    double r = radius;

    QPointF oldP(0.0, (clockwise ? -1.0 : 1.0) * radius);
    QPointF newP;
    QPointF newCenter(0.0, 0.0);

    spiral->moveTo(oldP);

    double startAngle = clockwise ? 90.0 : -90.0;

    for (uint i = 0; i < segments; ++i) {

        if (type == round) {
            spiral->arcTo(r, r, startAngle, 90);
        } else {
            newP.setX(r * cos(adv_rad * (i + 2)) + newCenter.x());
            newP.setY(r * sin(adv_rad * (i + 2)) + newCenter.y());

            spiral->lineTo(newP);

            newCenter += (newP - newCenter) * (1.0 - fade);
            oldP = newP;
        }

        r *= fade;
        startAngle += adv_ang;
    }

    QPointF topLeft = spiral->outline().boundingRect().topLeft();
    spiral->normalize();

    QTransform m;

    // sadly it's not feasible to simply add angle while creation.
    // make cw-spiral start at mouse-pointer
    // one_pi_180 = 1/(pi/180) = 180/pi.
    m.rotate((angle + (clockwise ? ArtworkGlobal::pi : 0.0)) * ArtworkGlobal::one_pi_180);

    spiral->applyAbsoluteTransformation(m);
    spiral->setAbsolutePosition(spiral->absolutePosition() + QPointF(cx, cy));

    loadCommon(spiral, element);
    loadStyle(spiral, element);
    spiral->setZIndex(nextZIndex());

    return spiral;
}

KShape * ArtworkImport::loadStar(const KXmlElement &element)
{
    enum StarType { star_outline, spoke, wheel, polygon, framed_star, star, gear };

    double cx = KUnit::parseValue(element.attribute("cx"));
    double cy = KUnit::parseValue(element.attribute("cy"));

    double outerRadius  = qAbs(KUnit::parseValue(element.attribute("outerradius")));
    double innerRadius  = qAbs(KUnit::parseValue(element.attribute("innerradius")));
    uint edges  = qMax(element.attribute("edges").toUInt(), static_cast<uint>(3));

    double innerAngle  = element.attribute("innerangle").toUInt();
    double angle = element.attribute("angle").toDouble();

    double roundness  = element.attribute("roundness").toDouble();

    int type = element.attribute("type").toInt();

    KPathShape * starShape = 0;

    if (type == star_outline || type == polygon) {
        StarShape * paramStar = new StarShape();

        paramStar->setCornerCount(edges);
        paramStar->setBaseRadius(innerRadius);
        paramStar->setTipRadius(outerRadius);
        paramStar->setBaseRoundness(roundness);
        paramStar->setTipRoundness(roundness);
        paramStar->setConvex(type == polygon);

        QPointF centerPos = paramStar->absolutePosition(KoFlake::TopLeftCorner) + paramStar->starCenter();
        QTransform m;
        m.translate(centerPos.x(), centerPos.y());
        m.rotate((angle + ArtworkGlobal::pi) * ArtworkGlobal::one_pi_180);
        paramStar->applyAbsoluteTransformation(m);

        starShape = paramStar;
    } else {

        KPathShape * pathStar = new KPathShape();

        // We start at angle + ArtworkGlobal::pi_2:
        QPointF p2, p3;
        QPointF p(
            outerRadius * cos(angle + ArtworkGlobal::pi_2),
            outerRadius * sin(angle + ArtworkGlobal::pi_2));
        pathStar->moveTo(p);

        double inAngle = ArtworkGlobal::twopi / 360 * innerAngle;

        if (type == star) {
            int j = (edges % 2 == 0) ? (edges - 2) / 2 : (edges - 1) / 2;
            //innerRadius = getOptimalInnerRadius( outerRadius, edges, innerAngle );
            int jumpto = 0;
            bool discontinueous = (edges % 4 == 2);

            double outerRoundness = (ArtworkGlobal::twopi * outerRadius * roundness) / edges;
            double nextOuterAngle;

            for (uint i = 1; i < edges + 1; ++i) {
                double nextInnerAngle = angle + inAngle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * (jumpto + 0.5);
                p.setX(innerRadius * cos(nextInnerAngle));
                p.setY(innerRadius * sin(nextInnerAngle));
                if (roundness == 0.0)
                    pathStar->lineTo(p);
                else {
                    nextOuterAngle = angle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * jumpto;
                    p2.setX(outerRadius * cos(nextOuterAngle) -
                            cos(angle + ArtworkGlobal::twopi / edges * jumpto) * outerRoundness);
                    p2.setY(outerRadius * sin(nextOuterAngle) -
                            sin(angle + ArtworkGlobal::twopi / edges * jumpto) * outerRoundness);

                    pathStar->curveTo(p2, p, p);
                }

                jumpto = (i * j) % edges;
                nextInnerAngle = angle + inAngle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * (jumpto - 0.5);
                p.setX(innerRadius * cos(nextInnerAngle));
                p.setY(innerRadius * sin(nextInnerAngle));
                pathStar->lineTo(p);

                nextOuterAngle = angle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * jumpto;
                p.setX(outerRadius * cos(nextOuterAngle));
                p.setY(outerRadius * sin(nextOuterAngle));

                if (roundness == 0.0)
                    pathStar->lineTo(p);
                else {
                    p2.setX(innerRadius * cos(nextInnerAngle));
                    p2.setY(innerRadius * sin(nextInnerAngle));

                    p3.setX(outerRadius * cos(nextOuterAngle) +
                            cos(angle + ArtworkGlobal::twopi / edges * jumpto) * outerRoundness);
                    p3.setY(outerRadius * sin(nextOuterAngle) +
                            sin(angle + ArtworkGlobal::twopi / edges * jumpto) * outerRoundness);

                    pathStar->curveTo(p2, p3, p);
                }
                if (discontinueous && i == (edges / 2)) {
                    angle += ArtworkGlobal::pi;
                    nextOuterAngle = angle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * jumpto;
                    p.setX(outerRadius * cos(nextOuterAngle));
                    p.setY(outerRadius * sin(nextOuterAngle));
                    pathStar->moveTo(p);
                }
            }
        } else {
            if (type == wheel || type == spoke)
                innerRadius = 0.0;

            double innerRoundness = (ArtworkGlobal::twopi * innerRadius * roundness) / edges;
            double outerRoundness = (ArtworkGlobal::twopi * outerRadius * roundness) / edges;

            for (uint i = 0; i < edges; ++i) {
                double nextOuterAngle = angle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * (i + 1.0);
                double nextInnerAngle = angle + inAngle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * (i + 0.5);
                if (type != polygon) {
                    p.setX(innerRadius * cos(nextInnerAngle));
                    p.setY(innerRadius * sin(nextInnerAngle));

                    if (roundness == 0.0)
                        pathStar->lineTo(p);
                    else {
                        p2.setX(outerRadius *
                                cos(angle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * (i)) -
                                cos(angle + ArtworkGlobal::twopi / edges * (i)) * outerRoundness);
                        p2.setY(outerRadius *
                                sin(angle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * (i)) -
                                sin(angle + ArtworkGlobal::twopi / edges * (i)) * outerRoundness);

                        p3.setX(innerRadius * cos(nextInnerAngle) +
                                cos(angle + inAngle + ArtworkGlobal::twopi / edges * (i + 0.5)) * innerRoundness);
                        p3.setY(innerRadius * sin(nextInnerAngle) +
                                sin(angle + inAngle + ArtworkGlobal::twopi / edges * (i + 0.5)) * innerRoundness);

                        if (type == gear) {
                            pathStar->lineTo(p2);
                            pathStar->lineTo(p3);
                            pathStar->lineTo(p);
                        } else
                            pathStar->curveTo(p2, p3, p);
                    }
                }

                p.setX(outerRadius * cos(nextOuterAngle));
                p.setY(outerRadius * sin(nextOuterAngle));

                if (roundness == 0.0)
                    pathStar->lineTo(p);
                else {
                    p2.setX(innerRadius * cos(nextInnerAngle) -
                            cos(angle + inAngle + ArtworkGlobal::twopi / edges * (i + 0.5)) * innerRoundness);
                    p2.setY(innerRadius * sin(nextInnerAngle) -
                            sin(angle + inAngle + ArtworkGlobal::twopi / edges * (i + 0.5)) * innerRoundness);

                    p3.setX(outerRadius * cos(nextOuterAngle) +
                            cos(angle + ArtworkGlobal::twopi / edges * (i + 1.0)) * outerRoundness);
                    p3.setY(outerRadius * sin(nextOuterAngle) +
                            sin(angle + ArtworkGlobal::twopi / edges * (i + 1.0)) * outerRoundness);

                    if (type == gear) {
                        pathStar->lineTo(p2);
                        pathStar->lineTo(p3);
                        pathStar->lineTo(p);
                    } else
                        pathStar->curveTo(p2, p3, p);
                }
            }
        }
        if (type == wheel || type == framed_star) {
            pathStar->close();
            for (int i = edges - 1; i >= 0; --i) {
                double nextOuterAngle = angle + ArtworkGlobal::pi_2 + ArtworkGlobal::twopi / edges * (i + 1.0);
                p.setX(outerRadius * cos(nextOuterAngle));
                p.setY(outerRadius * sin(nextOuterAngle));
                pathStar->lineTo(p);
            }
        }
        pathStar->close();
        pathStar->normalize();

        starShape = pathStar;
    }

    starShape->setFillRule(Qt::OddEvenFill);

    // translate path to center:
    QTransform m;
    m.translate(cx, cy);
    starShape->applyAbsoluteTransformation(m);

    loadCommon(starShape, element);
    loadStyle(starShape, element);
    starShape->setZIndex(nextZIndex());

    return starShape;
}

KShape * ArtworkImport::loadImage(const KXmlElement &element)
{
    QString fname = element.attribute("fname");
    QTransform m(element.attribute("m11", "1.0").toDouble(),
              element.attribute("m12", "0.0").toDouble(), 0,
              element.attribute("m21", "0.0").toDouble(),
              element.attribute("m22", "1.0").toDouble(), 0,
              element.attribute("dx", "0.0").toDouble(),
              element.attribute("dy", "0.0").toDouble(), 1);

    QImage img;
    if (!img.load(fname)) {
        kWarning() << "Could not load image " << fname;
        return 0;
    }

    KImageData * data = m_document->imageCollection()->createImageData(img.mirrored(false, true));
    if (! data)
        return 0;

    KShape * picture = createShape("PictureShape");
    picture->setUserData(data);
    picture->setSize(img.size());
    picture->setTransformation(m);

    loadCommon(picture, element);
    picture->setZIndex(nextZIndex());

    return picture;
}

KShape * ArtworkImport::loadText(const KXmlElement &element)
{
    QFont font;
    font.setFamily(element.attribute("family", "Times"));
    font.setPointSize(element.attribute("size", "12").toInt());
    font.setItalic(element.attribute("italic").toInt() == 1);
    font.setWeight(QFont::Normal);
    font.setBold(element.attribute("bold").toInt() == 1);

    enum Position { Above, On, Under };

    //int position = element.attribute( "position", "0" ).toInt();
    int alignment = element.attribute("alignment", "0").toInt();
    /* TODO reactivate when we have a shadow implementation
    bool shadow = ( element.attribute( "shadow" ).toInt() == 1 );
    bool translucentShadow = ( element.attribute( "translucentshadow" ).toInt() == 1 );
    int shadowAngle = element.attribute( "shadowangle" ).toInt();
    int shadowDistance = element.attribute( "shadowdist" ).toInt();
    double offset = element.attribute( "offset" ).toDouble();
    */
    QString text = element.attribute("text", "");

    ArtisticTextShape * textShape = new ArtisticTextShape();
    if (! textShape)
        return 0;

    textShape->setFont(font);
    textShape->setText(text);
    textShape->setTextAnchor(static_cast<ArtisticTextShape::TextAnchor>(alignment));

    KXmlElement e = element.firstChild().toElement();
    if (e.tagName() == "PATH") {
        // as long as there is no text on path support, just try to get a transformation
        // if the path is only a single line
        KPathShape * path = dynamic_cast<KPathShape*>(loadPath(e));
        if (path) {
            QTransform matrix = path->absoluteTransformation(0);
            QPainterPath outline = matrix.map(path->outline());
            qreal outlineLength = outline.length();
            qreal textLength = textShape->size().width();
            qreal diffLength = textLength - outlineLength;
            if (diffLength > 0.0) {
                // elongate path so that text fits completely on it
                int subpathCount = path->subpathCount();
                int subpathPointCount = path->subpathPointCount(subpathCount - 1);
                KPathPoint * lastPoint = path->pointByIndex(KoPathPointIndex(subpathCount - 1, subpathPointCount - 1));
                KPathPoint * prevLastPoint = path->pointByIndex(KoPathPointIndex(subpathCount - 1, subpathPointCount - 2));
                if (lastPoint && prevLastPoint) {
                    QPointF tangent;
                    if (lastPoint->activeControlPoint1())
                        tangent = matrix.map(lastPoint->point()) - matrix.map(lastPoint->controlPoint1());
                    else if (prevLastPoint->activeControlPoint2())
                        tangent = matrix.map(lastPoint->point()) - matrix.map(prevLastPoint->controlPoint2());
                    else
                        tangent = matrix.map(lastPoint->point()) - matrix.map(prevLastPoint->point());

                    // normalize tangent vector
                    qreal tangentLength = sqrt(tangent.x() * tangent.x() + tangent.y() * tangent.y());
                    tangent /= tangentLength;
                    path->lineTo(matrix.inverted().map(matrix.map(lastPoint->point()) + diffLength * tangent));
                    path->normalize();
                    outline = path->absoluteTransformation(0).map(path->outline());
                }
            }
            textShape->putOnPath(outline);
            textShape->setStartOffset(element.attribute("offset").toDouble());
            delete path;
        }
    }

    loadCommon(textShape, element);
    loadStyle(textShape, element);
    textShape->setZIndex(nextZIndex());
    textShape->applyAbsoluteTransformation(m_mirrorMatrix.inverted());

    return textShape;
}

int ArtworkImport::nextZIndex()
{
    static int zIndex = 0;

    return zIndex++;
}

KShape * ArtworkImport::createShape(const QString &shapeID) const
{
    KShapeFactoryBase * factory = KShapeRegistry::instance()->get(shapeID);
    if (! factory) {
        kWarning() << "Could not find factory for shape id" << shapeID;
        return 0;
    }

    KShape * shape = factory->createDefaultShape(m_document->resourceManager());
    if (shape && shape->shapeId().isEmpty())
        shape->setShapeId(factory->id());

    KPathShape * path = dynamic_cast<KPathShape*>(shape);
    if (path && shapeID == KoPathShapeId)
        path->clear();
    // reset tranformation that might come from the default shape
    shape->setTransformation(QTransform());

    return shape;
}
