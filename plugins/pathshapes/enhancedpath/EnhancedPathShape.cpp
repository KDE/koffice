/* This file is part of the KDE project
 * Copyright (C) 2007,2010 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2009-2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Carlos Licea <carlos@kdab.com>
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *   Contact: Suresh Chande suresh.chande@nokia.com
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

#include "EnhancedPathShape.h"
#include "EnhancedPathCommand.h"
#include "EnhancedPathParameter.h"
#include "EnhancedPathHandle.h"
#include "EnhancedPathFormula.h"

#include <KOdfXmlNS.h>
#include <KXmlWriter.h>
#include <KXmlReader.h>
#include <KShapeSavingContext.h>
#include <KTextOnShapeContainer.h>
#include <KUnit.h>
#include <KOdfWorkaround.h>
#include <KPathPoint.h>

EnhancedPathShape::EnhancedPathShape(const QRectF &viewBox)
: m_viewBox(viewBox), m_viewBoxOffset(0.0, 0.0), m_mirrorVertically(false), m_mirrorHorizontally(false)
{
}

EnhancedPathShape::~EnhancedPathShape()
{
    reset();
}

void EnhancedPathShape::reset()
{
    qDeleteAll(m_commands);
    m_commands.clear();
    qDeleteAll(m_enhancedHandles);
    m_enhancedHandles.clear();
    setHandles(QList<QPointF>());
    qDeleteAll(m_formulae);
    m_formulae.clear();
    qDeleteAll(m_parameters);
    m_parameters.clear();
    m_modifiers.clear();
    m_viewMatrix.reset();
    m_viewBoxOffset = QPointF();
    clear();
}

void EnhancedPathShape::moveHandleAction(int handleId, const QPointF & point, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(modifiers);
    EnhancedPathHandle *handle = m_enhancedHandles[ handleId ];
    if (handle) {
        handle->changePosition(shapeToViewbox(point));
    }
}

void EnhancedPathShape::updatePath(const QSizeF &)
{
    clear();

    foreach (EnhancedPathCommand *cmd, m_commands)
        cmd->execute();

    m_viewBound = outline().boundingRect();

    m_mirrorMatrix.reset();
    m_mirrorMatrix.translate(m_viewBound.center().x(), m_viewBound.center().y());
    m_mirrorMatrix.scale(m_mirrorHorizontally ? -1 : 1, m_mirrorVertically ? -1 : 1);
    m_mirrorMatrix.translate(-m_viewBound.center().x(), -m_viewBound.center().y());

    QTransform matrix;
    matrix.translate(m_viewBoxOffset.x(), m_viewBoxOffset.y());
    matrix = m_mirrorMatrix * m_viewMatrix * matrix;

    KoSubpathList::const_iterator pathIt(m_subpaths.constBegin());
    for (; pathIt != m_subpaths.constEnd(); ++pathIt) {
        KoSubpath::const_iterator it((*pathIt)->constBegin());
        for (; it != (*pathIt)->constEnd(); ++it) {
            (*it)->map(matrix);
        }
    }
    const int handleCount = m_enhancedHandles.count();
    QList<QPointF> handles;
    for (int i = 0; i < handleCount; ++i)
        handles.append(matrix.map(m_enhancedHandles[i]->position()));
    setHandles(handles);

    normalize();
}

void EnhancedPathShape::setSize(const QSizeF &newSize)
{
    // handle offset
    KParameterShape::setSize(newSize);

    // calculate scaling factors from viewbox size to shape size
    qreal xScale = m_viewBound.width() == 0 ? 1 : newSize.width()/m_viewBound.width();
    qreal yScale = m_viewBound.height() == 0 ? 1 : newSize.height()/m_viewBound.height();

    // create view matrix, take mirroring into account
    m_viewMatrix.reset();
    m_viewMatrix.scale(xScale, yScale);

    updatePath(newSize);
}

QPointF EnhancedPathShape::normalize()
{
    QPointF offset = KParameterShape::normalize();

    m_viewBoxOffset -= offset;

    return offset;
}

QPointF EnhancedPathShape::shapeToViewbox(const QPointF &point) const
{
    return (m_mirrorMatrix * m_viewMatrix).inverted().map( point-m_viewBoxOffset );
}

void EnhancedPathShape::evaluateHandles()
{
    const int handleCount = m_enhancedHandles.count();
    QList<QPointF> handles;
    for (int i = 0; i < handleCount; ++i)
        handles.append(m_enhancedHandles[i]->position());
    setHandles(handles);
}

QRectF EnhancedPathShape::viewBox() const
{
    return m_viewBox;
}

qreal EnhancedPathShape::evaluateReference(const QString &reference)
{
    if (reference.isEmpty())
        return 0.0;

    QChar c = reference[0];

    qreal res = 0.0;

    switch(c.toAscii()) {
    // referenced modifier
    case '$': {
        bool success = false;
        int modifierIndex = reference.mid(1).toInt(&success);
        res = m_modifiers.value(modifierIndex);
        break;
    }
    // referenced formula
    case '?': {
        QString fname = reference.mid(1);
        FormulaStore::const_iterator formulaIt = m_formulae.constFind(fname);
        if (formulaIt != m_formulae.constEnd())
        {
            EnhancedPathFormula * formula = formulaIt.value();
            if (formula)
                res = formula->evaluate();
        }
        break;
    }
    // maybe an identifier ?
    default:
        EnhancedPathNamedParameter p(reference, this);
        res = p.evaluate();
        break;
    }

    return res;
}

void EnhancedPathShape::modifyReference(const QString &reference, qreal value)
{
    if (reference.isEmpty())
        return;

    QChar c = reference[0];

    if (c.toAscii() == '$') {
        bool success = false;
        int modifierIndex = reference.mid(1).toInt(&success);
        if (modifierIndex >= 0 && modifierIndex < m_modifiers.count())
            m_modifiers[modifierIndex] = value;
    }
}

EnhancedPathParameter * EnhancedPathShape::parameter(const QString & text)
{
    Q_ASSERT(! text.isEmpty());

    ParameterStore::const_iterator parameterIt = m_parameters.constFind(text);
    if (parameterIt != m_parameters.constEnd()) {
        return parameterIt.value();
    } else {
        EnhancedPathParameter *parameter = 0;
        QChar c = text[0];
        if (c.toAscii() == '$' || c.toAscii() == '?') {
            parameter = new EnhancedPathReferenceParameter(text, this);
        } else {
            if (c.isDigit()) {
                bool success = false;
                qreal constant = text.toDouble(&success);
                if (success)
                    parameter = new EnhancedPathConstantParameter(constant, this);
            } else {
                Identifier identifier = EnhancedPathNamedParameter::identifierFromString(text);
                if (identifier != IdentifierUnknown)
                    parameter = new EnhancedPathNamedParameter(identifier, this);
            }
        }

        if (parameter)
            m_parameters[text] = parameter;

        return parameter;
    }
}

void EnhancedPathShape::addFormula(const QString &name, const QString &formula)
{
    if (name.isEmpty() || formula.isEmpty())
        return;

    m_formulae[name] = new EnhancedPathFormula(formula, this);
}

void EnhancedPathShape::addHandle(const QMap<QString,QVariant> &handle)
{
    if (handle.isEmpty())
        return;

    if (! handle.contains("draw:handle-position"))
        return;
    QVariant position = handle.value("draw:handle-position");

    QStringList tokens = position.toString().simplified().split(' ');
    if (tokens.count() < 2)
        return;

    EnhancedPathHandle *newHandle = new EnhancedPathHandle(this);
    newHandle->setPosition(parameter(tokens[0]), parameter(tokens[1]));

    // check if we have a polar handle
    if (handle.contains("draw:handle-polar")) {
        QVariant polar = handle.value("draw:handle-polar");
        QStringList tokens = polar.toString().simplified().split(' ');
        if (tokens.count() == 2) {
            newHandle->setPolarCenter(parameter(tokens[0]), parameter(tokens[1]));

            QVariant minRadius = handle.value("draw:handle-radius-range-minimum");
            QVariant maxRadius = handle.value("draw:handle-radius-range-maximum");
            if (minRadius.isValid() && maxRadius.isValid())
                newHandle->setRadiusRange(parameter(minRadius.toString()), parameter(maxRadius.toString()));
        }
    } else {
        QVariant minX = handle.value("draw:handle-range-x-minimum");
        QVariant maxX = handle.value("draw:handle-range-x-maximum");
        if (minX.isValid() && maxX.isValid())
            newHandle->setRangeX(parameter(minX.toString()), parameter(maxX.toString()));

        QVariant minY = handle.value("draw:handle-range-y-minimum");
        QVariant maxY = handle.value("draw:handle-range-y-maximum");
        if (minY.isValid() && maxY.isValid())
            newHandle->setRangeY(parameter(minY.toString()), parameter(maxY.toString()));
    }

    m_enhancedHandles.append(newHandle);

    evaluateHandles();
}

void EnhancedPathShape::addModifiers(const QString &modifiers)
{
    if (modifiers.isEmpty())
        return;

    QStringList tokens = modifiers.simplified().split(' ');
    int tokenCount = tokens.count();
    for (int i = 0; i < tokenCount; ++i)
       m_modifiers.append(tokens[i].toDouble());
}

void EnhancedPathShape::addCommand(const QString &command)
{
    addCommand(command, true);
}

void EnhancedPathShape::addCommand(const QString &command, bool triggerUpdate)
{
    QString commandStr = command.simplified();
    if (commandStr.isEmpty())
        return;

    // the first character is the command
    EnhancedPathCommand *cmd = new EnhancedPathCommand(commandStr[0], this);

    // strip command char
    commandStr = commandStr.mid(1).simplified();

    // now parse the command parameters
    if (!commandStr.isEmpty()) {
        QStringList tokens = commandStr.split(' ');
        for (int i = 0; i < tokens.count(); ++i)
            cmd->addParameter(parameter(tokens[i]));
    }
    m_commands.append(cmd);

    if (triggerUpdate)
        updatePath(size());
}

void EnhancedPathShape::saveOdf(KShapeSavingContext &context) const
{
    if (isParametricShape()) {
        context.xmlWriter().startElement("draw:custom-shape");
        saveOdfAttributes(context, OdfAllAttributes&~OdfSize);

        // save the right size so that when loading we fit the viewbox
        // to the right size without getting any wrong scaling
        // -> calculate the right size from the current size/viewbound ratio
        const QSizeF currentSize = outline().boundingRect().size();
        context.xmlWriter().addAttributePt("svg:width", m_viewBox.width()*currentSize.width()/m_viewBound.width());
        context.xmlWriter().addAttributePt("svg:height", m_viewBox.height()*currentSize.height()/m_viewBound.height());

        if (parent())
            parent()->saveOdfChildElements(context);
        
        context.xmlWriter().startElement("draw:enhanced-geometry");
        context.xmlWriter().addAttribute("svg:viewBox", QString("%1 %2 %3 %4").arg(m_viewBox.x()).arg(m_viewBox.y()).arg(m_viewBox.width()).arg(m_viewBox.height()));

        if (m_mirrorHorizontally) {
            context.xmlWriter().addAttribute("draw:mirror-horizontal", "true");
        }
        if (m_mirrorVertically) {
            context.xmlWriter().addAttribute("draw:mirror-vertical", "true");
        }
        
        QString modifiers;
        foreach (qreal modifier, m_modifiers)
            modifiers += QString::number(modifier) + ' ';
        context.xmlWriter().addAttribute("draw:modifiers", modifiers.trimmed());

        QString path;
        foreach (EnhancedPathCommand * c, m_commands)
            path += c->toString() + ' ';
        context.xmlWriter().addAttribute("draw:enhanced-path", path.trimmed());

        FormulaStore::const_iterator i = m_formulae.constBegin();
        for (; i != m_formulae.constEnd(); ++i) {
            context.xmlWriter().startElement("draw:equation");
            context.xmlWriter().addAttribute("draw:name", i.key());
            context.xmlWriter().addAttribute("draw:formula", i.value()->toString());
            context.xmlWriter().endElement(); // draw:equation
        }

        foreach (EnhancedPathHandle * handle, m_enhancedHandles)
            handle->saveOdf(context);

        context.xmlWriter().endElement(); // draw:enhanced-geometry
        saveOdfCommonChildElements(context);
        context.xmlWriter().endElement(); // draw:custom-shape

    } else {
        KPathShape::saveOdf(context);
    }
}

bool EnhancedPathShape::loadOdf(const KXmlElement & element, KShapeLoadingContext &context)
{
    reset();
    loadOdfAttributes(element, context, OdfAdditionalAttributes | OdfCommonChildElements);

    const KXmlElement enhancedGeometry(KoXml::namedItemNS(element, KOdfXmlNS::draw, "enhanced-geometry" ) );
    if (!enhancedGeometry.isNull()) {
        // load the modifiers
        QString modifiers = enhancedGeometry.attributeNS(KOdfXmlNS::draw, "modifiers");
        if (!modifiers.isEmpty()) {
            addModifiers(modifiers);
        }

        KXmlElement grandChild;
        forEachElement(grandChild, enhancedGeometry) {
            if (grandChild.namespaceURI() != KOdfXmlNS::draw)
                continue;
            if (grandChild.localName() == "equation") {
                QString name = grandChild.attributeNS(KOdfXmlNS::draw, "name");
                QString formula = grandChild.attributeNS(KOdfXmlNS::draw, "formula");
                addFormula(name, formula);
            } else if (grandChild.localName() == "handle") {
                EnhancedPathHandle * handle = new EnhancedPathHandle(this);
                if (handle->loadOdf(grandChild, context)) {
                    m_enhancedHandles.append(handle);
                    evaluateHandles();
                } else {
                    delete handle;
                }
            }
        }

        setMirrorHorizontally(enhancedGeometry.attributeNS(KOdfXmlNS::draw, "mirror-horizontal") == "true");
        setMirrorVertically(enhancedGeometry.attributeNS(KOdfXmlNS::draw, "mirror-vertical") == "true");

        // load the enhanced path data
        QString path = enhancedGeometry.attributeNS(KOdfXmlNS::draw, "enhanced-path");
#ifndef NWORKAROUND_ODF_BUGS
        KOdfWorkaround::fixEnhancedPath(path, enhancedGeometry, context);
#endif
        if (!path.isEmpty()) {
            parsePathData(path);
        }

        // load the viewbox
        QRectF viewBox = loadOdfViewbox(enhancedGeometry);
        if (! viewBox.isEmpty()) {
            m_viewBox = viewBox;
        } else {
            // if there is no view box defined make it is big as the path.
            m_viewBox = m_viewBound;
        }

        QString gluePoints(enhancedGeometry.attributeNS(KOdfXmlNS::draw, "glue-points"));
        if (!gluePoints.isEmpty()) {
            qreal x = -1;
            foreach (const QString &token, gluePoints.split(' ')) {
                if (x == -1) { // processing 'x'
                    x = token.toInt();
                } else {
                    addConnectionPoint(QPointF(x, token.toInt()));
                    x = -1;
                }
            }
        }
    }

    QSizeF size;
    size.setWidth(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "width")));
    size.setHeight(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "height")));
    // the viewbox is to be fitted into the size of the shape, so before setting
    // the size we just loaded // we set the viewbox to be the basis to calculate
    // the viewbox matrix from
    m_viewBound = m_viewBox;
    setSize(size);

    QPointF pos;
    pos.setX(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "x")));
    pos.setY(KUnit::parseValue(element.attributeNS(KOdfXmlNS::svg, "y")));
    setPosition(pos - m_viewMatrix.map(QPointF(0, 0)) - m_viewBoxOffset);

    loadOdfAttributes(element, context, OdfMandatories | OdfTransformation);

    KTextOnShapeContainer::tryWrapShape(this, element, context);

    return true;
}

void EnhancedPathShape::parsePathData(const QString &data)
{
    if (data.isEmpty())
        return;

    int start = -1;
    bool separator = true;
    for (int i = 0; i < data.length(); ++i) {
        QChar ch = data.at(i);
        ushort uni_ch = ch.unicode();
        if (separator && (uni_ch == 'M' || uni_ch == 'L'
            || uni_ch == 'C' || uni_ch == 'Z'
            || uni_ch == 'N' || uni_ch == 'F'
            || uni_ch == 'S' || uni_ch == 'T'
            || uni_ch == 'U' || uni_ch == 'A'
            || uni_ch == 'B' || uni_ch == 'W'
            || uni_ch == 'V' || uni_ch == 'X'
            || uni_ch == 'Y' || uni_ch == 'Q')) {
            if (start != -1) { // process last chars
                addCommand(data.mid(start, i - start), false);
            }
            start = i;
        }
        separator = ch.isSpace();
    }
    if (start < data.length())
        addCommand(data.mid(start));
    if (start != -1)
        updatePath(size());
}

void EnhancedPathShape::setMirrorHorizontally(bool mirrorHorizontally)
{
    if ( m_mirrorHorizontally != mirrorHorizontally) {
        m_mirrorHorizontally = mirrorHorizontally;
        updatePath(size());
    }

}

void EnhancedPathShape::setMirrorVertically(bool mirrorVertically)
{
    if ( m_mirrorVertically != mirrorVertically) {
        m_mirrorVertically = mirrorVertically;
        updatePath(size());
    }
}
