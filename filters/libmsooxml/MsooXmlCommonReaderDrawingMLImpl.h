/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2009-2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * Contact: Suresh Chande suresh.chande@nokia.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 */

#ifndef MSOOXMLCOMMONREADERDRAWINGML_IMPL_H
#define MSOOXMLCOMMONREADERDRAWINGML_IMPL_H

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535897932384626
#endif

#if !defined DRAWINGML_NS && !defined NO_DRAWINGML_NS
#error missing DRAWINGML_NS define!
#endif
#if !defined DRAWINGML_PIC_NS && !defined NO_DRAWINGML_PIC_NS
#error missing DRAWINGML_PIC_NS define!
#endif

#undef MSOOXML_CURRENT_NS
#ifndef NO_DRAWINGML_PIC_NS
#define MSOOXML_CURRENT_NS DRAWINGML_PIC_NS
#endif

#include <KoXmlWriter.h>
#include <MsooXmlUnits.h>
#include "Charting.h"
#include "ChartExport.h"
#include "XlsxXmlChartReader.h"

#include <MsooXmlReader.h>
#include <MsooXmlCommonReader.h>
#include <QScopedPointer>

// ================================================================


void MSOOXML_CURRENT_CLASS::initDrawingML()
{
    m_currentDoubleValue = 0;
    m_hyperLink = false;
    m_listStylePropertiesAltered = false;
    m_inGrpSpPr = false;
    m_fillImageRenderingStyleStretch = false;
}

// ----------------------------------------------------------------

KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::copyFile(const QString& sourceName,
                                                           const QString& destinationDir,
                                                           QString& destinationName,
                                                           bool oleType)
{
    destinationName = destinationDir + sourceName.mid(sourceName.lastIndexOf('/') + 1);
    if (oleType) {
        // If it's of type ole, we don't know the file type, by default it has .bin
        // ending which we're removing here.
        destinationName.remove(".bin");
    }

    if (m_copiedFiles.contains(destinationName)) {
        kDebug() << sourceName << "already copied - skipping";
    }
    else {
//! @todo should we check name uniqueness here in case the sourceName can be located in various directories?
        RETURN_IF_ERROR( m_context->import->copyFile(sourceName, destinationName, oleType) )
        addManifestEntryForFile(destinationName);
        m_copiedFiles.insert(destinationName);
    }
    return KoFilter::OK;
}

// ================================================================
// DrawingML tags
// ================================================================

static QString mirrorToOdf(bool flipH, bool flipV)
{
    if (!flipH && !flipV)
        return QString();
    if (flipH && flipV)
        return QLatin1String("horizontal vertical");
    if (flipH)
        return QLatin1String("horizontal");
    if (flipV)
        return QLatin1String("vertical");
    return QLatin1String("none");
}

#undef CURRENT_EL
#define CURRENT_EL pic
//! pic handler (Picture)
/*! ECMA-376, 19.3.1.37, p. 2848; 20.1.2.2.30, p.3049 - DrawingML
 This element specifies the existence of a picture object within the document.
*/
//! @todo use it in DrawingML too: ECMA-376, 20.2.2.5, p. 3463
/*!
 Parent elements:
 - control (§19.3.2.1)
 - grpSp (§19.3.1.22)
 - grpSp (§20.1.2.2.20) - DrawingML
 - lockedCanvas (§20.3.2.1) - DrawingML
 - oleObj (§19.3.2.4)
 - [done] spTree (§19.3.1.45)

 Child elements:
 - [done] blipFill (Picture Fill) §19.3.1.4
 - [done] blipFill (Picture Fill) §20.1.8.14 - DrawingML
 - extLst (Extension List with Modification Flag) §19.3.1.20
 - extLst (Extension List) §20.1.2.2.15 - DrawingML
 - [done] nvPicPr (Non-Visual Properties for a Picture) §19.3.1.32
 - [done] nvPicPr (Non-Visual Properties for a Picture) §20.1.2.2.28 - DrawingML
 - [done] spPr (Shape Properties) §19.3.1.44
 - [done] spPr (Shape Properties) §20.1.2.2.35 - DrawingML
 - style (Shape Style) §19.3.1.46
 - style (Shape Style) §20.1.2.2.37 - DrawingML
*/
//! @todo support all elements
//! CASE #P401
//! @todo CASE #P421
//! CASE #P422
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_pic()
{
    READ_PROLOGUE

    // Reset picture properties
    m_xlinkHref.clear();
    m_hasPosOffsetH = false;
    m_hasPosOffsetV = false;
    m_posOffsetH = 0;
    m_posOffsetV = 0;
    m_cNvPrId.clear();
    m_cNvPrName.clear();
    m_cNvPrDescr.clear();
    m_fillImageRenderingStyleStretch = false;
    m_flipH = false;
    m_flipV = false;
    m_rot = 0;
    m_isPlaceHolder = false;

    // Create a new drawing style for this picture
    pushCurrentDrawStyle(new KoGenStyle(KoGenStyle::GraphicAutoStyle, "graphic"));

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(spPr)
            ELSE_TRY_READ_IF_IN_CONTEXT(blipFill)
            ELSE_TRY_READ_IF(nvPicPr)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

#ifdef PPTXXMLSLIDEREADER_H
    // Ooxml supports slides getting pictures from layout slide, this is not supported in odf
    // Therefore we are buffering the picture frames from layout and using them later in the slide
    QBuffer picBuf;
    KoXmlWriter picWriter(&picBuf);
    KoXmlWriter *bodyBackup = body;

    if (m_context->type == SlideLayout) {
        body = &picWriter;
    }
#endif

    body->startElement("draw:frame"); // CASE #P421
#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == Slide || m_context->type == SlideLayout) {
        body->addAttribute("draw:layer", "layout");
    }
    else { // Slidemaster
        body->addAttribute("draw:layer", "backgroundobjects");
    }
    body->addAttribute("presentation:user-transformed", MsooXmlReader::constTrue);
#endif
//todo        body->addAttribute("presentation:style-name", styleName);
//! @todo for pptx: maybe use KoGenStyle::PresentationAutoStyle?
    if (m_noFill) {
        m_currentDrawStyle->addAttribute("style:fill", constNone);
    }

#ifdef DOCXXMLDOCREADER_H
    //QString currentDrawStyleName(mainStyles->insert(*m_currentDrawStyle, "gr"));
#endif
#if defined(DOCXXMLDOCREADER_H)
    //kDebug() << "currentDrawStyleName:" << currentDrawStyleName;
    //body->addAttribute("draw:style-name", currentDrawStyleName);
#endif

//! @todo CASE #1341: images within w:hdr should be anchored as paragraph (!wp:inline) or as-char (wp:inline)
    if (m_drawing_inline) {
        body->addAttribute("text:anchor-type", "as-char");
    }
    else {
        body->addAttribute("text:anchor-type", "char");
    }
    if (!m_docPrName.isEmpty()) { // from docPr/@name
        body->addAttribute("draw:name", m_docPrName);
    }
//! @todo add more cases for text:anchor-type! use m_drawing_inline and see CASE #1343
    int realX = m_svgX;
    int realY = m_svgY;
    if (m_hasPosOffsetH) {
        kDebug() << "m_posOffsetH" << m_posOffsetH;
        realX += m_posOffsetH;
    }
    if (m_hasPosOffsetV) {
        kDebug() << "m_posOffsetV" << m_posOffsetV;
        realY += m_posOffsetV;
    }
    if (m_rot == 0) {
        body->addAttribute("svg:x", EMU_TO_CM_STRING(realX));
        body->addAttribute("svg:y", EMU_TO_CM_STRING(realY));
    }
    body->addAttribute("svg:width", EMU_TO_CM_STRING(m_svgWidth));
    body->addAttribute("svg:height", EMU_TO_CM_STRING(m_svgHeight));

    if (m_rot != 0) {
        // m_rot is in 1/60,000th of a degree
        qreal angle, xDiff, yDiff;
        MSOOXML::Utils::rotateString(m_rot, m_svgWidth, m_svgHeight, angle, xDiff, yDiff, m_flipH, m_flipV);
        QString rotString = QString("rotate(%1) translate(%2cm %3cm)")
                            .arg(angle).arg((m_svgX + xDiff)/360000).arg((m_svgY + yDiff)/360000);
        body->addAttribute("draw:transform", rotString);
    }

    const QString styleName(mainStyles->insert(*m_currentDrawStyle, "gr"));
#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideMaster) {
        mainStyles->markStyleForStylesXml(styleName);
    }
#endif
    body->addAttribute("draw:style-name", styleName);

    // Now it's time to link to the actual picture.  Only do it if
    // there is an image to link to.  If so, this was created in
    // read_blip().
    if (!m_xlinkHref.isEmpty()) {
        body->startElement("draw:image");
        body->addAttribute("xlink:href", m_xlinkHref);
        //! @todo xlink:type?
        body->addAttribute("xlink:type", "simple");
        //! @todo xlink:show?
        body->addAttribute("xlink:show", "embed");
        //! @todo xlink:actuate?
        body->addAttribute("xlink:actuate", "onLoad");
        body->endElement(); //draw:image
#ifdef DOCXXMLDOCREADER_H
        if (!m_cNvPrName.isEmpty() || !m_cNvPrDescr.isEmpty()) {
            body->startElement("svg:title");
            body->addTextSpan(m_cNvPrDescr.isEmpty() ? m_cNvPrName : m_cNvPrDescr);
            body->endElement(); //svg:title
        }
#endif
        m_xlinkHref.clear();
    }

    // Add style information
    //! @todo: horizontal-on-{odd,even}?
    const QString mirror(mirrorToOdf(m_flipH, m_flipV));
    if (!mirror.isEmpty()) {
        m_currentDrawStyle->addProperty("style:mirror", mirror);
    }

    body->endElement(); //draw:frame

#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideLayout) {
        if (!d->phRead) {
            const QString elementContents = QString::fromUtf8(picBuf.buffer(), picBuf.buffer().size());
            m_context->slideLayoutProperties->layoutFrames.push_back(elementContents);
        }
        body = bodyBackup;
    }
#endif

    popCurrentDrawStyle();

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL nvPicPr
//! nvPicPr handler (Non-Visual Properties for a Picture)
//! ECMA-376, 19.3.1.32, p. 2845; 20.1.2.2.28, p. 3048 - DrawingML
/*! This element specifies all non-visual properties for a picture.

 Parent elements:
    - [done] pic (§19.3.1.37)
    - [done] pic (§20.1.2.2.30) - DrawingML
 Child elements:
    - cNvPicPr (Non-Visual Picture Drawing Properties) §19.3.1.11
    - [done] cNvPicPr (Non-Visual Picture Drawing Properties) §20.1.2.2.7 - DrawingML
    - [done] cNvPr (Non-Visual Drawing Properties) §19.3.1.12
    - [done] cNvPr (Non-Visual Drawing Properties) §20.1.2.2.8 - DrawingML
    - [done] nvPr (Non-Visual Properties) §19.3.1.33
*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_nvPicPr()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(cNvPicPr)
            ELSE_TRY_READ_IF_IN_CONTEXT(cNvPr)
#ifdef PPTXXMLSLIDEREADER_H
            ELSE_TRY_READ_IF(nvPr) // only §19.3.1.33
#endif
            ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL cNvPicPr
//! cNvPicPr handler (Non-Visual Picture Drawing Properties)
//! ECMA-376, 19.3.1.11, p. 2825; 20.1.2.2.7, p. 3027 - DrawingML
/*! This element specifies the non-visual properties for the picture canvas.
 These properties are to be used by the generating application to determine
 how certain properties are to be changed for the picture object in question.

 Parent elements:
    - [done] nvPicPr (§19.3.1.32)
    - [done] nvPicPr (§20.1.2.2.28) - DrawingML
 Child elements:
    - extLst (Extension List) §20.1.2.2.15
    - picLocks (Picture Locks) §20.1.2.2.31
 Attributes:
    - preferRelativeResize (Relative Resize Preferred)
*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_cNvPicPr()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
//! @todo add ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL cNvPr
//! cNvPr handler (Non-Visual Drawing Properties)
//! ECMA-376, 19.3.1.12, p. 2826; 20.1.2.2.8, p. 3028 - DrawingML
/*! This element specifies non-visual canvas properties.
 This allows for additional information that does not affect
 the appearance of the picture to be stored.

 Parent elements:
    - nvCxnSpPr (§19.3.1.29)
    - nvCxnSpPr (§20.1.2.2.25) - DrawingML
    - nvGraphicFramePr (§19.3.1.30)
    - nvGraphicFramePr (§20.1.2.2.26) - DrawingML
    - nvGrpSpPr (§19.3.1.31)
    - nvGrpSpPr (§20.1.2.2.27) - DrawingML
    - [done] nvPicPr (§19.3.1.32)
    - [done] nvPicPr (§20.1.2.2.28) - DrawingML
    - [done] nvSpPr (§19.3.1.34)
    - [done] nvSpPr (§20.1.2.2.29) - DrawingML
 Child elements:
    - extLst (Extension List) §20.1.2.2.15
    - hlinkClick (Click Hyperlink) §21.1.2.3.5
    - hlinkHover (Hyperlink for Hover) §20.1.2.2.23
 Attributes:
    - [done] descr (Alternative Text for Object)
    - hidden (Hidden)
    - [done] id (Unique Identifier)
    - [done] name (Name)
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_cNvPr(cNvPrCaller caller)
{
    READ_PROLOGUE

    m_cNvPrId.clear();
    m_cNvPrName.clear();
    m_cNvPrDescr.clear();
    const QXmlStreamAttributes attrs(attributes());
    if (caller == cNvPr_nvSpPr || caller == cNvPr_nvPicPr) { // for sanity, p:nvGrpSpPr can be also the caller
        READ_ATTR_WITHOUT_NS_INTO(id, m_cNvPrId)
        kDebug() << "id:" << m_cNvPrId;
        TRY_READ_ATTR_WITHOUT_NS_INTO(name, m_cNvPrName)
        kDebug() << "name:" << m_cNvPrName;
        TRY_READ_ATTR_WITHOUT_NS_INTO(descr, m_cNvPrDescr)
        kDebug() << "descr:" << m_cNvPrDescr;
    }

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
//            TRY_READ_IF()
//! @todo add ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL nvSpPr
//! nvSpPr handler (Non-Visual Properties for a Shape)
//! ECMA-376, 19.3.1.34, p. 2846; 20.1.2.2.29, p. 3049 - DrawingML.
/*! This element specifies all non-visual properties for a shape.
 This element is a container for the non-visual identification properties,
 shape properties and application properties that are to be associated with a shape.
 This allows for additional information that does not affect the appearance of the shape to be stored.

 Parent elements:
    - [done] sp (§19.3.1.43)
    - [done] sp (§20.1.2.2.33)
  Child elements:
    - [done] cNvPr (Non-Visual Drawing Properties) §19.3.1.12
    - [done] cNvPr (Non-Visual Drawing Properties) §20.1.2.2.8 - DrawingML
    - [done] cNvSpPr (Non-Visual Drawing Properties for a Shape) §19.3.1.13
    - [done] cNvSpPr (Non-Visual Drawing Properties for a Shape) §20.1.2.2.9 - DrawingML
    - [done] nvPr (Non-Visual Properties) §19.3.1.33
*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_nvSpPr()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF_IN_CONTEXT(cNvPr)
#ifdef PPTXXMLSLIDEREADER_H
            ELSE_TRY_READ_IF(nvPr) // only §19.3.1.33
#endif
            ELSE_TRY_READ_IF(cNvSpPr)
            ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL grpSp
//! grpSp handler (Group shape)
/*!
 Parent elements:
 - [done] grpSp (§19.3.1.22);
 - [done] spTree (§19.3.1.45)

 Child elements:
 - contentPart (Content Part) §19.3.1.14
 - cxnSp (Connection Shape) §19.3.1.19
 - extLst (Extension List with Modification Flag) §19.3.1.20
 - [done] graphicFrame (Graphic Frame) §19.3.1.21
 - [done] grpSp (Group Shape) §19.3.1.22
 - [done] grpSpPr (Group Shape Properties) §19.3.1.23
 - nvGrpSpPr (Non-Visual Properties for a Group Shape) §19.3.1.31
 - [done] pic (Picture) §19.3.1.37
 - [done] sp (Shape) §19.3.1.43
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_grpSp()
{
    READ_PROLOGUE

    body->startElement("draw:g");

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        kDebug() << *this;
        if (isStartElement()) {
            TRY_READ_IF(grpSp)
            ELSE_TRY_READ_IF(pic)
            ELSE_TRY_READ_IF(sp)
            ELSE_TRY_READ_IF(grpSpPr)
#ifdef PPTXXMLSLIDEREADER_H
            ELSE_TRY_READ_IF(graphicFrame)
#endif
        //! @todo add ELSE_WRONG_FORMAT
        }
    }
    body->endElement(); // draw:g

    // Properties are set in grpSpPr
    m_svgProp.pop_back();

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL grpSpPr
//! grpSp handler (Group shape properties)
/*!
 Parent elements:
 - [done] grpSp (§19.3.1.22);
 - spTree (§19.3.1.45)

 Child elements:
 - blipFill (Picture Fill) §20.1.8.14
 - effectDag (Effect Container) §20.1.8.25
 - effectLst (Effect Container) §20.1.8.26
 - extLst (Extension List) §20.1.2.2.15
 - gradFill (Gradient Fill) §20.1.8.33
 - grpFill (Group Fill) §20.1.8.35
 - noFill (No Fill) §20.1.8.44
 - pattFill (Pattern Fill) §20.1.8.47
 - scene3d (3D Scene Properties) §20.1.4.1.26
 - solidFill (Solid Fill) §20.1.8.54
 - [done] xfrm (2D Transform for Grouped Objects) §20.1.7.5
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_grpSpPr()
{
    READ_PROLOGUE

    m_inGrpSpPr = true;

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF_NS(a, xfrm)
        //! @todo add ELSE_WRONG_FORMAT
        }
    }

    m_inGrpSpPr = false;

    GroupProp prop;
    prop.svgXOld = m_svgX;
    prop.svgYOld = m_svgY;
    prop.svgWidthOld = m_svgWidth;
    prop.svgHeightOld = m_svgHeight;
    prop.svgXChOld = m_svgChX;
    prop.svgYChOld = m_svgChY;
    prop.svgWidthChOld = m_svgChWidth;
    prop.svgHeightChOld = m_svgChHeight;

    m_svgProp.push_back(prop);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL nvCxnSpPr
//! nvCxnSpPr handler (Non visual properties for a connection shape)
//! @todo propertly
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_nvCxnSpPr()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF_IN_CONTEXT(cNvPr)
#ifdef PPTXXMLSLIDEREADER_H
            ELSE_TRY_READ_IF(nvPr) // only §19.3.1.33
#endif
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL cNvSpPr
//! cNvSpPr handler (Non-Visual Drawing Properties for a Shape)
//! ECMA-376, 19.3.1.13, p. 2828; 20.1.2.2.9, p. 3030.
/*! This element specifies the non-visual drawing properties for a shape.

 Parent elements:
    - [done] nvSpPr (§19.3.1.34)
    - [done] nvSpPr (§20.1.2.2.29) - DrawingML
 Child elements:
    - extLst (Extension List) §20.1.2.2.15
    - spLocks (Shape Locks) §20.1.2.2.34
 Attributes:
    - [done] txBox (Text Box)
*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_cNvSpPr()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    // Read attributes
    // FIXME: Make a member?
    //bool isTextBox = MSOOXML::Utils::convertBooleanAttr(attrs.value("txBox").toString(), false);

    // Read child elements
    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        /*        if (isStartElement()) {
                    TRY_READ_IF(...)
        //! @todo add ELSE_WRONG_FORMAT
                }*/
    }
    READ_EPILOGUE
}

void MSOOXML_CURRENT_CLASS::preReadSp()
{
    // Reset the position and size
    m_svgX = 0;
    m_svgY = 0;
    m_svgWidth = -1;
    m_svgHeight = -1;
    m_xfrm_read = false;
    m_flipH = false;
    m_flipV = false;
    m_rot = 0;

#ifdef PPTXXMLSLIDEREADER_H
    //We assume that the textbox is empty by default
    d->textBoxHasContent = false;

    // If called from the pptx converter, handle different contexts
    // (Slide, SlideMaster, SlideLayout)
    if (m_context->type == Slide) {
        m_currentPresentationStyle = KoGenStyle(KoGenStyle::PresentationAutoStyle, "presentation");
    }
    else if (m_context->type == SlideMaster) {
        m_currentShapeProperties = new PptxShapeProperties();
    }
    else if (m_context->type == SlideLayout) {
        // moved down
        m_currentShapeProperties = 0;
    }
    m_isPlaceHolder = false;
    ++d->shapeNumber;
#endif

    m_cNvPrId.clear();
    m_cNvPrName.clear();
    m_cNvPrDescr.clear();
    m_rot = 0;
}

void MSOOXML_CURRENT_CLASS::generateFrameSp()
{
#ifdef PPTXXMLSLIDEREADER_H
    const QString styleId(d->phStyleId());

    kDebug() << "outputDrawFrame for" << (m_context->type == SlideLayout ? "SlideLayout" : "Slide");

    inheritDefaultBodyProperties();
    inheritBodyProperties(); // Properties may or may not override default ones.
#else
#endif
    if (m_contentType == "line") {
        body->startElement("draw:line");
    }
    else {
        body->startElement("draw:frame"); // CASE #P475
    }
    if (!m_cNvPrName.isEmpty()) {
        body->addAttribute("draw:name", m_cNvPrName);
    }

    m_currentDrawStyle->addProperty("draw:textarea-vertical-align", m_shapeTextPosition);
    m_currentDrawStyle->addProperty("fo:margin-left", EMU_TO_CM_STRING(m_shapeTextLeftOff.toInt()));
    m_currentDrawStyle->addProperty("fo:margin-right", EMU_TO_CM_STRING(m_shapeTextRightOff.toInt()));
    m_currentDrawStyle->addProperty("fo:margin-top", EMU_TO_CM_STRING(m_shapeTextTopOff.toInt()));
    m_currentDrawStyle->addProperty("fo:margin-bottom", EMU_TO_CM_STRING(m_shapeTextBottomOff.toInt()));

    const QString styleName(mainStyles->insert(*m_currentDrawStyle, "gr"));

#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideMaster) {
        mainStyles->markStyleForStylesXml(styleName);
    }
#endif
    body->addAttribute("draw:style-name", styleName);

#ifdef PPTXXMLSLIDEREADER_H

    const QString presentationClass(MSOOXML::Utils::ST_PlaceholderType_to_ODF(d->phType));

    if (m_context->type == Slide || m_context->type == SlideLayout) {
        body->addAttribute("draw:layer", "layout");
    }
    else {
        body->addAttribute("draw:layer", "backgroundobjects");
        // StyleID will be empty for any text that is in masterslide that is wanted
        // to be shown in the actual slides, such as company names etc.
        if (!styleId.isEmpty()) {
            body->addAttribute("presentation:placeholder", "true");
            body->addAttribute("presentation:class", presentationClass);
        }
    }

    QString presentationStyleName;
    //body->addAttribute("draw:style-name", );
    if (!m_currentPresentationStyle.isEmpty()) {
        presentationStyleName = mainStyles->insert(m_currentPresentationStyle, "pr");
        if (m_context->type == SlideMaster) {
            mainStyles->markStyleForStylesXml(presentationStyleName);
        }
    }
    if (!presentationStyleName.isEmpty()) {
        body->addAttribute("presentation:style-name", presentationStyleName);
    }

    if (m_context->type == Slide) {
        // CASE #P476
        body->addAttribute("draw:id", m_cNvPrId);
        body->addAttribute("presentation:class", presentationClass);
        kDebug() << "presentationClass:" << d->phType << "->" << presentationClass;
        kDebug() << "m_svgWidth:" << m_svgWidth << "m_svgHeight:" << m_svgHeight
                 << "m_svgX:" << m_svgX << "m_svgY:" << m_svgY;
        PptxPlaceholder *placeholder = m_context->slideLayoutProperties->placeholders.value(presentationClass);
        kDebug() << "m_context->slideLayoutProperties:" << m_context->slideLayoutProperties
                 << QString("m_context->slideLayoutProperties->placeholders.value(\"%1\")")
                   .arg(presentationClass) << placeholder;
        if (!placeholder) {
            kDebug() << "trying placeholder for id:" << d->phIdx;
            placeholder = m_context->slideLayoutProperties->placeholders.value(d->phIdx);
            kDebug() << "m_context->slideLayoutProperties:" << m_context->slideLayoutProperties
                << QString("m_context->slideLayoutProperties->placeholders.value(\"%1\")")
                   .arg(d->phIdx) << placeholder;
        }
        if (!m_xfrm_read && m_context->slideLayoutProperties && placeholder) {
            kDebug() << "Copying attributes from slide layout:" << m_context->slideLayoutProperties->pageLayoutStyleName;
            m_svgX = placeholder->x;
            m_svgY = placeholder->y;
            m_svgWidth = placeholder->width;
            m_svgHeight = placeholder->height;
            m_rot = placeholder->rot;
        }
    }
    if (m_svgWidth > -1 && m_svgHeight > -1) {
        body->addAttribute("presentation:user-transformed", MsooXmlReader::constTrue);
        if (m_contentType == "line") {
            QString y1 = EMU_TO_CM_STRING(m_svgY);
            QString y2 = EMU_TO_CM_STRING(m_svgY + m_svgHeight);
            QString x1 = EMU_TO_CM_STRING(m_svgX);
            QString x2 = EMU_TO_CM_STRING(m_svgX + m_svgWidth);
        if (m_rot != 0) {
            qreal angle, xDiff, yDiff;
            MSOOXML::Utils::rotateString(m_rot, m_svgWidth, m_svgHeight, angle, xDiff, yDiff, m_flipH, m_flipV);
            //! @todo, in case of connector, these should maybe be reversed?
            x1 = EMU_TO_CM_STRING(m_svgX + xDiff);
            y1 = EMU_TO_CM_STRING(m_svgY + yDiff);
            x2 = EMU_TO_CM_STRING(m_svgX + m_svgWidth - xDiff);
            y2 = EMU_TO_CM_STRING(m_svgY + m_svgHeight - yDiff);
        }
        if (m_flipV) {
            QString temp = y2;
            y2 = y1;
            y1 = temp;
        }
        if (m_flipH) {
            QString temp = x2;
            x2 = x1;
            x1 = temp;
        }
            body->addAttribute("svg:x1", x1);
            body->addAttribute("svg:y1", y1);
            body->addAttribute("svg:x2", x2);
            body->addAttribute("svg:y2", y2);
        }
        if (m_contentType != "line") {
            if (m_rot == 0) {
                body->addAttribute("svg:x", EMU_TO_CM_STRING(m_svgX));
                body->addAttribute("svg:y", EMU_TO_CM_STRING(m_svgY));
            }
            body->addAttribute("svg:width", EMU_TO_CM_STRING(m_svgWidth));
            body->addAttribute("svg:height", EMU_TO_CM_STRING(m_svgHeight));
        }
        if (m_rot != 0 && m_contentType != "line") {
            // m_rot is in 1/60,000th of a degree
            qreal angle, xDiff, yDiff;
            MSOOXML::Utils::rotateString(m_rot, m_svgWidth, m_svgHeight, angle, xDiff, yDiff, m_flipH, m_flipV);
            QString rotString = QString("rotate(%1) translate(%2cm %3cm)")
                                    .arg(angle).arg((m_svgX + xDiff)/360000).arg((m_svgY + yDiff)/360000);
            body->addAttribute("draw:transform", rotString);
        }
    }
#elif defined(XLSXXMLDRAWINGREADER_CPP)
    if (m_currentDrawingObject->m_positions.contains(XlsxDrawingObject::FromAnchor)) {
        XlsxDrawingObject::Position f = m_currentDrawingObject->m_positions[XlsxDrawingObject::FromAnchor];
        body->addAttributePt("svg:x", EMU_TO_POINT(f.m_colOff));
        body->addAttributePt("svg:y", EMU_TO_POINT(f.m_rowOff));
        if (m_currentDrawingObject->m_positions.contains(XlsxDrawingObject::ToAnchor)) {
            f = m_currentDrawingObject->m_positions[XlsxDrawingObject::ToAnchor];
            body->addAttribute("table:end-cell-address", KSpread::Util::encodeColumnLabelText(f.m_col+1) + QString::number(f.m_row+1));
            body->addAttributePt("table:end-x", EMU_TO_POINT(f.m_colOff));
            body->addAttributePt("table:end-y", EMU_TO_POINT(f.m_rowOff));
        } else {
            body->addAttributePt("svg:width", EMU_TO_POINT(m_svgWidth));
            body->addAttributePt("svg:height", EMU_TO_POINT(m_svgHeight));
        }
    }
#else
#ifdef __GNUC__
#warning TODO: docx
#endif
#endif // PPTXXMLSLIDEREADER_H
}

KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::generatePlaceHolderSp()
{
#ifdef PPTXXMLSLIDEREADER_H
    const QString styleId(d->phStyleId());

    kDebug() << "styleId:" << styleId << "d->phType:" << d->phType << "d->phIdx:" << d->phIdx;

    if (m_context->type == SlideLayout) {
        PptxShapeProperties* masterShapeProperties = 0;
        if (!styleId.isEmpty()) {
            masterShapeProperties = m_context->slideProperties->shapesMap.value(styleId);
        }
        else if (d->shapeNumber < (uint)m_context->slideProperties->shapes.count()) {
            masterShapeProperties = m_context->slideProperties->shapes[d->shapeNumber];
        }
        kDebug() << "masterShapeProperties:" << masterShapeProperties;

        if (masterShapeProperties) {
            m_currentShapeProperties = new PptxShapeProperties(*masterShapeProperties);
        } else { // Case where it was not present in master slide at all
            m_currentShapeProperties = new PptxShapeProperties;
            m_currentShapeProperties->x = m_svgX;
            m_currentShapeProperties->y = m_svgY;
            m_currentShapeProperties->width = m_svgWidth;
            m_currentShapeProperties->height = m_svgHeight;
            m_currentShapeProperties->rot = m_rot;
        }
        if (!d->phType.isEmpty()) {
            m_context->slideLayoutProperties->shapesMap[d->phType] = m_currentShapeProperties;
        }
        if (!d->phIdx.isEmpty()) {
            m_context->slideLayoutProperties->shapesMap[d->phIdx] = m_currentShapeProperties;
        }
        m_context->slideLayoutProperties->shapes.append(m_currentShapeProperties); 
    }
    else if (m_context->type == SlideMaster) {
        kDebug() << "m_context->slideProperties->shapesMap insert:" << styleId;
        if (!styleId.isEmpty()) {
            m_context->slideProperties->shapesMap[styleId] = m_currentShapeProperties;
        }
        if (!d->phIdx.isEmpty()) {
            m_context->slideProperties->shapesMap[d->phIdx] = m_currentShapeProperties;
        }
        m_context->slideProperties->shapes.append(m_currentShapeProperties);
    }
    if (!m_outputDrawFrame && m_context->type == SlideLayout) {
        // presentation:placeholder
        Q_ASSERT(m_placeholderElWriter);
        QString presentationObject;
        presentationObject = MSOOXML::Utils::ST_PlaceholderType_to_ODF(d->phType);
        QString phStyleId = d->phType;
        if (phStyleId.isEmpty()) {
            // were indexing placeholders by id if type is not present, so shaped can refer to them by id
            phStyleId = d->phIdx;
        }

        // Keep this placeholder information for reuse in slides because ODF requires
        // not only reference but redundant copy of the properties to be present in slides.
        PptxPlaceholder *placeholder;
        if (m_xfrm_read) {
            placeholder = new PptxPlaceholder();
            placeholder->x = m_svgX;
            placeholder->y = m_svgY;
            placeholder->width = m_svgWidth;
            placeholder->height = m_svgHeight;
            placeholder->rot = m_rot;
        }
        else if (m_currentShapeProperties && m_currentShapeProperties->width >= 0) {
            kDebug() << "copying geometry from master to placeholder";
            placeholder = new PptxPlaceholder(*m_currentShapeProperties);
        }
        else {
            // We should never come here, as this means that values were not defined in the layout nor
            // in the masterslide
            kDebug() << "Xfrm values not defined neither in layout or masterslide";
            return KoFilter::WrongFormat;
        }
        kDebug() << "adding placeholder" << presentationObject << "phStyleId:" << phStyleId;
        m_context->slideLayoutProperties->placeholders.insert(phStyleId, placeholder);

        m_placeholderElWriter->startElement("presentation:placeholder");
        m_placeholderElWriter->addAttribute("presentation:object", presentationObject);
        if (placeholder->rot == 0) {
            m_placeholderElWriter->addAttribute("svg:x", EMU_TO_CM_STRING(placeholder->x));
            m_placeholderElWriter->addAttribute("svg:y", EMU_TO_CM_STRING(placeholder->y));
        }
        m_placeholderElWriter->addAttribute("svg:width", EMU_TO_CM_STRING(placeholder->width));
        m_placeholderElWriter->addAttribute("svg:height", EMU_TO_CM_STRING(placeholder->height));
        m_rot = placeholder->rot;
        if (m_rot != 0) {
            qreal angle, xDiff, yDiff;
            MSOOXML::Utils::rotateString(m_rot, m_svgWidth, m_svgHeight, angle, xDiff, yDiff, m_flipH, m_flipV);
            QString rotString = QString("rotate(%1) translate(%2cm %3cm)")
                                .arg(angle).arg((m_svgX + xDiff)/360000).arg((m_svgY + yDiff)/360000);
            m_placeholderElWriter->addAttribute("draw:transform", rotString);

        }

        m_placeholderElWriter->endElement();
    }

#endif

#ifdef PPTXXMLSLIDEREADER_H
    m_currentShapeProperties = 0; // Making sure that nothing uses them.
#endif
    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL cxnSp
//! cxnSp handler (connection shape)
//!TODO: don't imitate this to be normal shape
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_cxnSp()
{
    READ_PROLOGUE

#ifdef PPTXXMLSLIDEREADER_H
    // Ooxml supports slides getting items from layout slide, this is not supported in odf
    // Therefore we are buffering the potential item frames from layout and using them later in the slide
    QBuffer layoutBuf;
    KoXmlWriter layoutWriter(&layoutBuf);
    KoXmlWriter *bodyBackup = body;

    if (m_context->type == SlideLayout) {
        body = &layoutWriter;
    }
#endif

    preReadSp();

    pushCurrentDrawStyle(new KoGenStyle(KoGenStyle::GraphicAutoStyle, "graphic"));

    MSOOXML::Utils::XmlWriteBuffer drawFrameBuf; // buffer this draw:frame, because we have
    // to write after the child elements are generated
    body = drawFrameBuf.setWriter(body);
    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(nvCxnSpPr)
            ELSE_TRY_READ_IF(spPr)
            ELSE_TRY_READ_IF(style)
#ifdef PPTXXMLSLIDEREADER_H
            else {
                TRY_READ_IF(txBody)
            }
#endif
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    m_outputDrawFrame = true;

#ifdef PPTXXMLSLIDEREADER_H
    const QString styleId(d->phStyleId());
    if (m_context->type == SlideLayout && !styleId.isEmpty()) {
        m_outputDrawFrame = false;
        body = drawFrameBuf.originalWriter();
        drawFrameBuf.clear();
        kDebug() << "giving up outputDrawFrame for because ph@type is not empty:" << d->phType << "m_context->type=" << m_context->type;
    }
#endif

    if (m_outputDrawFrame) {
        body = drawFrameBuf.originalWriter();

        generateFrameSp();

        (void)drawFrameBuf.releaseWriter();
        body->endElement(); //draw:frame, //draw:line
    }

    KoFilter::ConversionStatus stat = generatePlaceHolderSp();
    if (stat != KoFilter::OK) {
        return stat;
    }

    popCurrentDrawStyle();

#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideLayout) {
        if (!d->phRead) {
            const QString elementContents = QString::fromUtf8(layoutBuf.buffer(), layoutBuf.buffer().size());
            m_context->slideLayoutProperties->layoutFrames.push_back(elementContents);
        }
        body = bodyBackup;
    }
#endif

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL sp
//! sp handler (Shape)
//! ECMA-376, 19.3.1.43, p. 2854; 20.1.2.2.33, p. 3053 - DrawingML.
/*! This element specifies the existence of a single shape.
 A shape can either be a preset or a custom geometry,
 defined using the DrawingML framework.

 Parent elements:
    - grpSp (§19.3.1.22)
    - grpSp (§20.1.2.2.20) - DrawingML
    - lockedCanvas (§20.3.2.1) - DrawingML
    - [done] spTree (§19.3.1.45)
 Child elements:
    - extLst (Extension List with Modification Flag) §19.3.1.20
    - extLst (Extension List) §20.1.2.2.15 - DrawingML
    - [done] nvSpPr (Non-Visual Properties for a Shape) §19.3.1.34
    - [done] nvSpPr (Non-Visual Properties for a Shape) §20.1.2.2.29 - DrawingML
    - [done] spPr (Shape Properties) §19.3.1.44
    - [done] spPr (Shape Properties) §20.1.2.2.35 - DrawingML
    - style (Shape Style) §19.3.1.46
    - [done] style (Shape Style) §20.1.2.2.37 - DrawingML
    - [done] txBody (Shape Text Body) §19.3.1.51 - PML
    - [done] txSp (Text Shape) §20.1.2.2.41 - DrawingML
 Attributes:
 - [unsupported?] useBgFill
*/
//! @todo support all elements
//! CASE #P405
//! CASE #P425
//! CASE #P430
//! CASE #P476
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_sp()
{
    READ_PROLOGUE

    m_contentType.clear();

#ifdef PPTXXMLSLIDEREADER_H
    // Ooxml supports slides getting items from layout slide, this is not supported in odf
    // Therefore we are buffering the potential item frames from layout and using them later in the slide
    QBuffer layoutBuf;
    KoXmlWriter layoutWriter(&layoutBuf);
    KoXmlWriter *bodyBackup = body;

    if (m_context->type == SlideLayout) {
        body = &layoutWriter;
    }
#elif defined(XLSXXMLDRAWINGREADER_CPP)
    KoXmlWriter *bodyBackup = body;
    body = m_currentDrawingObject->setShape(new XlsxShape());
#endif

    preReadSp();

    pushCurrentDrawStyle(new KoGenStyle(KoGenStyle::GraphicAutoStyle, "graphic"));

    MSOOXML::Utils::XmlWriteBuffer drawFrameBuf; // buffer this draw:frame, because we have
    // to write after the child elements are generated
    body = drawFrameBuf.setWriter(body);

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(nvSpPr)
            ELSE_TRY_READ_IF(spPr)
            ELSE_TRY_READ_IF(style)
#if defined(PPTXXMLSLIDEREADER_H)
            ELSE_TRY_READ_IF(txBody)
#endif
            else if (qualifiedName() == QLatin1String(QUALIFIED_NAME(txBody))) {
                KoXmlWriter* w = body;
                body->startElement("draw:text-box");
                TRY_READ(DrawingML_txBody)
                w->endElement();
            }
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    m_outputDrawFrame = true;

#ifdef PPTXXMLSLIDEREADER_H
    const QString styleId(d->phStyleId());
    if (m_context->type == SlideLayout && !styleId.isEmpty()) {
        m_outputDrawFrame = false;
        body = drawFrameBuf.originalWriter();
        drawFrameBuf.clear();
        kDebug() << "giving up outputDrawFrame for because ph@type is not empty:" << d->phType << "m_context->type=" << m_context->type;
    }
#endif

    if (m_outputDrawFrame) {
        body = drawFrameBuf.originalWriter();

        generateFrameSp();

        (void)drawFrameBuf.releaseWriter();
        body->endElement(); //draw:frame, //draw:line
    }

    KoFilter::ConversionStatus stat = generatePlaceHolderSp();
    if (stat != KoFilter::OK) {
        return stat;
    }

    popCurrentDrawStyle();

#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideLayout) {
        if (!d->phRead) {
            const QString elementContents = QString::fromUtf8(layoutBuf.buffer(), layoutBuf.buffer().size());
            m_context->slideLayoutProperties->layoutFrames.push_back(elementContents);
        }
        body = bodyBackup;
    }    
#elif defined(XLSXXMLDRAWINGREADER_CPP)
    body = bodyBackup;
#endif

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL style
//! style handler (Shape style)
/*
 Parent elements:
 - cxnSp (§19.3.1.19);
 - pic (§19.3.1.37);
 - [done] sp (§19.3.1.43)

 Child elements:
 - effectRef (Effect Reference) §20.1.4.2.8
 - [done] fillRef (Fill Reference) §20.1.4.2.10
 . fontRef (Font Reference) §20.1.4.1.17
 - [done] lnRef (Line Reference) §20.1.4.2.19

*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_style()
{
    READ_PROLOGUE

    // We don't want to overlap the current style
    if (!m_currentDrawStyle->isEmpty()) {
        skipCurrentElement();
        READ_EPILOGUE
    }

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF_NS(a, fillRef)
            ELSE_TRY_READ_IF_NS(a, lnRef)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL spPr
//! spPr handler (Shape Properties)
/*! ECMA-376, 19.3.1.44, p. 2855; 20.1.2.2.35, p. 3055 (DrawingML)
 This element specifies the visual shape properties that can be applied to a shape.
 These properties include the shape fill, outline, geometry, effects, and 3D orientation.

 Parent elements:
    - cxnSp (§19.3.1.19)
    - cxnSp (§20.1.2.2.10) - DrawingML
    - lnDef (§20.1.4.1.20) - DrawingML
    - [done] pic (§19.3.1.37)
    - [done] pic (§20.1.2.2.30) - DrawingML
    - [done] sp (§19.3.1.43)
    - [done] sp (§20.1.2.2.33) - DrawingML
    - spDef (§20.1.4.1.27) - DrawingML
    - txDef (§20.1.4.1.28) - DrawingML

 Child elements:
    - blipFill (Picture Fill) §20.1.8.14
    - custGeom (Custom Geometry) §20.1.9.8
    - effectDag (Effect Container) §20.1.8.25
    - effectLst (Effect Container) §20.1.8.26
    - extLst (Extension List) §20.1.2.2.15
    - [done] gradFill (Gradient Fill) §20.1.8.33
    - grpFill (Group Fill) §20.1.8.35
    - ln (Outline) §20.1.2.2.24
    - [done] noFill (No Fill) §20.1.8.44
    - pattFill (Pattern Fill) §20.1.8.47
    - [done] prstGeom (Preset geometry) §20.1.9.18
    - scene3d (3D Scene Properties) §20.1.4.1.26
    - [done] solidFill (Solid Fill) §20.1.8.54
    - sp3d (Apply 3D shape properties) §20.1.5.12
    - [done] xfrm (2D Transform for Individual Objects) §20.1.7.6
 Attributes:
    - bwMode (Black and White Mode)
*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_spPr()
{
    READ_PROLOGUE
    m_noFill = false;

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            if (qualifiedName() == QLatin1String("a:xfrm")) {
                TRY_READ(xfrm)
                m_xfrm_read = true;
            }
            else if (qualifiedName() == QLatin1String("a:solidFill")) {
#ifdef PPTXXMLSLIDEREADER_H
                d->textBoxHasContent = true; // We count normal fill and gardient as content
#endif
                TRY_READ(solidFill)
                // We must set the color immediately, otherwise currentColor may be modified by eg. ln
                m_currentDrawStyle->addProperty("draw:fill", QLatin1String("solid"));
                m_currentDrawStyle->addProperty("draw:fill-color", m_currentColor.name());
                m_currentColor = QColor();
            }
            else if ( qualifiedName() == QLatin1String("a:ln") ) {
                TRY_READ(ln)
            }
            else if (qualifiedName() == QLatin1String("a:noFill")) {
                m_noFill = true;
            }
            else if (qualifiedName() == QLatin1String("a:prstGeom")) {
                TRY_READ(prstGeom)
            }
            else if (qualifiedName() == QLatin1String("a:gradFill")) {
#ifdef PPTXXMLSLIDEREADER_H
                d->textBoxHasContent = true;
#endif
                m_currentGradientStyle = KoGenStyle(KoGenStyle::GradientStyle);
                TRY_READ(gradFill)
                m_currentDrawStyle->addProperty("draw:fill", "gradient");
                const QString gradName = mainStyles->insert(m_currentGradientStyle);
                m_currentDrawStyle->addProperty("draw:fill-gradient-name", gradName);
            }
//! @todo a:prstGeom...
//! @todo add ELSE_WRONG_FORMAT
        }
    }

#ifdef PPTXXMLSLIDEREADER_H
    const QString styleId(d->phStyleId());
    kDebug() << "styleId:" << styleId;

    if (m_context->type == Slide && !m_xfrm_read) { // loading values from slideLayout is needed
        //Q_ASSERT(d->shapeNumber >= 1 && d->shapeNumber <= m_context->slideLayoutProperties->shapes.count());
        PptxShapeProperties* props = 0;
        if (!styleId.isEmpty()) {
            props = m_context->slideLayoutProperties->shapesMap.value(styleId);
        }
        else if(d->shapeNumber >= 1 && d->shapeNumber <= (uint)m_context->slideLayoutProperties->shapes.count())
        {
            props = m_context->slideLayoutProperties->shapes[d->shapeNumber - 1];
        }
        if (!props) { // It was not present in layout, we need to get the place from slideMaster
            props = m_context->slideProperties->shapesMap.value(styleId);
            if (!props) {
                // In case there was nothing for this even in slideMaster, let's default to 'body' text position
                // Spec doesn't say anything about this case, but in reality there are such documents
                props = m_context->slideProperties->shapesMap.value("body");
            }
        }
        if (props) {
            m_svgX = props->x;
            m_svgY = props->y;
            m_svgWidth = props->width;
            m_svgHeight = props->height;
            m_rot = props->rot;
            m_isPlaceHolder = props->isPlaceHolder;
            kDebug() << "Copied from PptxShapeProperties:"
                << "d->shapeNumber:" << d->shapeNumber
                << "m_svgWidth:" << m_svgWidth << "m_svgHeight:" << m_svgHeight
                << "m_svgX:" << m_svgX << "m_svgY:" << m_svgY;
        }
    }
#endif

    READ_EPILOGUE
}

// ================================================================
//                             NameSpace "c"
// ================================================================
#ifndef MSOOXMLDRAWINGTABLESTYLEREADER_CPP

#undef MSOOXML_CURRENT_NS
#define MSOOXML_CURRENT_NS "c"

#undef CURRENT_EL
#define CURRENT_EL chart
//! chart handler (Charting diagrams)
/*!
@todo documentation
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_chart()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITH_NS(r, id)
    if (!r_id.isEmpty()) {
        const QString filepath = m_context->relationships->target(m_context->path, m_context->file, r_id);

        Charting::Chart* chart = new Charting::Chart;
        ChartExport* chartexport = new ChartExport(chart, m_context->themes);
#if defined(XLSXXMLDRAWINGREADER_CPP)
        chart->m_sheetName = m_context->worksheetReaderContext->worksheetName;
        chartexport->setSheetReplacement( false );
        if(m_currentDrawingObject->m_positions.contains(XlsxDrawingObject::FromAnchor)) {
            XlsxDrawingObject::Position f = m_currentDrawingObject->m_positions[XlsxDrawingObject::FromAnchor];
            chart->m_fromRow = f.m_row;
            chart->m_fromColumn = f.m_col;
            if(m_currentDrawingObject->m_positions.contains(XlsxDrawingObject::ToAnchor)) {
                f = m_currentDrawingObject->m_positions[XlsxDrawingObject::ToAnchor];
                chart->m_toRow = f.m_row;
                chart->m_toColumn = f.m_col;
            }
        }
#else
        chartexport->m_drawLayer = true;
        chartexport->m_x = EMU_TO_POINT(qMax(0, m_svgX));
        chartexport->m_y = EMU_TO_POINT(qMax(0, m_svgY));
        chartexport->m_width = m_svgWidth > 0 ? EMU_TO_POINT(m_svgWidth) : 100;
        chartexport->m_height = m_svgHeight > 0 ? EMU_TO_POINT(m_svgHeight) : 100;
#endif
        
        KoStore* storeout = m_context->import->outputStore();
        QScopedPointer<XlsxXmlChartReaderContext> context(new XlsxXmlChartReaderContext(storeout, chartexport));
        XlsxXmlChartReader reader(this);
        const KoFilter::ConversionStatus result = m_context->import->loadAndParseDocument(&reader, filepath, context.data());
        if (result != KoFilter::OK) {
            raiseError(reader.errorString());
            return result;
        }

#if defined(XLSXXMLDRAWINGREADER_CPP)
        m_currentDrawingObject->setChart(context.take());
#else
        chartexport->saveIndex(body);
#endif
    }

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#endif

// ================================================================
//                             NameSpace "a"
// ================================================================


#undef MSOOXML_CURRENT_NS
#define MSOOXML_CURRENT_NS "a"

#undef CURRENT_EL
#define CURRENT_EL fillRef
//! fillREf handler (Fill reference)
/*
 Parent elements:
 - [done] style (§21.3.2.24);
 - [done] style (§21.4.2.28);
 - [done] style (§20.1.2.2.37);
 - [done] style (§20.5.2.31);
 - [done] style (§19.3.1.46);
 - tblBg (§20.1.4.2.25);
 - tcStyle (§20.1.4.2.29)

 Child elements:
 - hslClr (Hue, Saturation, Luminance Color Model) §20.1.2.3.13
 - prstClr (Preset Color) §20.1.2.3.22
 - [done] schemeClr (Scheme Color) §20.1.2.3.29
 - [done] scrgbClr (RGB Color Model - Percentage Variant) §20.1.2.3.30
 - [done] srgbClr (RGB Color Model - Hex Variant) §20.1.2.3.32
 - [done] sysClr (System Color) §20.1.2.3.33

*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_fillRef()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(schemeClr)
            ELSE_TRY_READ_IF(scrgbClr)
            ELSE_TRY_READ_IF(sysClr)
            ELSE_TRY_READ_IF(srgbClr)
//! @todo add ELSE_WRONG_FORMAT
        }
    }


    m_currentDrawStyle->addProperty("draw:fill", QLatin1String("solid"));
    m_currentDrawStyle->addProperty("draw:fill-color", m_currentColor.name());

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lnRef
//! fillREf handler (Line reference)
/*
 Parent elements:
 - [done] style (§21.3.2.24);
 - [done] style (§21.4.2.28);
 - [done] style (§20.1.2.2.37);
 - [done] style (§20.5.2.31);
 - [done] style (§19.3.1.46);
 - tblBg (§20.1.4.2.25);
 - tcStyle (§20.1.4.2.29)

 Child elements:
 - hslClr (Hue, Saturation, Luminance Color Model) §20.1.2.3.13
 - prstClr (Preset Color) §20.1.2.3.22
 - [done] schemeClr (Scheme Color) §20.1.2.3.29
 - [done] scrgbClr (RGB Color Model - Percentage Variant) §20.1.2.3.30
 - [done] srgbClr (RGB Color Model - Hex Variant) §20.1.2.3.32
 - [done] sysClr (System Color) §20.1.2.3.33
*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lnRef()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(schemeClr)
            ELSE_TRY_READ_IF(srgbClr)
            ELSE_TRY_READ_IF(sysClr)
            ELSE_TRY_READ_IF(scrgbClr)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    m_currentPen.setColor(m_currentColor);
    KoOdfGraphicStyles::saveOdfStrokeStyle(*m_currentDrawStyle, *mainStyles, m_currentPen);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL overrideClrMapping
//! overrideClrMapping handler (Override Color Mapping)
/* This element provides an override for the color mapping in a document. When defined, this color mapping is
   used in place of the already defined color mapping, or master color mapping. This color mapping is defined in
   the same manner as the other mappings within this document.

 Parent elements:
 - [done] clrMapOvr (§19.3.1.7)

 Child elements:
 - extLst (Extension List) §20.1.2.2.15

*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_overrideClrMapping()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    int index = 0;
    while (index < attrs.size()) {
        const QString handledAttr = attrs.at(index).name().toString();
        const QString attrValue = attrs.value(handledAttr).toString();
#ifdef PPTXXMLSLIDEREADER_H
        m_context->colorMap[handledAttr] = attrValue;
#endif
        ++index;
    }

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL p
//! p handler (Text Paragraphs) ECMA-376, DrawingML 21.1.2.2.6, p. 3587.
//!   This element specifies the presence of a paragraph of text within the containing text body.
/*!
 Parent elements:
 - rich (§21.2.2.156)
 - t (§21.4.3.8)
 - txBody (§21.3.2.26)
 - txBody (§20.1.2.2.40)
 - txBody (§20.5.2.34)
 - [done] txBody (§19.3.1.51) - PML
 - txPr (§21.2.2.216)

 Child elements:
 - [done] br (Text Line Break) §21.1.2.2.1
 - endParaRPr (End Paragraph Run Properties) §21.1.2.2.3
 - [done] fld (Text Field) §21.1.2.2.4
 - pPr (Text Paragraph Properties) §21.1.2.2.7
 - [done] r (Text Run) §21.1.2.3.8
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_DrawingML_p()
{
    READ_PROLOGUE2(DrawingML_p)

    const read_p_args args = m_read_DrawingML_p_args;
    m_read_DrawingML_p_args = 0;
    m_paragraphStyleNameWritten = false;
    m_listStylePropertiesAltered = false;

    m_currentCombinedBulletProperties.clear();
    // Note that if buNone has been specified, we don't create a list
    m_currentListLevel = 1; // By default we're in the first level

#ifdef PPTXXMLSLIDEREADER_H
    inheritListStyles();
#else
    m_prevListLevel = m_currentListLevel = 0;
#endif

    // Creating a list ouf of what we have, note that ppr maybe overwrite the list style if it wishes
    m_currentListStyle = KoGenStyle(KoGenStyle::ListAutoStyle, "list");
    QMapIterator<int, MSOOXML::Utils::ParagraphBulletProperties> i(m_currentCombinedBulletProperties);
    int index = 0;
    while (i.hasNext()) {
        index++;
        i.next();
        m_currentListStyle.addChildElement(QString("list-style-properties%1").arg(index),
            i.value().convertToListProperties());
    }

    MSOOXML::Utils::XmlWriteBuffer textPBuf;

    if (args & read_p_Skip) {
        kDebug() << "SKIP!";
    } else {
        body = textPBuf.setWriter(body);
        m_currentParagraphStyle = KoGenStyle(KoGenStyle::ParagraphAutoStyle, "paragraph");
    }

    bool pprRead = false;
    bool rRead = false;

    while (!atEnd()) {
        readNext();
        kDebug() << "isStartElement:" << isStartElement();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            if (QUALIFIED_NAME_IS(p)) {
// CASE #301: avoid nested paragaraphs
                kDebug() << "Nested" << qualifiedName() << "detected: skipping the inner element";
                TRY_READ_WITH_ARGS(DrawingML_p, read_p_Skip;)
            }
// CASE #400.1
            else if (QUALIFIED_NAME_IS(pPr)) {
                TRY_READ(DrawingML_pPr)
                pprRead = true;
            }
            else if (QUALIFIED_NAME_IS(br)) {
                body->startElement("text:line-break");
                body->endElement(); // text:line-break
            }
// CASE #400.2
//! @todo add more conditions testing the parent
            else if (QUALIFIED_NAME_IS(r)) {
                rRead = true;
#ifdef PPTXXMLSLIDEREADER_H
                d->textBoxHasContent = true;
#endif
                TRY_READ(DrawingML_r)
            }
            ELSE_TRY_READ_IF(fld)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

#ifdef PPTXXMLSLIDEREADER_H
    if (!pprRead) {
        inheritDefaultParagraphStyle(m_currentParagraphStyle);
        inheritParagraphStyle(m_currentParagraphStyle);
    }
    if (!rRead) {
        // We are inheriting to paragraph's text-properties because there is no text
        // and thus m_currentTextStyle is not used
        inheritDefaultTextStyle(m_currentParagraphStyle);
        inheritTextStyle(m_currentParagraphStyle);
    }
#endif

    if (args & read_p_Skip) {
        //nothing
    } else {
        body = textPBuf.originalWriter();
        if (m_listStylePropertiesAltered) {
            if (m_prevListLevel > 0) {
                // Ending our current level
                body->endElement(); // text:list
                // Ending any additional levels needed
                for(; m_prevListLevel > 1; --m_prevListLevel) {
                    body->endElement(); // text:list-item
                    body->endElement(); // text:list
                }
                m_prevListLevel = 0;
            }
        }

        if (!rRead) {
            // Making sure that if we were previously in a list and if there's an empty line, that
            // we don't output a bullet to it
            m_currentListLevel = 0;
            m_lstStyleFound = false;
        }
        else if (m_currentCombinedBulletProperties.value(m_currentListLevel).isEmpty() && !m_listStylePropertiesAltered) {
            m_currentListLevel = 0;
            m_lstStyleFound = false;
        }
        else {
            m_lstStyleFound = true;
        }

        // In MSOffice it's possible that a paragraph defines a list-style that should be used without
        // being a list-item. We need to handle that case and need to make sure that such paragraph's
        // end as first-level list-items in ODF.
        if (m_currentListLevel > 0 || m_prevListLevel > 0) {
#ifdef PPTXXMLSLIDEREADER_H
             if (m_prevListLevel < m_currentListLevel) {
                 if (m_prevListLevel > 0) {
                     // Because there was an existing list, we need to start ours with list:item
                     body->startElement("text:list-item");
                 }
                 for(int listDepth = m_prevListLevel; listDepth < m_currentListLevel; ++listDepth) {
                     body->startElement("text:list");
                     if (listDepth == 0) {
                         QString listStyleName = mainStyles->insert(m_currentListStyle);
                         if (m_context->type == SlideMaster) {
                             mainStyles->markStyleForStylesXml(listStyleName);
                         }
                         Q_ASSERT(!listStyleName.isEmpty());
                         body->addAttribute("text:style-name", listStyleName);
                         m_currentParagraphStyle.addProperty("style:list-style-name", listStyleName);
                    }
                    body->startElement("text:list-item");
                 }
             } else if (m_prevListLevel > m_currentListLevel) {
                 body->endElement(); // This ends the latest list text:list
                 for(int listDepth = m_prevListLevel-1; listDepth > m_currentListLevel; --listDepth) {
                     //Ending any additional list levels needed
                     body->endElement(); // text:list-item
                     body->endElement(); // text:list
                 }
                 // Starting our own stuff for this level
                 if (m_currentListLevel > 0) {
                     body->endElement(); // revoving last lists text:list-item
                     body->startElement("text:list-item");
                 }
             } else { // m_prevListLevel==m_currentListLevel
                 body->startElement("text:list-item");
             }
#else
             for(int i = 0; i < m_currentListLevel; ++i) {
                 body->startElement("text:list");
                 // Todo, should most likely add the name of the current list style
                 body->startElement("text:list-item");
             }
#endif
         }

         body->startElement("text:p", false);
#ifdef PPTXXMLSLIDEREADER_H
         if (m_context->type == SlideMaster) {
             m_moveToStylesXml = true;
         }
#endif

         setupParagraphStyle();

         (void)textPBuf.releaseWriter();
         body->endElement(); //text:p
#ifdef PPTXXMLSLIDEREADER_H
         // We should end our own list level
         if (m_currentListLevel > 0) {
             body->endElement(); // text:list-item
         }
#endif

#ifdef PPTXXMLSLIDEREADER_H
        m_prevListLevel = m_currentListLevel;
#else
         // For !=powerpoint we create a new list for each paragraph rather then nesting the lists cause the word
         // and excel filters still need to be adjusted to proper handle nested lists.
         const bool closeList = true;
         if (closeList) {
             for(int i = 0; i < m_currentListLevel; ++i) {
                 body->endElement(); // text:list-item
                 body->endElement(); // text:list
             }
             m_prevListLevel = m_currentListLevel = 0;
             m_lstStyleFound = false;
         } else {
             m_prevListLevel = m_currentListLevel;
         }
#endif
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL r
//! r handler (Text Run)
/*! ECMA-376, 21.1.2.3.8, p.3623.

 Parent elements:
 - [done] p (§21.1.2.2.6)

 Child elements:
 - [done] rPr (Text Run Properties) §21.1.2.3.9
 - [done] t (Text String) §21.1.2.3.11
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_DrawingML_r()
{
    READ_PROLOGUE2(DrawingML_r)

    m_hyperLink = false;

    MSOOXML::Utils::XmlWriteBuffer rBuf;
    body = rBuf.setWriter(body);

    m_currentTextStyleProperties = new KoCharacterStyle();
    m_currentTextStyle = KoGenStyle(KoGenStyle::TextAutoStyle, "text");

#ifdef PPTXXMLSLIDEREADER_H
    inheritDefaultTextStyle(m_currentTextStyle);
    inheritTextStyle(m_currentTextStyle);
#endif

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            if (QUALIFIED_NAME_IS(rPr)) {
                TRY_READ(DrawingML_rPr)
            }
            ELSE_TRY_READ_IF(t)
            ELSE_WRONG_FORMAT
        }
    }

    // elements
    m_currentTextStyleProperties->saveOdf(m_currentTextStyle);

    body = rBuf.originalWriter();

    if (m_hyperLink) {
        body->startElement("text:a");
        body->addAttribute("xlink:type", "simple");
        body->addAttribute("xlink:href", QUrl(m_hyperLinkTarget).toEncoded());
    }

    const QString currentTextStyleName(mainStyles->insert(m_currentTextStyle));

#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideMaster) {
        mainStyles->markStyleForStylesXml(currentTextStyleName);
    }
#endif

    body->startElement("text:span", false);
    body->addAttribute("text:style-name", currentTextStyleName);

    (void)rBuf.releaseWriter();

    body->endElement(); //text:span
    if (m_hyperLink) {
        body->endElement(); // text:a
    }

    delete m_currentTextStyleProperties;
    m_currentTextStyleProperties = 0;

    READ_EPILOGUE
}

static QFont::Capitalization capToOdf(const QString& cap)
{
    if (cap == QLatin1String("small")) {
        return QFont::SmallCaps;
    }
    else if (cap == QLatin1String("all")) {
        return QFont::AllUppercase;
    }
    return QFont::MixedCase;
}

#undef CURRENT_EL
#define CURRENT_EL rPr
//! rPr handler (Text Run Properties) DrawingML ECMA-376, 21.1.2.3.9, p.3624.
//! This element contains all run level text properties for the text runs within a containing paragraph.
/*!
 Parent elements:
 - br (§21.1.2.2.1)
 - fld (§21.1.2.2.4)
 - [done] r (§21.1.2.3.8)
 Attributes:
 - altLang (Alternative Language)
 - b (Bold)
 - baseline (Baseline)
 - bmk (Bookmark Link Target)
 - cap (Capitalization)
 - dirty (Dirty)
 - err (Spelling Error)
 - i (Italics)
 - kern (Kerning)
 - kumimoji (Kumimoji)
 - lang (Language ID)
 - noProof (No Proofing)
 - normalizeH (Normalize Heights)
 - smtClean (SmartTag Clean)
 - smtId (SmartTag ID)
 - spc (Spacing)
 - strike (Strikethrough)
 - sz (Font Size)
 - u (Underline)
 Child elements:
 - [done] blipFill (Picture Fill) §20.1.8.14
 - cs (Complex Script Font) §21.1.2.3.1
 - ea (East Asian Font) §21.1.2.3.3
 - effectDag (Effect Container) §20.1.8.25
 - effectLst (Effect Container) §20.1.8.26
 - extLst (Extension List) §20.1.2.2.15
 - gradFill (Gradient Fill) §20.1.8.33
 - grpFill (Group Fill) §20.1.8.35
 - [done] highlight (Highlight Color) §21.1.2.3.4
 - [done] hlinkClick (Click Hyperlink) §21.1.2.3.5
 - hlinkMouseOver (Mouse-Over Hyperlink) §21.1.2.3.6
 - [done] latin (Latin Font) §21.1.2.3.7
 - [done] ln (Outline) §20.1.2.2.24
 - [done] noFill (No Fill) §20.1.8.44
 - pattFill (Pattern Fill) §20.1.8.47
 - rtl (Right to Left Run) §21.1.2.2.8
 - [done] solidFill (Solid Fill) §20.1.8.54
 - sym (Symbol Font) §21.1.2.3.10
 - uFill (Underline Fill) §21.1.2.3.12
 - uFillTx (Underline Fill Properties Follow Text) §21.1.2.3.13
 - uLn (Underline Stroke) §21.1.2.3.14
 - uLnTx (Underline Follows Text) §21.1.2.3.15
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_DrawingML_rPr()
{
    READ_PROLOGUE2(DrawingML_rPr)

    m_hyperLink = false;

    const QXmlStreamAttributes attrs(attributes());

    m_currentColor = QColor();

    // Read child elements
    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(latin)
            ELSE_TRY_READ_IF_IN_CONTEXT(blipFill)
            ELSE_TRY_READ_IF(solidFill)
            ELSE_TRY_READ_IF_IN_CONTEXT(noFill)
            else if (QUALIFIED_NAME_IS(highlight)) {
                TRY_READ(DrawingML_highlight)
            }
            ELSE_TRY_READ_IF(ln)
            ELSE_TRY_READ_IF(hlinkClick)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    if (m_currentColor.isValid()) {
        m_currentTextStyleProperties->setForeground(m_currentColor);
        m_currentColor = QColor();
    }

    // Read Attributes

    // DrawingML: b, i, strike, u attributes:
    if (attrs.hasAttribute("b")) {
        m_currentTextStyleProperties->setFontWeight(
            MSOOXML::Utils::convertBooleanAttr(attrs.value("b").toString()) ? QFont::Bold : QFont::Normal);
    }
    if (attrs.hasAttribute("i")) {
        m_currentTextStyleProperties->setFontItalic(
            MSOOXML::Utils::convertBooleanAttr(attrs.value("i").toString()));
    }

    TRY_READ_ATTR_WITHOUT_NS(cap);
    if (!cap.isEmpty()) {
        m_currentTextStyleProperties->setFontCapitalization(capToOdf(cap));
    }

    TRY_READ_ATTR_WITHOUT_NS(spc)
    if (!spc.isEmpty()) {
        int spcInt;
        STRING_TO_INT(spc, spcInt, "rPr@spc")
        m_currentTextStyleProperties->setFontLetterSpacing(qreal(spcInt) / 100.0);
    }

#ifdef PPTXXMLSLIDEREADER_H
    kDebug() << "d->phType ____" << d->phType;
#endif

    TRY_READ_ATTR_WITHOUT_NS(sz)
    if (!sz.isEmpty()) {
        int szInt;
        STRING_TO_INT(sz, szInt, "rPr@sz")
        m_currentTextStyleProperties->setFontPointSize(qreal(szInt) / 100.0);
    }
    // from 20.1.10.79 ST_TextStrikeType (Text Strike Type)
    TRY_READ_ATTR_WITHOUT_NS(strike)
    if (strike == QLatin1String("sngStrike")) {
        m_currentTextStyleProperties->setStrikeOutType(KoCharacterStyle::SingleLine);
        m_currentTextStyleProperties->setStrikeOutStyle(KoCharacterStyle::SolidLine);
    } else if (strike == QLatin1String("dblStrike")) {
        m_currentTextStyleProperties->setStrikeOutType(KoCharacterStyle::DoubleLine);
        m_currentTextStyleProperties->setStrikeOutStyle(KoCharacterStyle::SolidLine);
    } else {
        // empty or "noStrike"
    }
    // from
    TRY_READ_ATTR_WITHOUT_NS(baseline)
    if (!baseline.isEmpty()) {
        int baselineInt;
        STRING_TO_INT(baseline, baselineInt, "rPr@baseline")
        if (baselineInt > 0)
            m_currentTextStyleProperties->setVerticalAlignment( QTextCharFormat::AlignSuperScript );
        else if (baselineInt < 0)
            m_currentTextStyleProperties->setVerticalAlignment( QTextCharFormat::AlignSubScript );
    }

    TRY_READ_ATTR_WITHOUT_NS(u)
    if (!u.isEmpty()) {
        MSOOXML::Utils::setupUnderLineStyle(u, m_currentTextStyleProperties);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL hlinkClick
//! hlinkClick handler
/*!
 Parent elements:
 - cNvPr (§21.3.2.7)
 - cNvPr (§20.1.2.2.8)
 - cNvPr (§20.2.2.3) 
 - cNvPr (§20.5.2.8) 
 - cNvPr (§19.3.1.12)
 - defRPr (§21.1.2.3.2)
 - docPr (§20.4.2.5) 
 - endParaRPr (§21.1.2.2.3)
 - [done] rPr (§21.1.2.3.9)

 Child elements:
 - extLst (§20.1.2.2.15)
 - snd (§20.1.2.2.32)

TODO....
 Attributes..
 Children..
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_hlinkClick()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITH_NS(r, id)

    if (r_id.isEmpty()) {
        m_hyperLinkTarget.clear();
    }
    else {
        m_hyperLink = true;
        m_hyperLinkTarget = m_context->relationships->target(m_context->path, m_context->file, r_id);
        m_hyperLinkTarget.remove(0, m_context->path.length() + 1);
    }

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL pPr
//! pPr handler (Text Paragraph Properties) 21.1.2.2.7, p.3588.
/*!
 Parent elements:
  - fld (§21.1.2.2.4)
  - p (§21.1.2.2.6)
 Attributes:
  - [incomplete] algn (Alignment)
  - defTabSz (Default Tab Size)
  - eaLnBrk (East Asian Line Break)
  - fontAlgn (Font Alignment)
  - hangingPunct (Hanging Punctuation)
  - indent (Indent)
  - latinLnBrk (Latin Line Break)
  - [done] lvl (Level)
  - marL (Left Margin)
  - marR (Right Margin)
  - rtl (Right To Left)
 Child elements:
  - [done] buAutoNum (Auto-Numbered Bullet) §21.1.2.4.1
  - [done] buBlip (Picture Bullet) §21.1.2.4.2
  - [done] buChar (Character Bullet) §21.1.2.4.3
  - buClr (Color Specified) §21.1.2.4.4
  - buClrTx (Follow Text) §21.1.2.4.5
  - [done] buFont (Specified) §21.1.2.4.6
  - buFontTx (Follow text) §21.1.2.4.7
  - [done] buNone (No Bullet) §21.1.2.4.8
  - buSzPct (Bullet Size Percentage) §21.1.2.4.9
  - buSzPts (Bullet Size Points) §21.1.2.4.10
  - buSzTx (Bullet Size Follows Text) §21.1.2.4.11
  - defRPr (Default Text Run Properties) §21.1.2.3.2
  - extLst (Extension List) §20.1.2.2.15
  - [done] lnSpc (Line Spacing) §21.1.2.2.5
  - [done] spcAft (Space After) §21.1.2.2.9
  - [done] spcBef (Space Before) §21.1.2.2.10
  - tabLst (Tab List) §21.1.2.2.14
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_DrawingML_pPr()
{
    READ_PROLOGUE2(DrawingML_pPr)
    const QXmlStreamAttributes attrs(attributes());

    m_listStylePropertiesAltered = false;

    TRY_READ_ATTR_WITHOUT_NS(lvl)

    if (!lvl.isEmpty()) {
        m_currentListLevel = lvl.toInt() + 1;
    }

#ifdef PPTXXMLSLIDEREADER_H
    inheritDefaultParagraphStyle(m_currentParagraphStyle);
    inheritParagraphStyle(m_currentParagraphStyle);
#endif

    TRY_READ_ATTR_WITHOUT_NS(algn)
    algnToODF("fo:text-align", algn);

    TRY_READ_ATTR_WITHOUT_NS(marL)
    TRY_READ_ATTR_WITHOUT_NS(marR)
    TRY_READ_ATTR_WITHOUT_NS(indent)
    TRY_READ_ATTR_WITHOUT_NS(defTabSz)

    bool ok = false;

    if (!marL.isEmpty()) {
        const qreal marginal = qreal(EMU_TO_POINT(marL.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("fo:margin-left", marginal);
    }
    if (!marR.isEmpty()) {
        const qreal marginal = qreal(EMU_TO_POINT(marR.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("fo:margin-right", marginal);
    }
    if (!indent.isEmpty()) {
        const qreal firstInd = qreal(EMU_TO_POINT(indent.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("fo:text-indent", firstInd);
    }
    if (!defTabSz.isEmpty()) {
        const qreal tabSize = qreal(EMU_TO_POINT(defTabSz.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("style:tab-stop-distance", tabSize);
    }

    m_bulletFont = "";

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(buAutoNum)
            ELSE_TRY_READ_IF(buNone)
            ELSE_TRY_READ_IF(buChar)
            ELSE_TRY_READ_IF(buFont)
            ELSE_TRY_READ_IF(buBlip)
            else if (QUALIFIED_NAME_IS(spcBef)) {
                m_currentSpacingType = spacingMarginTop;
                TRY_READ(spcBef)
            }
            else if (QUALIFIED_NAME_IS(spcAft)) {
                m_currentSpacingType = spacingMarginBottom;
                TRY_READ(spcAft)
            }
            else if (QUALIFIED_NAME_IS(lnSpc)) {
                m_currentSpacingType = spacingLines;
                TRY_READ(lnSpc)
            }
        }
    }

    if (m_bulletFont == "Wingdings") {
        // Ooxml files have very often wingdings fonts, but usually they are not installed
        // Making the bullet character look ugly, thus defaulting to "-"
        m_lstStyleFound = true;
        m_listStylePropertiesAltered = true;
        m_currentBulletProperties.setBulletChar("-");
    }

    if (m_listStylePropertiesAltered) {
        m_currentListStyle = KoGenStyle(KoGenStyle::ListAutoStyle, "list");

        // For now we take a stand that any altered style makes its own list.
        m_currentBulletProperties.m_level = m_currentListLevel;

        m_currentListStyle.addChildElement("list-style-properties",
            m_currentBulletProperties.convertToListProperties());
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL xfrm
//! xfrm handler (2D Transform for Individual Objects)
//! DrawingML ECMA-376, 20.1.7.6, p. 3187.
/*! This element represents 2-D transforms for ordinary shapes.

 Parent elements:
    - graphicFrame (§20.1.2.2.18)
    - spPr (§21.2.2.197)
    - spPr (§21.3.2.23)
    - spPr (§21.4.3.7)
    - [done] spPr (§20.1.2.2.35) - DrawingML
    - spPr (§20.2.2.6)
    - spPr (§20.5.2.30)
    - [done] spPr (§19.3.1.44)
    - txSp (§20.1.2.2.41)
 Child elements:
    - [done] ext (Extents) §20.1.7.3
    - [done] off (Offset) §20.1.7.4
    - [done] chExt (Child extends) ..in case of a group shape
    - [done] chOff (Child offset) ..in case of a group shape
 Attributes:
    - flipH (Horizontal Flip)
    - flipV (Vertical Flip)
    - rot (Rotation)
*/
//! @todo support all child elements
//! CASE #P476
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_xfrm()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    // Read attributes.
    m_flipH = MSOOXML::Utils::convertBooleanAttr(attrs.value("flipH").toString(), false);
    m_flipV = MSOOXML::Utils::convertBooleanAttr(attrs.value("flipV").toString(), false);
    m_rot = 0;
    TRY_READ_ATTR_WITHOUT_NS(rot)
    STRING_TO_INT(rot, m_rot, "xfrm@rot")

    bool off_read = false;
    bool ext_read = false;
    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            if (QUALIFIED_NAME_IS(off)) {
                TRY_READ(off);
                off_read = true;
            } else if (QUALIFIED_NAME_IS(ext)) {
                TRY_READ(ext);
                ext_read = true;
            }
            ELSE_TRY_READ_IF(chOff)
            ELSE_TRY_READ_IF(chExt)
        }
//! @todo add ELSE_WRONG_FORMAT
    }

    /*//! @todo
        if (m_context->type == Slide) { // load values from master is needed
            if (!off_read) {
                m_svgX = m_currentShapeProperties->x;
                m_svgY = m_currentShapeProperties->y;
                kDebug() << "Inherited svg:x/y from master (m_currentShapeProperties)";
            }
            if (!ext_read) {
                m_svgWidth = m_currentShapeProperties->width;
                m_svgHeight = m_currentShapeProperties->y;
                kDebug() << "Inherited svg:width/height from master (m_currentShapeProperties)";
            }
        }*/
#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideMaster) { // save
        if (!off_read) {
            raiseElNotFoundError("a:off");
            return KoFilter::WrongFormat;
        }
        if (!ext_read) {
            raiseElNotFoundError("a:ext");
            return KoFilter::WrongFormat;
        }
    }
    if (m_currentShapeProperties && (m_context->type == SlideMaster || m_context->type == SlideLayout)) {
        m_currentShapeProperties->x = m_svgX;
        m_currentShapeProperties->y = m_svgY;
        m_currentShapeProperties->width = m_svgWidth;
        m_currentShapeProperties->height = m_svgHeight;
        m_currentShapeProperties->rot = m_rot;
        m_currentShapeProperties->isPlaceHolder = m_isPlaceHolder;
    }
#endif

    kDebug() << "svg:x" << m_svgX << "svg:y" << m_svgY << "svg:width" << m_svgWidth << "svg:height" << m_svgHeight << "rotation" << m_rot;

    READ_EPILOGUE
}

#undef MSOOXML_CURRENT_NS
#define MSOOXML_CURRENT_NS "a"

//! off handler (Offset)
//! DrawingML ECMA-376, 20.1.7.4, p. 3185.
/*! This element specifies the location of the bounding box of an object.
    Effects on an object are not included in this bounding box.

 Parent elements:
    - xfrm (§21.3.2.28)
    - xfrm (§20.1.7.5)
    - [done] xfrm (§20.1.7.6)
    - xfrm (§20.5.2.36)
    - xfrm (§19.3.1.53)

 No child elements.

 Attributes:
    - [done] x (X-Axis Coordinate)
    - [done] y (Y-Axis Coordinate)
*/
//! @todo support all elements
#undef CURRENT_EL
#define CURRENT_EL off
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_off()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    READ_ATTR_WITHOUT_NS(x)
    STRING_TO_INT(x, m_svgX, "off@x")
    READ_ATTR_WITHOUT_NS(y)
    STRING_TO_INT(y, m_svgY, "off@y")

    if (!m_inGrpSpPr) {
        int index = 0;
        while (index < m_svgProp.size()) {
            //(a:off(x) - a:chOff(x))/a:chExt(x) * a(p):ext(x) + a(p):off(x)
            GroupProp prop = m_svgProp.at(m_svgProp.size() - 1 - index);
            m_svgX = (m_svgX - prop.svgXChOld) / prop.svgWidthChOld * prop.svgWidthOld + prop.svgXOld;
            m_svgY = (m_svgY - prop.svgYChOld) / prop.svgHeightChOld * prop.svgHeightOld + prop.svgYOld;
            ++index;
        }
    }

    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

//! chOff handler (Child offset)
//! Look parent, children
#undef CURRENT_EL
#define CURRENT_EL chOff
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_chOff()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    READ_ATTR_WITHOUT_NS(x)
    STRING_TO_INT(x, m_svgChX, "chOff@x")
    READ_ATTR_WITHOUT_NS(y)
    STRING_TO_INT(y, m_svgChY, "chOff@y")


    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

//! ext handler (Extents)
//! DrawingML ECMA-376, 20.1.7.3, p. 3185.
/*! This element specifies the size of the bounding box enclosing the referenced object.
 Parent elements:
 - xfrm (§21.3.2.28)
 - xfrm (§20.1.7.5)
 - [done] xfrm (§20.1.7.6)
 - xfrm (§20.5.2.36)
 - xfrm (§19.3.1.53)

 No child elements.

 Attributes:
 - cx (Extent Length) Specifies the length of the extents rectangle in EMUs. This rectangle shall dictate
      the size of the object as displayed (the result of any scaling to the original object).
 - cy (Extent Width) Specifies the width of the extents rectangle in EMUs.
*/
//! @todo support all child elements
#undef CURRENT_EL
#define CURRENT_EL ext
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_ext()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    READ_ATTR_WITHOUT_NS(cx)
    STRING_TO_INT(cx, m_svgWidth, "ext@cx")
    READ_ATTR_WITHOUT_NS(cy)
    STRING_TO_INT(cy, m_svgHeight, "ext@cy")

    if (!m_inGrpSpPr) {
        int index = 0;
        while (index < m_svgProp.size()) {
            //(a:off(x) - a:chOff(x))/a:chExt(x) * a(p):ext(x) + a(p):off(x)
            GroupProp prop = m_svgProp.at(m_svgProp.size() - 1 - index);

            m_svgWidth = m_svgWidth * prop.svgWidthOld / prop.svgWidthChOld;
            m_svgHeight = m_svgHeight * prop.svgHeightOld / prop.svgHeightChOld;
            ++index;
        }
    }

    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

//! chExt handler (Child extend)
//! Look parent, children
#undef CURRENT_EL
#define CURRENT_EL chExt
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_chExt()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    READ_ATTR_WITHOUT_NS(cx)
    STRING_TO_INT(cx, m_svgChWidth, "chExt@cx")
    READ_ATTR_WITHOUT_NS(cy)
    STRING_TO_INT(cy, m_svgChHeight, "chExt@cy")

    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL blip
//! blip handler (Blip)
//! ECMA-376, 20.1.8.13, p. 3194
/*! This element specifies the existence of an image (binary large image or picture)
    and contains a reference to the image data.

 Parent elements:
    - blipFill (§21.3.2.2) - DrawingML, p. 3919
    - [done] blipFill (§20.1.8.14) - DrawingML, p. 3195
    - blipFill (§20.2.2.1) - DrawingML, p. 3456
    - blipFill (§20.5.2.2) - DrawingML, p. 3518
    - [done] blipFill (§19.3.1.4) - PresentationML, p. 2818
    - [done] buBlip (§21.1.2.4.2)

 Child elements:
    - alphaBiLevel (Alpha Bi-Level Effect) §20.1.8.1
    - alphaCeiling (Alpha Ceiling Effect) §20.1.8.2
    - alphaFloor (Alpha Floor Effect) §20.1.8.3
    - alphaInv (Alpha Inverse Effect) §20.1.8.4
    - alphaMod (Alpha Modulate Effect) §20.1.8.5
    - alphaModFix (Alpha Modulate Fixed Effect) §20.1.8.6
    - alphaRepl (Alpha Replace Effect) §20.1.8.8
    - [done] biLevel (Bi-Level (Black/White) Effect) §20.1.8.11
    - blur (Blur Effect) §20.1.8.15
    - clrChange (Color Change Effect) §20.1.8.16
    - clrRepl (Solid Color Replacement) §20.1.8.18
    - duotone (Duotone Effect) §20.1.8.23
    - extLst (Extension List) §20.1.2.2.15
    - fillOverlay (Fill Overlay Effect) §20.1.8.29
    - [done] grayscl (Gray Scale Effect) §20.1.8.34
    - hsl (Hue Saturation Luminance Effect) §20.1.8.39
    - [done] lum (Luminance Effect) §20.1.8.42
    - tint (Tint Effect) §20.1.8.60

 Attributes:
    - cstate (Compression State)
    - [done] embed (Embedded Picture Reference), 22.8.2.1 ST_RelationshipId (Explicit Relationship ID), p. 4324
    - link (Linked Picture Reference)
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_blip()
{
    READ_PROLOGUE

    m_xlinkHref.clear();

    // Read attributes.
    const QXmlStreamAttributes attrs(attributes());
//! @todo more attrs
    TRY_READ_ATTR_WITH_NS(r, embed)
    kDebug() << "embed:" << r_embed;
    if (!r_embed.isEmpty()) {
        const QString sourceName(m_context->relationships->target(m_context->path,
                                                                  m_context->file, r_embed));
        kDebug() << "sourceName:" << sourceName;

        m_context->import->imageSize(sourceName, m_imageSize);

        if (sourceName.isEmpty()) {
            return KoFilter::FileNotFound;
        }
#if defined(XLSXXMLDRAWINGREADER_CPP)
        QString destinationName = QLatin1String("Pictures/") + sourceName.mid(sourceName.lastIndexOf('/') + 1);;
        if(m_context->import->copyFile(sourceName, destinationName, false) == KoFilter::OK) {
            XlsxXmlEmbeddedPicture *picture = new XlsxXmlEmbeddedPicture(destinationName);
            if (m_currentDrawingObject->m_positions.contains(XlsxDrawingObject::FromAnchor)) {  // if we got 'from' cell
                picture->m_fromCell = m_currentDrawingObject->m_positions[XlsxDrawingObject::FromAnchor]; // store the starting cell
                if (m_currentDrawingObject->m_positions.contains(XlsxDrawingObject::ToAnchor)) {   // if we got 'to' cell
                    picture->m_toCell = m_currentDrawingObject->m_positions[XlsxDrawingObject::ToAnchor]; // store the ending cell
                }
            }
            m_currentDrawingObject->setPicture(picture);
        }
#else
        QString destinationName;
        RETURN_IF_ERROR( copyFile(sourceName, QLatin1String("Pictures/"), destinationName) )
        addManifestEntryForFile(destinationName);
        m_recentSourceName = sourceName;
        addManifestEntryForPicturesDir();
        m_xlinkHref = destinationName;
#endif
    }

    // Read child elements
    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(biLevel)
            ELSE_TRY_READ_IF(grayscl)
            ELSE_TRY_READ_IF(lum)
//! @todo add ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL stretch
//! stretch handler (Stretch)
//! ECMA-376, 20.1.8.56, p. 3233
/*! This element specifies that a BLIP should be stretched
 to fill the target rectangle. The other option is a tile where
 a BLIP is tiled to fill the available area.

 Parent elements:
    - blipFill (§21.3.2.2) - DrawingML, p. 3919
    - [done] blipFill (§20.1.8.14) - DrawingML, p. 3195
    - blipFill (§20.2.2.1) - DrawingML, p. 3456
    - blipFill (§20.5.2.2) - DrawingML, p. 3518
    - [done] blipFill (§19.3.1.4) - PresentationML, p. 2818

 Child elements:
    - [done] fillRect (Fill Rectangle) §20.1.8.30
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_stretch()
{
    READ_PROLOGUE

    m_fillImageRenderingStyleStretch = true;
    m_currentDrawStyle->addProperty("style:repeat", QLatin1String("stretch"));

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(fillRect)
            ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL biLevel
//! biLevel handler (BiLevel (Black/White) Effect)
/*! ECMA-376, 20.1.8.13, p. 3193

  This element specifies a bi-level (black/white) effect. Input colors
  whose luminance is less than the specified threshold value are
  changed to black. Input colors whose luminance are greater than or
  equal the specified value are set to white. The alpha effect values
  are unaffected by this effect.

 Parent elements:
 - [done] blip (§20.1.8.13)
 - cont (§20.1.8.20)
 - effectDag (§20.1.8.25)

 No child elements.

 Attributes
 - thresh (Threshold)
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_biLevel()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    m_currentDrawStyle->addProperty("draw:color-mode", QLatin1String("mono"));
//! @todo thresh attribute (no real counterpoint in ODF)

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL grayscl
//! grayscl handler (Grayscale effect)
/*! ECMA-376, 20.1.8.34, p 3217

  This element specifies a gray scale effect. Converts all effect
  color values to a shade of gray, corresponding to their
  luminance. Effect alpha (opacity) values are unaffected.

 Parent elements:
 - [done] blip (§20.1.8.13)
 - cont (§20.1.8.20)
 - effectDag (§20.1.8.25)

 No child elements.

 No attributes
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_grayscl()
{
    READ_PROLOGUE

    m_currentDrawStyle->addProperty("draw:color-mode", QLatin1String("greyscale"));

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lum
//! lum handler (luminance effect)
/*! ECMA-376, 20.1.8.34, p 3222

  This element specifies a luminance effect. Brightness linearly
  shifts all colors closer to white or black.  Contrast scales all
  colors to be either closer or further apart

 Parent elements:
 - [done] blip (§20.1.8.13)
 - cont (§20.1.8.20)
 - effectDag (§20.1.8.25)

 No child elements.

 Attributes
 - [done] bright (Brightness)
 - [done] contrast (Contrast)
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lum()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR_WITHOUT_NS(bright)
    TRY_READ_ATTR_WITHOUT_NS(contrast)

    // Numbers are in format 70000, so we need to remove 3 zeros
    // Adding bright to luminance may not be correct (but better than hardcoding to watermark mode)
    if (!bright.isEmpty()) {
        m_currentDrawStyle->addProperty("draw:luminance", bright.left(bright.length()-3) + '%');
    }

    if (!contrast.isEmpty()) {
        m_currentDrawStyle->addProperty("draw:contrast", contrast.left(contrast.length()-3) + '%');
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tile
//! tile handler (Placeholder Shape)
/*! ECMA-376, 19.3.1.36, p. 3234
 This element specifies that a BLIP should be tiled to fill the available space. This element defines a "tile"
 rectangle within the bounding box. The image is encompassed within the tile rectangle, and the tile rectangle
 is tiled across the bounding box to fill the entire area.

 Parent elements:
 - blipFill (§21.3.2.2)
 - blipFill (§20.1.8.14)
 - blipFill (§20.2.2.1)
 - blipFill (§20.5.2.2)
 - blipFill (§19.3.1.4)

 No child elements.
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_tile()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    m_currentDrawStyle->addProperty("style:repeat", QLatin1String("repeat"));
//! @todo algn - convert to "ODF's Fill Image Tile Reference Point"
    m_currentDrawStyle->addProperty("draw:fill-image-ref-point", "top-left");

//! @todo flip
//! @todo sx
//! @todo sy
//! @todo tx
//! @todo ty

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL fillRect
//! fillRect handler (Fill Rectangle)
//! ECMA-376, 20.1.8.30, p. 3212
/*! This element specifies a fill rectangle. When stretching of an image
    is specified, a source rectangle, srcRect, is scaled to fit the specified fill rectangle.

 Parent elements:
    - [done] stretch (§20.1.8.56)

 No child elements.

 Attributes:
    - b (Bottom Offset)
    - l (Left Offset)
    - r (Right Offset)
    - t (Top Offset)

 Complex type: CT_RelativeRect, p. 4545
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_fillRect()
{
    READ_PROLOGUE

//    const QXmlStreamAttributes attrs( attributes() );
//! @todo use ST_Percentage_withMsooxmlFix_to_double for attributes b, l, r, t
    /*    TRY_READ_ATTR_WITHOUT_NS(r, b)
        TRY_READ_ATTR_WITHOUT_NS(r, l)
        TRY_READ_ATTR_WITHOUT_NS(r, r)
        TRY_READ_ATTR_WITHOUT_NS(r, t)*/
//MSOOXML_EXPORT qreal ST_Percentage_withMsooxmlFix_to_double(const QString& val, bool& ok);

    //m_fillImageRenderingStyle = QLatin1String("stretch");
    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL graphic
//! graphic handler (Graphic Object)
/*! ECMA-376, 20.1.2.2.16, p.3037.

 This element specifies the existence of a single graphic object.
 Document authors should refer to this element when they wish to persist
 a graphical object of some kind. The specification for this graphical
 object is provided entirely by the document author and referenced within
 the graphicData child element.

 Parent elements:
 - [done] anchor (§20.4.2.3)
 - graphicFrame (§21.3.2.12)
 - graphicFrame (§20.1.2.2.18)
 - graphicFrame (§20.5.2.16)
 - graphicFrame (§19.3.1.21)
 - [done] inline (§20.4.2.8)

 Child elements:
 - [done] graphicData (Graphic Object Data) §20.1.2.2.17
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_graphic()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(graphicData)
            ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL graphicData
//! graphicData handler (Graphic Object Data)
/*! ECMA-376, 20.1.2.2.17, p.3038.

 This element specifies the reference to a graphic object within the document.
 This graphic object is provided entirely by the document authors who choose
 to persist this data within the document.

 Parent elements:
 - [done] graphic (§20.1.2.2.16)

 Child elements:
 - Any element in any namespace

 Attributes:
 - uri (Uniform Resource Identifier)
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_graphicData()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF_NS(pic, pic)
#ifndef MSOOXMLDRAWINGTABLESTYLEREADER_CPP
            ELSE_TRY_READ_IF_NS(c, chart)
#endif
#ifdef PPTXXMLSLIDEREADER_CPP
            ELSE_TRY_READ_IF_NS(p, oleObj)
            ELSE_TRY_READ_IF_NS(a, tbl)
#endif
//! @todo add ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#define MSOOXML_CURRENT_NS "a" // note: osed only for blipFill namespace is parametrized, can be a or p
#undef CURRENT_EL
#define CURRENT_EL blipFill
//! blipFill handler (Picture Fill)
//! ECMA-376, PresentationML, 19.3.1.4, p. 2818; DrawingML, 20.1.8.14, p. 3195
//! @todo use it in DrawingML, 20.2.2.1, p. 3456
/*! This element specifies the type of picture fill that the picture object has.
 Because a picture has a picture fill already by default, it is possible to have
 two fills specified for a picture object.

 BLIPs refer to Binary Large Image or Pictures. Blip Fills are made up of several components: a Blip
 Reference, a Source Rectangle, and a Fill Mode.
 See also M.4.8.4.3 Blip Fills, ECMA-376, p. 5411.

 Parent elements:
    - bg (§21.4.3.1)
    - bgFillStyleLst (§20.1.4.1.7)
    - [done] bgPr (§19.3.1.2)
    - defRPr (§21.1.2.3.2)
    - endParaRPr (§21.1.2.2.3)
    - fill (§20.1.8.28)
    - fill (§20.1.4.2.9)
    - fillOverlay (§20.1.8.29)
    - fillStyleLst (§20.1.4.1.13)
    - grpSpPr (§21.3.2.14)
    - grpSpPr (§20.1.2.2.22)
    - grpSpPr (§20.5.2.18)
    - grpSpPr (§19.3.1.23)
    - [done] pic (§20.1.2.2.30) - DrawingML
    - [done] pic (§19.3.1.37) - PresentationML
    - [done] rPr (§21.1.2.3.9)
    - spPr (§21.2.2.197)
    - spPr (§21.3.2.23)
    - spPr (§21.4.3.7)
    - spPr (§20.1.2.2.35)
    - spPr (§20.2.2.6)
    - spPr (§20.5.2.30)
    - spPr (§19.3.1.44)
    - tblPr (§21.1.3.15)
    - tcPr (§21.1.3.17)
    - uFill (§21.1.2.3.12)

 Child elements:
    - [done] blip (Blip) §20.1.8.13
    - srcRect (Source Rectangle) §20.1.8.55
    - [done] stretch (Stretch) §20.1.8.56
    - [done] tile (Tile) §20.1.8.58

 Attributes:
    - dpi (DPI Setting)
    - rotWithShape (Rotate With Shape)
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_blipFill(blipFillCaller caller)
{
    kDebug() << "Caller:" << (char)caller;
    // we do not use READ_PROLOGUE because namespace depends on caller here
    PUSH_NAME_INTERNAL
    QString ns;
    // 'p' by default; for dml in docx use 'pic'
#ifdef DOCXXMLDOCREADER_CPP
    if (caller == blipFill_pic) {
        ns = QLatin1String("pic");
    }
    else {
        ns = QChar((char)caller);
    }
#elif defined(XLSXXMLDRAWINGREADER_CPP)
    if (caller == blipFill_pic) {
        ns = QLatin1String("xdr");
    } else {
        ns = QChar((char)caller);
    }
#else
    ns = QChar((char)caller);
#endif
    const QString qn(ns + ":" STRINGIFY(CURRENT_EL));
    if (!expectEl(qn)) {
        return KoFilter::WrongFormat;
    }

    m_fillImageRenderingStyleStretch = false;

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF_QSTRING(qn)
        if (isStartElement()) {
            TRY_READ_IF(blip)
            ELSE_TRY_READ_IF(stretch)
            ELSE_TRY_READ_IF(tile)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    // we do not use READ_EPILOGUE because namespace depends on caller here
    POP_NAME_INTERNAL
    if (!expectElEnd(qn)) {
        kDebug() << "READ_EPILOGUE:" << qn << "not found!";
        return KoFilter::WrongFormat;
    }
    return KoFilter::OK;
}

//! @todo KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_srcRect()
/*
 No child elements.

 Attributes:
    - b (Bottom Offset)
    - l (Left Offset)
    - r (Right Offset)
    - t (Top Offset)

 Complex type: CT_RelativeRect, p. 4545

 const QXmlStreamAttributes attrs( attributes() );
 use qreal ST_Percentage_withMsooxmlFix_to_double(const QString& val, bool& ok)....
*/


#if 0 //todo
#undef CURRENT_EL
#define CURRENT_EL background
//! background handler (Document Background)
/*! ECMA-376, 17.2.1, p. 199.
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_background()
{
}
#endif

// ---- namespace like "a" (pptx) or "wp" (docx)

#undef MSOOXML_CURRENT_NS
#ifndef NO_DRAWINGML_NS
#define MSOOXML_CURRENT_NS DRAWINGML_NS
#endif

void MSOOXML_CURRENT_CLASS::saveStyleWrap(const char * style)
{
    m_currentDrawStyle->addProperty("style:wrap", style, KoGenStyle::GraphicType);
}

void MSOOXML_CURRENT_CLASS::algnToODF(const char * odfEl, const QString& ov)
{
    if (ov.isEmpty())
        return;

    QString v;
    if (ov == QLatin1String("l"))
        v = QLatin1String("start");
    else if (ov == QLatin1String("r"))
        v = QLatin1String("end");
    else if (ov == QLatin1String("just"))
        v = QLatin1String("justify");
    else if (ov == QLatin1String("ctr"))
        v = QLatin1String("center");
    //@todo handle thaiDist, justLow, dist
    if (!v.isEmpty())
        m_currentParagraphStyle.addProperty(odfEl, v);
}

void MSOOXML_CURRENT_CLASS::distToODF(const char * odfEl, const QString& emuValue)
{
    if (emuValue.isEmpty() || emuValue == "0") // skip 0cm which is the default
        return;
    QString s = MSOOXML::Utils::EMU_to_ODF(emuValue);
    if (!s.isEmpty()) {
        m_currentDrawStyle->addProperty(QLatin1String(odfEl), s, KoGenStyle::GraphicType);
    }
}

//! @todo Currently all read_wrap*() uses the same read_wrap(), no idea if they can behave differently
//! CASE #1425
void MSOOXML_CURRENT_CLASS::readWrap()
{
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(wrapText)
    if (wrapText == "bothSides")
        saveStyleWrap("parallel");
    else if (wrapText == "largest")
        saveStyleWrap("dynamic");
    else
        saveStyleWrap(wrapText.toLatin1());
//! @todo Is saveStyleWrap(wrapText) OK?
}

#undef CURRENT_EL
#define CURRENT_EL lstStyle
//! lstStyle handler (Text List Styles) ECMA-376, DrawingML 21.1.2.4.12, p. 3651.
//!          This element specifies the list of styles associated with this body of text.
/*!
 Parent elements:
 - lnDef (§20.1.4.1.20)
 - rich (§21.2.2.156)
 - spDef (§20.1.4.1.27)
 - t (§21.4.3.8)
 - txBody (§21.3.2.26)
 - txBody (§20.1.2.2.40)
 - txBody (§20.5.2.34)
 - [done] txBody (§19.3.1.51)
 - txDef (§20.1.4.1.28)
 - txPr (§21.2.2.216)

 Child elements:
 - defPPr (Default Paragraph Style) §21.1.2.2.2
 - extLst (Extension List) §20.1.2.2.15
 - [done] lvl1pPr (List Level 1 Text Style) §21.1.2.4.13
 - [done] lvl2pPr (List Level 2 Text Style) §21.1.2.4.14
 - [done] lvl3pPr (List Level 3 Text Style) §21.1.2.4.15
 - [done] lvl4pPr (List Level 4 Text Style) §21.1.2.4.16
 - [done] lvl5pPr (List Level 5 Text Style) §21.1.2.4.17
 - [done] lvl6pPr (List Level 6 Text Style) §21.1.2.4.18
 - [done] lvl7pPr (List Level 7 Text Style) §21.1.2.4.19
 - [done] lvl8pPr (List Level 8 Text Style) §21.1.2.4.20
 - [done] lvl9pPr (List Level 9 Text Style) §21.1.2.4.21
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lstStyle()
{
    READ_PROLOGUE
    m_currentListStyle = KoGenStyle(KoGenStyle::ListAutoStyle, "list");

    m_currentCombinedBulletProperties.clear();
    m_currentBulletProperties.clear();
    m_currentCombinedTextStyles.clear();
    m_currentCombinedParagraphStyles.clear();

#ifdef PPTXXMLSLIDEREADER_H
    inheritListStyles();
    // Only slidemaster needs to inherit, this because first there is bodyStyle,
    // then there can be a body frame, the frame must have properties from bodyStyle and it must not
    // overwrite them, where as in case of slide/slideLayout there is no style in their files
    // Note also that we do not inherit defaultStyles, we only save the changes that this lvl creates
    // Default styles are used when we actually create the content
    if (m_context->type == SlideMaster) {
        inheritAllTextAndParagraphStyles();
    }
#endif

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF_NS(a, lvl1pPr)
            ELSE_TRY_READ_IF_NS(a, lvl2pPr)
            ELSE_TRY_READ_IF_NS(a, lvl3pPr)
            ELSE_TRY_READ_IF_NS(a, lvl4pPr)
            ELSE_TRY_READ_IF_NS(a, lvl5pPr)
            ELSE_TRY_READ_IF_NS(a, lvl6pPr)
            ELSE_TRY_READ_IF_NS(a, lvl7pPr)
            ELSE_TRY_READ_IF_NS(a, lvl8pPr)
            ELSE_TRY_READ_IF_NS(a, lvl9pPr)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

#ifdef PPTXXMLSLIDEREADER_H
    saveCurrentListStyles();
    saveCurrentStyles();
#endif

    // Should be zero to not mess anything
    m_currentListLevel = 0;

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL latin
/*! latin handler (Latin Font) ECMA-376, 21.1.2.3.7, p.3621.
 Parent elements:
 - defRPr (§21.1.2.3)
 - endParaRPr (§21.1.2.2.3)
 - font (§20.1.4.2.13)
 - majorFont (§20.1.4.1.24)
 - minorFont (§20.1.4.1.25)
 - [done] rPr (§21.1.2.3.9)

 No child elements.

 Attributes:
 - charset (Similar Character Set)
 - panose (Panose Setting)
 - [incomplete] pitchFamily (Similar Font Family)
 - [done] typeface (Text Typeface)
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_latin()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR_WITHOUT_NS(typeface)

#ifdef PPTXXMLSLIDEREADER_H
    // We skip reading this one properly as we do not know the correct theme in the time of reading
    if (documentReaderMode) {
        defaultLatinFonts[defaultLatinFonts.size() - 1] = typeface;

        skipCurrentElement();
        READ_EPILOGUE
    }
#endif

    if (!typeface.isEmpty()) {
        QString font = typeface;
        if (typeface.startsWith("+mj")) {
            font = m_context->themes->fontScheme.majorFonts.latinTypeface;
        }
        else if (typeface.startsWith("+mn")) {
           font = m_context->themes->fontScheme.minorFonts.latinTypeface;
        }
        m_currentTextStyleProperties->setFontFamily(font);
    }
    TRY_READ_ATTR_WITHOUT_NS(pitchFamily)
    if (!pitchFamily.isEmpty()) {
        int pitchFamilyInt;
        STRING_TO_INT(pitchFamily, pitchFamilyInt, "latin@pitchFamily")
        QFont::StyleHint h = QFont::AnyStyle;
        const int hv = pitchFamilyInt % 0x10;
        switch (hv) {
        case 1: //Roman
            h = QFont::Times;
            break;
        case 2: //Swiss
            h = QFont::SansSerif;
            break;
        case 3: //Modern
            h = QFont::SansSerif;
            //TODO
            break;
        case 4: //Script
            //TODO
            break;
        case 5: //Decorative
            h = QFont::Decorative;
            break;
        }
        const bool fixed = pitchFamilyInt & 0x01; // Fixed Pitch
        m_currentTextStyleProperties->setFontFixedPitch(fixed);
        m_currentTextStyleProperties->setFontStyleHint(h);
    }
    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL highlight
/*! highlight handler  (Highlight Color) ECMA-376, 21.1.2.3.4, p.3616.

 This element specifies the highlight color that is present for a run of text.

 Parent elements:
 - defRPr (§21.1.2.3.2)
 - endParaRPr (§21.1.2.2.3)
 - [done] rPr (§21.1.2.3.9)

 Child elements:
 - hslClr (Hue, Saturation, Luminance Color Model) §20.1.2.3.13
 - prstClr (Preset Color) §20.1.2.3.22
 - [done] schemeClr (Scheme Color) §20.1.2.3.29
 - [done] scrgbClr (RGB Color Model - Percentage Variant) §20.1.2.3.30
 - [done] srgbClr (RGB Color Model - Hex Variant) §20.1.2.3.32
 - [done] sysClr (System Color) §20.1.2.3.33
*/
//! @todo support all elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_DrawingML_highlight()
{
    READ_PROLOGUE2(DrawingML_highlight)

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(schemeClr)
            ELSE_TRY_READ_IF(scrgbClr)
            ELSE_TRY_READ_IF(srgbClr)
            ELSE_TRY_READ_IF(sysClr)
//! @todo add ELSE_WRONG_FORMAT
        }
    }
//    m_currentTextStyleProperties->setBackground(m_currentColor);
    // note: paragraph background is unsupported in presentation applications anyway...
    if (m_currentColor.isValid()) {
        m_currentParagraphStyle.addProperty("fo:background-color", m_currentColor.name());
        m_currentColor = QColor();
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL solidFill
//!Solid Fill
//! DrawingML ECMA-376 20.1.8.54, p. 3234.
/*!
This element especifies a solid color fill.
 Parents:
    - bg (§21.4.3.1)
    - bgFillStyleLst (§20.1.4.1.7)
    - bgPr (§19.3.1.2)
    - defRPr (§21.1.2.3.2)
    - endParaRPr (§21.1.2.2.3)
    - fill (§20.1.8.28)
    - fill (§20.1.4.2.9)
    - fillOverlay (§20.1.8.29)
    - fillStyleLst (§20.1.4.1.13)
    - grpSpPr (§21.3.2.14)
    - grpSpPr (§20.1.2.2.22)
    - grpSpPr (§20.5.2.18)
    - grpSpPr (§19.3.1.23)
    - ln (§20.1.2.2.24)
    - lnB (§21.1.3.5)
    - lnBlToTr (§21.1.3.6)
    - lnL (§21.1.3.7)
    - lnR (§21.1.3.8)
    - lnT (§21.1.3.9)
    - lnTlToBr (§21.1.3.10)
    - [done] rPr (§21.1.2.3.9)
    - spPr (§21.2.2.197)
    - spPr (§21.3.2.23)
    - spPr (§21.4.3.7)
    - [done] spPr (§20.1.2.2.35)
    - spPr (§20.2.2.6)
    - spPr (§20.5.2.30)
    - [done] spPr (§19.3.1.44)
    - tblPr (§21.1.3.15)
    - tcPr (§21.1.3.17)
    - uFill (§21.1.2.3.12)
    - uLn (§21.1.2.3.14)

 Child elements:
    - hslClr (Hue, Saturation, Luminance Color Model) §20.1.2.3.13
    - prstClr (Preset Color) §20.1.2.3.22
    - [done] schemeClr (Scheme Color) §20.1.2.3.29
    - [done] scrgbClr (RGB Color Model - Percentage Variant) §20.1.2.3.30
    - [done] srgbClr (RGB Color Model - Hex Variant) §20.1.2.3.32
    - [done] sysClr (System Color) §20.1.2.3.33

 Attributes:
    None.
*/
//! CASE #P121
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_solidFill()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            //scheme color
            TRY_READ_IF(schemeClr)
//             rgb percentage
            ELSE_TRY_READ_IF(scrgbClr)
            //TODO hslClr hue, saturation, luminecence color
            //TODO prstClr preset color
            ELSE_TRY_READ_IF(srgbClr)
            ELSE_TRY_READ_IF(sysClr)
            //TODO stsClr system color
//! @todo add ELSE_WRONG_FORMAT
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL gradFill
//! Gradient Fill
/*
 Parent Elements:
 - bg (§21.4.3.1);
 - bgFillStyleLst (§20.1.4.1.7);
 - bgPr (§19.3.1.2); defRPr (§21.1.2.3.2);
 - endParaRPr (§21.1.2.2.3);
 - fill (§20.1.8.28);
 - fill (§20.1.4.2.9);
 - fillOverlay (§20.1.8.29);
 - fillStyleLst (§20.1.4.1.13);
 - grpSpPr (§21.3.2.14);
 - grpSpPr (§20.1.2.2.22);
 - grpSpPr (§20.5.2.18);
 - grpSpPr (§19.3.1.23);
 - ln (§20.1.2.2.24);
 - lnB (§21.1.3.5);
 - lnBlToTr (§21.1.3.6);
 - lnL (§21.1.3.7);
 - lnR (§21.1.3.8);
 - lnT (§21.1.3.9);
 - lnTlToBr (§21.1.3.10);
 - rPr (§21.1.2.3.9);
 - [done] spPr (§21.2.2.197);
 - [done] spPr (§21.3.2.23);
 - [done] spPr (§21.4.3.7);
 - [done] spPr (§20.1.2.2.35);
 - [done] spPr (§20.2.2.6);
 - [done] spPr (§20.5.2.30);
 - [done] spPr (§19.3.1.44);
 - tblPr (§21.1.3.15);
 - tcPr (§21.1.3.17);
 - uFill (§21.1.2.3.12);
 - uLn (§21.1.2.3.14)

 Child Elements:
 - [done] gsLst (Gradient Stop List) §20.1.8.37
 - lin (Linear Gradient Fill) §20.1.8.41
 - path (Path Gradient) §20.1.8.46
 - tileRect (Tile Rectangle) §20.1.8.59

*/
//! @todo support this properly
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_gradFill()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    m_gradRotation = false;
    m_gradPosition = 0;

    TRY_READ_ATTR_WITHOUT_NS(rotWithShape)
    if (rotWithShape == "1") {
        m_gradRotation = true;
    }

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(gsLst)
        }
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL gsLst
//! gradient stop list
/*
 Parent Elements:
 - [done] gradFill (§20.1.8.33)

 Child Elements:
 - [done] gs (Gradient stops) §20.1.8.36

*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_gsLst()
{
    READ_PROLOGUE

    QVector<QColor> colors;
    QVector<int> positions;
    QVector<int> alphas;

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            if (QUALIFIED_NAME_IS(gs)) {
                TRY_READ(gs)
                colors.push_back(m_currentColor);
                positions.push_back(m_gradPosition);
                alphas.push_back(m_currentAlpha);
            }
        }
    }

    bool gradFilled = false;
    // This gradient logic should be replace with a more generic one if possible
    if (colors.size() == 3) {
        // Case: axial gradient
        if (positions.at(0) == 0 && positions.at(1) == 50  && positions.at(2) == 100 &&
            colors.at(0) == colors.at(2) && colors.at(0) != colors.at(1)) {
            m_currentGradientStyle.addAttribute("draw:style", "axial");
            m_currentGradientStyle.addAttribute("draw:end-color", colors.at(0).name());
            if (alphas.at(0) > 0) {
                m_currentGradientStyle.addAttribute("draw:start-intensity", QString("%1%").arg(alphas.at(0)));
            }
            else {
                m_currentGradientStyle.addAttribute("draw:start-intensity", "100%");
            }
            if (alphas.at(2) > 0) {
                m_currentGradientStyle.addAttribute("draw:end-intensity", QString("%1%").arg(alphas.at(0)));
            }
            else {
                m_currentGradientStyle.addAttribute("draw:end-intensity", "100%");
            }
            m_currentGradientStyle.addAttribute("draw:start-color", colors.at(1).name());
            gradFilled = true;
        }
    }
    // Currently used for all other encountered gradient types
    if (colors.size() > 1 && !gradFilled) {
        m_currentGradientStyle.addAttribute("draw:style", "linear");
        if (alphas.at(0) > 0) {
            m_currentGradientStyle.addAttribute("draw:start-intensity", QString("%1%").arg(alphas.at(0)));
        }
        else {
            m_currentGradientStyle.addAttribute("draw:start-intensity", "100%");
        }
        if (alphas.at(alphas.size()-1) > 0) {
            m_currentGradientStyle.addAttribute("draw:end-intensity", QString("%1%").arg(alphas.at(alphas.size()-1)));
        }
        else {
            m_currentGradientStyle.addAttribute("draw:end-intensity", "100%");
        }
        m_currentGradientStyle.addAttribute("draw:start-color", colors.at(0).name());
        m_currentGradientStyle.addAttribute("draw:end-color", colors.at(colors.size()-1).name());
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL gs
//! gradient stops
/*
 Parent Elements:
 - [done] gsLst (§20.1.8.37)

 Child Elements:
 - hslClr (Hue, Saturation, Luminance Color Model) §20.1.2.3.13
 - prstClr (Preset Color) §20.1.2.3.22
 - [done] schemeClr (Scheme Color) §20.1.2.3.29
 - [done] scrgbClr (RGB Color Model - Percentage Variant) §20.1.2.3.30
 - [done] srgbClr (RGB Color Model - Hex Variant) §20.1.2.3.32
 - [done] sysClr (System Color) §20.1.2.3.33

*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_gs()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(pos)

    m_gradPosition = pos.toInt() / 1000;

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(schemeClr)
            ELSE_TRY_READ_IF(srgbClr)
            ELSE_TRY_READ_IF(sysClr)
            ELSE_TRY_READ_IF(scrgbClr)
        }
    }
    READ_EPILOGUE
}

//noFill 20.1.8.44
/*This element specifies No fill.
Parents:
    - bg (§21.4.3.1)
    - bgFillStyleLst (§20.1.4.1.7)
    - bgPr (§19.3.1.2)
    - defRPr (§21.1.2.3.2)
    - endParaRPr (§21.1.2.2.3)
    - fill (§20.1.8.28)
    - fill (§20.1.4.2.9)
    - fillOverlay (§20.1.8.29)
    - fillStyleLst (§20.1.4.1.13)
    - grpSpPr (§21.3.2.14)
    - grpSpPr (§20.1.2.2.22)
    - grpSpPr (§20.5.2.18)
    - grpSpPr (§19.3.1.23)
    - ln (§20.1.2.2.24)
    - lnB (§21.1.3.5)
    - lnBlToTr(§21.1.3.6)
    - lnL (§21.1.3.7)
    - lnR (§21.1.3.8)
    - lnT (§21.1.3.9)
    - lnTlToBr (§21.1.3.10)
    - [done] rPr (§21.1.2.3(§21.2.2.197)
    - spPr (§21.3.2.23)
    - spPr (§21.4.3.7)
    - spPr (§20.1.2.2.35)
    - spPr (§20.2.2.6)
    - spPr (§20(§19.3.1.44) 
    - tblPr (§21.1.3.15)
    - tcPr (§21.1.3.17)
    - uFill (§21.1.2.3.12)
    - uLn (§21.1.2.3.14)
*/
#undef CURRENT_EL
#define CURRENT_EL noFill
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_noFill(noFillCaller caller)
{
    READ_PROLOGUE
    if (caller == noFill_rPr) {
       m_currentTextStyleProperties->setTextOutline(QPen(Qt::SolidLine));
    }
    readNext();
    READ_EPILOGUE
}

// prstGeom (preset geometry)
/*
 Parent elements:
 - [done] spPr (§21.2.2.197)
 - [done] spPr (§21.3.2.23)
 - [done] spPr (§21.4.3.7)
 - [done] spPr (§20.1.2.2.35)
 - [done] spPr (§20.2.2.6)
 - [done] spPr (§20.5.2.30)
 - [done] spPr (§19.3.1.44)

 Child elements:
 - avLst (List of Shape Adjust Values) §20.1.9.5

*/
#undef CURRENT_EL
#define CURRENT_EL prstGeom
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_prstGeom()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(prst)
    m_contentType = prst;

    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
    }

    READ_EPILOGUE
}

/*
This element specifies a color bound to a user's theme. As with all
elements which define a color, it is possible to apply a list of color
transforms to the base color defined.

Parent Elements
  - accent1 (§20.1.4.1.1)
  - accent2 (§20.1.4.1.2)
  - accent3 (§20.1.4.1.3)
  - accent4 (§20.1.4.1.4)
  - accent5 (§20.1.4.1.5)
  - accent6 (§20.1.4.1.6)
  - alphaInv (§20.1.8.4)
  - bgClr (§20.1.8.10)
  - bgRef (§19.3.1.3)
  - buClr (§21.1.2.4.4)
  - clrFrom (§20.1.8.17)
  - clrMru (§19.2.1.4)
  - clrRepl (§20.1.8.18)
  - clrTo (§20.1.8.19)
  - clrVal (§19.5.27)
  - contourClr (§20.1.5.6)
  - custClr (§20.1.4.1.8)
  - dk1 (§20.1.4.1.9)
  - dk2 (§20.1.4.1.10)
  - duotone (§20.1.8.23)
  - effectClrLst (§21.4.4.7)
  - effectRef (§20.1.4.2.8)
  - extrusionClr (§20.1.5.7)
  - fgClr (§20.1.8.27)
  - fillClrLst (§21.4.4.8)
  - fillRef (§20.1.4.2.10)
  - folHlink (§20.1.4.1.15)
  - fontRef (§20.1.4.1.17)
  - from (§19.5.43)
  - glow (§20.1.8.32)
  - gs (§20.1.8.36)
  - highlight (§21.1.2.3.4)
  - hlink (§20.1.4.1.19)
  - innerShdw (§20.1.8.40)
  - linClrLst (§21.4.4.9)
  - lnRef (§20.1.4.2.19)
  - lt1 (§20.1.4.1.22)
  - lt2 (§20.1.4.1.23)
  - outerShdw (§20.1.8.45)
  - penClr (§19.2.1.23)
  - prstShdw (§20.1.8.49)
  - [done] solidFill (§20.1.8.54)
  - tcTxStyle (§20.1.4.2.30)
  - to (§19.5.90)
  - txEffectClrLst (§21.4.4.12)
  - txFillClrLst (§21.4.4.13)
  - txLinClrLst (§21.4.4.14)

Child elements:
  - [done] alpha (Alpha) §20.1.2.3.1
  - alphaMod (Alpha Modulation) §20.1.2.3.2
  - alphaOff (Alpha Offset) §20.1.2.3.3
  - blue (Blue) §20.1.2.3.4
  - blueMod (Blue Modification) §20.1.2.3.5
  - blueOff (Blue Offset) §20.1.2.3.6
  - comp (Complement) §20.1.2.3.7
  - gamma (Gamma) §20.1.2.3.8
  - gray (Gray) §20.1.2.3.9
  - green (Green) §20.1.2.3.10
  - greenMod (Green Modification) §20.1.2.3.11
  - greenOff (Green Offset) §20.1.2.3.12
  - hue (Hue) §20.1.2.3.14
  - hueMod (Hue Modulate) §20.1.2.3.15
  - hueOff (Hue Offset) §20.1.2.3.16
  - inv (Inverse) §20.1.2.3.17
  - invGamma (Inverse Gamma) §20.1.2.3.18
  - lum (Luminance) §20.1.2.3.19
  - [done] lumMod (Luminance Modulation) §20.1.2.3.20
  - [done] lumOff (Luminance Offset) §20.1.2.3.21
  - red (Red) §20.1.2.3.23
  - redMod (Red Modulation) §20.1.2.3.24
  - redOff (Red Offset) §20.1.2.3.25
  - sat (Saturation) §20.1.2.3.26
  - [done] satMod (Saturation Modulation) §20.1.2.3.27
  - satOff (Saturation Offset) §20.1.2.3.28
  - shade (Shade) §20.1.2.3.31
  - [done] tint (Tint) §20.1.2.3.34

Attributes
  - val (Value)    Specifies the desired scheme.
 */
#undef CURRENT_EL
#define CURRENT_EL schemeClr
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_schemeClr()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());
    READ_ATTR_WITHOUT_NS(val)

#ifdef PPTXXMLSLIDEREADER_H
    // We skip reading this one properly as we do not know the correct theme in the time of reading
    if (documentReaderMode) {
        defaultTextColors[defaultTextColors.size() - 1] = val;

        skipCurrentElement();
        READ_EPILOGUE
    }
#endif

    m_currentTint = 0;
    m_currentShadeLevel = 0;
    m_currentSatMod = 0;
    m_currentAlpha = 0;

    MSOOXML::DrawingMLColorSchemeItemBase *colorItem = 0;

#ifdef PPTXXMLSLIDEREADER_H

    QString valTransformed = m_context->colorMap.value(val);
    colorItem = m_context->themes->colorScheme.value(valTransformed);
#else
    // This should most likely be checked from a color map, see above
    colorItem = m_context->themes->colorScheme.value(val);
#endif
    // Parse the child elements
    MSOOXML::Utils::DoubleModifier lumMod;
    MSOOXML::Utils::DoubleModifier lumOff;
    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        // @todo: Hmm, are these color modifications only available for pptx?
        if (QUALIFIED_NAME_IS(lumMod)) {
            m_currentDoubleValue = &lumMod.value;
            TRY_READ(lumMod)
            lumMod.valid = true;
        } else if (QUALIFIED_NAME_IS(lumOff)) {
            m_currentDoubleValue = &lumOff.value;
            TRY_READ(lumOff)
            lumOff.valid = true;
        }
        ELSE_TRY_READ_IF(shade)
        ELSE_TRY_READ_IF(tint)
        ELSE_TRY_READ_IF(satMod)
        ELSE_TRY_READ_IF(alpha)
    }

    QColor col = Qt::white;
    if (colorItem) {
        col = colorItem->value();
    }

    col = MSOOXML::Utils::colorForLuminance(col, lumMod, lumOff);

#ifdef MSOOXMLDRAWINGTABLESTYLEREADER_CPP
    m_currentPen.setColor(col);
#endif
    m_currentColor = col;

    MSOOXML::Utils::modifyColor(m_currentColor, m_currentTint, m_currentShadeLevel, m_currentSatMod);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL satMod
//! satMod (Saturation modulation value)
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_satMod()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(val)

    if (!val.isEmpty()) {
        bool ok = false;
        int value = val.toInt(&ok);
        if (!ok) {
            value = 0;
        }
        m_currentSatMod = value/100000.0; // To get percentage in from 0.x
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL alpha
//! alpha (alpha value)
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_alpha()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(val)

    if (!val.isEmpty()) {
        bool ok = false;
        int value = val.toInt(&ok);
        if (!ok) {
            value = 0;
        }
        m_currentAlpha = value/1000; // To get percentage
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL tint
//! tint (tint value)
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_tint()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(val)

    if (!val.isEmpty()) {
        bool ok = false;
        int value = val.toInt(&ok);
        if (!ok) {
            value = 0;
        }
        m_currentTint = value/100000.0; // To get percentage (form 0.x)
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL shade
//! shade (shade value)
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_shade()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    TRY_READ_ATTR_WITHOUT_NS(val)

    if (!val.isEmpty()) {
        bool ok = false;
        int value = val.toInt(&ok);
        if (!ok) {
            value = 0;
        }
        m_currentShadeLevel = value/100000.0; // To get percentage (form 0.x)
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lumMod
//! 20.1.2.3.20 lumMod (Luminance Modulation)
//! This element specifies the input color with its luminance modulated by the given percentage.
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lumMod()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    READ_ATTR_WITHOUT_NS(val)

    bool ok;
    Q_ASSERT(m_currentDoubleValue);
    *m_currentDoubleValue = MSOOXML::Utils::ST_Percentage_withMsooxmlFix_to_double(val, ok);
    if (!ok)
        return KoFilter::WrongFormat;

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lumOff
//! 20.1.2.3.21 lumOff (Luminance Offset)
//! This element specifies the input color with its luminance shifted, but with its hue and saturation unchanged.
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lumOff()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    READ_ATTR_WITHOUT_NS(val)

    bool ok;
    Q_ASSERT(m_currentDoubleValue);
    *m_currentDoubleValue = MSOOXML::Utils::ST_Percentage_withMsooxmlFix_to_double(val, ok);
    if (!ok)
        return KoFilter::WrongFormat;

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL ln
//! Outline
//! DrawingML ECMA-376, 20.1.2.2.24, p. 3048.
/*! This element specifies an outline style that can be applied to a
    number of different objects such as shapes and text.

 Child elements:
    - bevel (Line Join Bevel) §20.1.8.9
    - custDash (Custom Dash) §20.1.8.21
    - extLst (Extension List) §20.1.2.2.15
    - gradFill (Gradient Fill) §20.1.8.33
    - headEnd (Line Head/End Style) §20.1.8.38
    - miter (Miter Line Join) §20.1.8.43
    - noFill (No Fill) §20.1.8.44
    - pattFill (Pattern Fill) §20.1.8.47
    - [done] prstDash (Preset Dash) §20.1.8.48
    - round (Round Line Join) §20.1.8.52
    - solidFill (Solid Fill) §20.1.8.54
    - tailEnd (Tail line end style) §20.1.8.57

 Attributes:
    - algn
    - cap
    - cmpd
    - w
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_ln()
{
    READ_PROLOGUE
    QXmlStreamAttributes attrs(attributes());

    m_currentPen = QPen();

    //align
    TRY_READ_ATTR_WITHOUT_NS(algn)
    //center
    if (algn.isEmpty() || algn == "ctr") {
    }
    //inset
    else if(algn == "in") {
    }

    //line ending cap
    TRY_READ_ATTR_WITHOUT_NS(cap)
    Qt::PenCapStyle penCap = m_currentPen.capStyle();
    //flat
    if (cap.isEmpty() || cap == "sq") {
       penCap = Qt::SquareCap;
    }
    //round
    else if (cap == "rnd") {
        penCap = Qt::RoundCap;
    }
    //square
    else if (cap == "flat") {
        penCap = Qt::FlatCap;
    }
    m_currentPen.setCapStyle(penCap);

    //TODO
    //compound line type
    TRY_READ_ATTR_WITHOUT_NS(cmpd)
    //double lines
    if( cmpd.isEmpty() || cmpd == "sng" ) {
    }
    //single line
    else if (cmpd == "dbl") {
    }
    //thick thin double lines
    else if (cmpd == "thickThin") {
    }
    //thin thick double lines
    else if (cmpd == "thinThick") {
    }
    //thin thick thin triple lines
    else if (cmpd == "tri") {
    }

    TRY_READ_ATTR_WITHOUT_NS(w) //width
    if(w.isEmpty()) {
        w = "0";
    }
    qreal penWidth = EMU_TO_POINT(w.toDouble());

    m_currentPen.setWidthF(penWidth);

    bool colorRead = false;

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if( isStartElement() ) {
            //Line join bevel
//             if(qualifiedName() == QLatin1String("a:bevel")) {
//                 TRY_READ()
//             }
//             //custom dash
//             else if(qualifiedName() == QLatin1String("a:custDash")) {
//             }
//             //extension list
//             else if(qualifiedName() == QLatin1String("a:extLst")) {
//             }
//             //gradient fill
//             else if(qualifiedName() == QLatin1String("a:gradFill")) {
//             }
//             //line head/end style
//             else if(qualifiedName() == QLatin1String("a:headEnd")) {
//             }
//             //miter line join
//             else if(qualifiedName() == QLatin1String("a:miter")) {
//             }
//             //no fill
//             else if(qualifiedName() == QLatin1String("a:noFill")) {
//             }
//             //pattern fill
//             else if(qualifiedName() == QLatin1String("a:pattFill")) {
//             }
//             //round line join
//             else if(qualifiedName() == QLatin1String("a:round")) {
//             }
            //solid fill
            if (qualifiedName() == QLatin1String("a:solidFill")) {
                TRY_READ(solidFill)
                colorRead = true;
            }
            else if (qualifiedName() == QLatin1String("a:prstDash")) {
                attrs = attributes();
                TRY_READ_ATTR_WITHOUT_NS(val)
                if (val == "dash") {
                    m_currentPen.setStyle(Qt::DashLine);
                }
            }
            //tail line end style
//             else if(qualifiedName() == QLatin1String("a:tailEnd")) {
//             }
        }
    }

    m_currentPen.setColor(m_currentColor);

    // No color means that it should not have outline
    if (!colorRead) {
        m_currentPen = QPen();
    }

    KoOdfGraphicStyles::saveOdfStrokeStyle(*m_currentDrawStyle, *mainStyles, m_currentPen);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL scrgbClr
//! RGB Color Model - Percentage Variant
//! DrawingML ECMA-376 20.1.2.3.30, p. 3074.
/*!
This element specifies a solid color fill.

 Child elements:
    - [done] alpha (Alpha) §20.1.2.3.1
    - alphaMod (Alpha Modulation) §20.1.2.3.2
    - alphaOff (Alpha Offset) §20.1.2.3.3
    - blue (Blue) §20.1.2.3.4
    - blueMod (Blue Modification) §20.1.2.3.5
    - blueOff (Blue Offset) §20.1.2.3.6
    - comp (Complement) §20.1.2.3.7
    - gamma (Gamma) §20.1.2.3.8
    - gray (Gray) §20.1.2.3.9
    - green (Green) §20.1.2.3.10
    - greenMod (Green Modification) §20.1.2.3.11
    - greenOff (Green Offset) §20.1.2.3.12
    - hue (Hue) §20.1.2.3.14
    - hueMod (Hue Modulate) §20.1.2.3.15
    - hueOff (Hue Offset) §20.1.2.3.16
    - [done] inv (Inverse) §20.1.2.3.17
    - invGamma (Inverse Gamma) §20.1.2.3.18
    - lum (Luminance) §20.1.2.3.19
    - lumMod (Luminance Modulation) §20.1.2.3.20
    - lumOff (Luminance Offset) §20.1.2.3.21
    - red (Red) §20.1.2.3.23
    - redMod (Red Modulation) §20.1.2.3.24
    - redOff (Red Offset) §20.1.2.3.25

 Attributes:
    - [done] b (blue)
    - [done] g (green)
    - [done] r (red)
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_scrgbClr()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    m_currentTint = 0;
    m_currentShadeLevel = 0;
    m_currentSatMod = 0;
    m_currentAlpha = 0;

    READ_ATTR_WITHOUT_NS(r)
    READ_ATTR_WITHOUT_NS(g)
    READ_ATTR_WITHOUT_NS(b)

    bool okR;
    bool okG;
    bool okB;

    m_currentColor = QColor::fromRgbF(qreal(MSOOXML::Utils::ST_Percentage_to_double(r, okR)),
                                      qreal(MSOOXML::Utils::ST_Percentage_to_double(g, okG)),
                                      qreal(MSOOXML::Utils::ST_Percentage_to_double(b, okB)));

    //TODO: all the color transformations
    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(tint)
            ELSE_TRY_READ_IF(alpha)
        }
    }

    MSOOXML::Utils::modifyColor(m_currentColor, m_currentTint, m_currentShadeLevel, m_currentSatMod);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL srgbClr
//! RGB Color Model -Hex Digit Variant
//! DrawingML ECMA-376 20.1.2.3.32, p. 3085.
/*!
This element specifies a color in RGB notation.

 Child elements:
    - alpha (Alpha) §20.1.2.3.1
    - alphaMod (Alpha Modulation) §20.1.2.3.2
    - alphaOff (Alpha Offset) §20.1.2.3.3
    - blue (Blue) §20.1.2.3.4
    - blueMod (Blue Modification) §20.1.2.3.5
    - blueOff (Blue Offset) §20.1.2.3.6
    - comp (Complement) §20.1.2.3.7
    - gamma (Gamma) §20.1.2.3.8
    - gray (Gray) §20.1.2.3.9
    - green (Green) §20.1.2.3.10
    - greenMod (Green Modification) §20.1.2.3.11
    - greenOff (Green Offset) §20.1.2.3.12
    - hue (Hue) §20.1.2.3.14
    - hueMod (Hue Modulate) §20.1.2.3.15
    - hueOff (Hue Offset) §20.1.2.3.16
    - inv (Inverse) §20.1.2.3.17
    - invGamma (Inverse Gamma) §20.1.2.3.18
    - lum (Luminance) §20.1.2.3.19
    - lumMod (Luminance Modulation) §20.1.2.3.20
    - lumOff (Luminance Offset) §20.1.2.3.21
    - red (Red) §20.1.2.3.23
    - redMod (Red Modulation) §20.1.2.3.24
    - redOff (Red Offset) §20.1.2.3.25
    - sat (Saturation) §20.1.2.3.26
    - [done] satMod (Saturation Modulation) §20.1.2.3.27
    - satOff (Saturation Offset) §20.1.2.3.28
    - [done] shade (Shade) §20.1.2.3.31
    - [done] tint (Tint) §20.1.2.3.34
 Attributes:
    - [done] val ("RRGGBB" hex digits)
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_srgbClr()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    m_currentTint = 0;
    m_currentShadeLevel = 0;
    m_currentSatMod = 0;
    m_currentAlpha = 0;

    READ_ATTR_WITHOUT_NS(val)

    m_currentColor = QColor( QLatin1Char('#') + val );

    //TODO: all the color transformations
    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(tint)
            ELSE_TRY_READ_IF(shade)
            ELSE_TRY_READ_IF(satMod)
            ELSE_TRY_READ_IF(alpha)
        }
    }

    MSOOXML::Utils::modifyColor(m_currentColor, m_currentTint, m_currentShadeLevel, m_currentSatMod);

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL sysClr
//! sysClr handler
// SysClr is bit controversial, it is supposed to use
// color defined by the system at the moment, the document is read
// however, it often means that when reading the document, it is not
// using the same colors, the creater wished.
// Sometimes sysclr saves attribue lastClr which tells which color
// the creator was using, the current implementation uses that
// and ignores real system colors.
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_sysClr()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    m_currentTint = 0;
    m_currentShadeLevel = 0;
    m_currentSatMod = 0;
    m_currentAlpha = 0;

    TRY_READ_ATTR_WITHOUT_NS(lastClr)

    if (!lastClr.isEmpty()) {
        m_currentColor = QColor( QLatin1Char('#') + lastClr );
    }

    //TODO: all the color transformations
    while (true) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(tint)
            ELSE_TRY_READ_IF(shade)
            ELSE_TRY_READ_IF(satMod)
            ELSE_TRY_READ_IF(alpha)
        }
    }

    MSOOXML::Utils::modifyColor(m_currentColor, m_currentTint, m_currentShadeLevel, m_currentSatMod);

    READ_EPILOGUE
}

KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::lvlHelper(const QString& level) {

    const QXmlStreamAttributes attrs(attributes());

    Q_ASSERT(m_currentTextStyleProperties == 0);
    m_currentTextStyleProperties = new KoCharacterStyle();

    // Number 3 makes eg. lvl4 -> 4
    m_currentListLevel = QString(level.at(3)).toInt();

    Q_ASSERT(m_currentListLevel > 0);
    m_currentBulletProperties.m_level = m_currentListLevel;

    TRY_READ_ATTR_WITHOUT_NS(marL)
    TRY_READ_ATTR_WITHOUT_NS(marR)
    TRY_READ_ATTR_WITHOUT_NS(indent)
    TRY_READ_ATTR_WITHOUT_NS(defTabSz)

    bool ok = false;

    m_currentParagraphStyle = KoGenStyle(KoGenStyle::ParagraphAutoStyle, "text");

    if (!marR.isEmpty()) {
        const qreal marginal = qreal(EMU_TO_POINT(marR.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("fo:margin-right", marginal);
    }
    if (!marL.isEmpty()) {
        const qreal marginal = qreal(EMU_TO_POINT(marL.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("fo:margin-left", marginal);
    }
    if (!indent.isEmpty()) {
        const qreal firstInd = qreal(EMU_TO_POINT(indent.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("fo:text-indent", firstInd);
    }
    if (!defTabSz.isEmpty()) {
        const qreal tabSize = qreal(EMU_TO_POINT(defTabSz.toDouble(&ok)));
        m_currentParagraphStyle.addPropertyPt("style:tab-stop-distance", tabSize);
    }

    TRY_READ_ATTR_WITHOUT_NS(algn)
    algnToODF("fo:text-align", algn);

    m_currentTextStyle = KoGenStyle(KoGenStyle::TextAutoStyle, "text");

    m_bulletFont = "";

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        if (isEndElement() && qualifiedName() == QString("a:%1").arg(level)) {
            break;
        }
        if (isStartElement()) {
            TRY_READ_IF(defRPr) // fills m_currentTextStyleProperties
            ELSE_TRY_READ_IF(buNone)
            ELSE_TRY_READ_IF(buAutoNum)
            ELSE_TRY_READ_IF(buChar)
            ELSE_TRY_READ_IF(buFont)
            ELSE_TRY_READ_IF(buBlip)
            else if (QUALIFIED_NAME_IS(spcBef)) {
                m_currentSpacingType = spacingMarginTop;
                TRY_READ(spcBef)
            }
            else if (QUALIFIED_NAME_IS(spcAft)) {
                m_currentSpacingType = spacingMarginBottom;
                TRY_READ(spcAft)
            }
            else if (QUALIFIED_NAME_IS(lnSpc)) {
                m_currentSpacingType = spacingLines;
                TRY_READ(lnSpc)
            }
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    if (m_bulletFont == "Wingdings") {
        // Ooxml files have very often wingdings fonts, but usually they are not installed
        // Making the bullet character look ugly, thus defaulting to "-"
        m_currentBulletProperties.setBulletChar("-");
        m_lstStyleFound = true;
        m_listStylePropertiesAltered = true;
    }

    m_currentTextStyleProperties->saveOdf(m_currentTextStyle);

    m_currentCombinedParagraphStyles[m_currentListLevel] = m_currentParagraphStyle;
    m_currentCombinedTextStyles[m_currentListLevel] = m_currentTextStyle;
    m_currentCombinedBulletProperties[m_currentListLevel] = m_currentBulletProperties;

    delete m_currentTextStyleProperties;
    m_currentTextStyleProperties = 0;

    return KoFilter::OK;
}

#undef CURRENT_EL
#define CURRENT_EL lvl1pPr
//! List level 1 text style
/*!

 Parent elements:
  - [done] bodyStyle (§19.3.1.5)
  - defaultTextStyle (§19.2.1.8)
  - [done] lstStyle (§21.1.2.4.12)
  - notesStyle (§19.3.1.28)
  - [done] otherStyle (§19.3.1.35)
  - [done] titleStyle (§19.3.1.49)

 Child elements:
  - [done] buAutoNum (Auto-Numbered Bullet)     §21.1.2.4.1
  - [done] buBlip (Picture Bullet)              §21.1.2.4.2
  - [done] buChar (Character Bullet)            §21.1.2.4.3
  - buClr (Color Specified)              §21.1.2.4.4
  - buClrTx (Follow Text)                §21.1.2.4.5
  - [done] buFont (Specified)                   §21.1.2.4.6
  - buFontTx (Follow text)               §21.1.2.4.7
  - [done] buNone (No Bullet)                   §21.1.2.4.8
  - buSzPct (Bullet Size Percentage)     §21.1.2.4.9
  - buSzPts (Bullet Size Points)         §21.1.2.4.10
  - buSzTx (Bullet Size Follows Text)    §21.1.2.4.11
  - [done] defRPr (Default Text Run Properties) §21.1.2.3.2
  - extLst (Extension List)              §20.1.2.2.15
  - [done] lnSpc (Line Spacing)                 §21.1.2.2.5
  - [done] spcAft (Space After)                 §21.1.2.2.9
  - [done] spcBef (Space Before)                §21.1.2.2.10
  - tabLst (Tab List)                    §21.1.2.2.14

*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl1pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl1pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl2pPr
//! Look for lvl1pPr documentation
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl2pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl2pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl3pPr
//! Look for lvl1pPr documentation  
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl3pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl3pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl4pPr
//! Look for lvl1pPr documentation
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl4pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl4pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl5pPr
//! Look for lvl1pPr documentation
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl5pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl5pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl6pPr
//! Look for lvl1pPr documentation
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl6pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl6pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl7pPr
//! Look for lvl1pPr documentation
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl7pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl7pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl8pPr
//! Look for lvl1pPr documentation
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl8pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl8pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lvl9pPr
//! Look for lvl1pPr documentation
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lvl9pPr()
{
    READ_PROLOGUE
    lvlHelper("lvl9pPr");
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL buBlip
//! buBlip - bullet picture
/*!
 Parent elements:
 - defPPr (§21.1.2.2.2)
 - [done] lvl1pPr (§21.1.2.4.13)
 - [done] lvl2pPr (§21.1.2.4.14)
 - [done] lvl3pPr (§21.1.2.4.15)
 - [done] lvl4pPr (§21.1.2.4.16)
 - [done] lvl5pPr (§21.1.2.4.17)
 - [done] lvl6pPr (§21.1.2.4.18)
 - [done] lvl7pPr (§21.1.2.4.19)
 - [done] lvl8pPr (§21.1.2.4.20)
 - [done] lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)

 Child elements:
 - [done] blip
*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_buBlip()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    m_xlinkHref.clear();

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(blip)
        }
    }

    if (!m_xlinkHref.isEmpty()) {
        m_currentBulletProperties.setPicturePath(m_xlinkHref);
        m_currentBulletProperties.setPictureSize(m_imageSize);
        m_lstStyleFound = true;
        m_listStylePropertiesAltered = true;
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL buChar
//! buChar - bullet character
/*!
 Parent elements:
 - defPPr (§21.1.2.2.2)
 - [done] lvl1pPr (§21.1.2.4.13)
 - [done] lvl2pPr (§21.1.2.4.14)
 - [done] lvl3pPr (§21.1.2.4.15)
 - [done] lvl4pPr (§21.1.2.4.16)
 - [done] lvl5pPr (§21.1.2.4.17)
 - [done] lvl6pPr (§21.1.2.4.18)
 - [done] lvl7pPr (§21.1.2.4.19)
 - [done] lvl8pPr (§21.1.2.4.20)
 - [done] lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)
*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_buChar()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    if (attrs.hasAttribute("char")) {
        m_currentBulletProperties.setBulletChar(attrs.value("char").toString());
        // if such a char is defined then we have actually a list-item even if OOXML doesn't handle them as such
        m_lstStyleFound = true;
    }

    m_listStylePropertiesAltered = true;

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL buFont
//! buFont - bullet font
/*!
 Parent elements:
 - defPPr (§21.1.2.2.2)
 - [done] lvl1pPr (§21.1.2.4.13)
 - [done] lvl2pPr (§21.1.2.4.14)
 - [done] lvl3pPr (§21.1.2.4.15)
 - [done] lvl4pPr (§21.1.2.4.16)
 - [done] lvl5pPr (§21.1.2.4.17)
 - [done] lvl6pPr (§21.1.2.4.18)
 - [done] lvl7pPr (§21.1.2.4.19)
 - [done] lvl8pPr (§21.1.2.4.20)
 - [done] lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)
*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_buFont()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR_WITHOUT_NS(typeface)

    if (!typeface.isEmpty()) {
        m_bulletFont = typeface;
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL fld
//! fld - Text Field
/*!
 Parent elements:
 - [done] p (§21.1.2.2.6)

 Child elements:

 - [done] pPr (Text Paragraph Properties) §21.1.2.2.7
 - [done] rPr (Text Run Properties) §21.1.2.3.9
 - [done] t (Text String) §21.1.2.3.11

*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_fld()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR(type)

    m_currentTextStyleProperties = new KoCharacterStyle();
    m_currentTextStyle = KoGenStyle(KoGenStyle::TextAutoStyle, "text");

    if (!type.isEmpty()) {
//! @todo support all possible fields here
    }

    MSOOXML::Utils::XmlWriteBuffer fldBuf;
    body = fldBuf.setWriter(body);

#ifdef PPTXXMLSLIDEREADER_H
    inheritDefaultTextStyle(m_currentTextStyle);
    inheritTextStyle(m_currentTextStyle);
#endif

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            if (QUALIFIED_NAME_IS(rPr)) {
                TRY_READ(DrawingML_rPr)
            }
            else if (QUALIFIED_NAME_IS(pPr)) {
                TRY_READ(DrawingML_pPr)
            }
            ELSE_TRY_READ_IF(t)
        }
    }

    m_currentTextStyleProperties->saveOdf(m_currentTextStyle);
    const QString currentTextStyleName(mainStyles->insert(m_currentTextStyle));
#ifdef PPTXXMLSLIDEREADER_H
    if (m_context->type == SlideMaster) {
        mainStyles->markStyleForStylesXml(currentTextStyleName);
    }
#endif

    body = fldBuf.originalWriter();

    body->startElement("text:span", false);
    body->addAttribute("text:style-name", currentTextStyleName);

    (void)fldBuf.releaseWriter();

    body->endElement(); //text:span

    delete m_currentTextStyleProperties;
    m_currentTextStyleProperties = 0;

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL spcBef
//! spcBef - spacing before
/*!
 Parent elements:

 - defPPr (§21.1.2.2.2)
 - [done] lvl1pPr (§21.1.2.4.13)
 - [done] lvl2pPr (§21.1.2.4.14)
 - [done] lvl3pPr (§21.1.2.4.15)
 - [done] lvl4pPr (§21.1.2.4.16)
 - [done] lvl5pPr (§21.1.2.4.17)
 - [done] lvl6pPr (§21.1.2.4.18)
 - [done] lvl7pPr (§21.1.2.4.19)
 - [done] lvl8pPr (§21.1.2.4.20)
 - [done] lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)

 Child elements:

 - [done] spcPct (Spacing Percent) §21.1.2.2.11
 - [done] spcPts (Spacing Points)  §21.1.2.2.12

*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_spcBef()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(spcPts)
            ELSE_TRY_READ_IF(spcPct)
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL spcAft
//! spcAft - spacing after
/*!
 Parent elements:

 - defPPr (§21.1.2.2.2)
 - [done] lvl1pPr (§21.1.2.4.13)
 - [done] lvl2pPr (§21.1.2.4.14)
 - [done] lvl3pPr (§21.1.2.4.15)
 - [done] lvl4pPr (§21.1.2.4.16)
 - [done] lvl5pPr (§21.1.2.4.17)
 - [done] lvl6pPr (§21.1.2.4.18)
 - [done] lvl7pPr (§21.1.2.4.19)
 - [done] lvl8pPr (§21.1.2.4.20)
 - [done] lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)

 Child elements:

 - [done] spcPct (Spacing Percent) §21.1.2.2.11
 - [done] spcPts (Spacing Points)  §21.1.2.2.12

*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_spcAft()
{
    READ_PROLOGUE

    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(spcPts)
            ELSE_TRY_READ_IF(spcPct)
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL lnSpc
//! lnSpc - line spacing
/*!
 Parent elements:

 - defPPr (§21.1.2.2.2)
 - lvl1pPr (§21.1.2.4.13)
 - lvl2pPr (§21.1.2.4.14)
 - lvl3pPr (§21.1.2.4.15)
 - lvl4pPr (§21.1.2.4.16)
 - lvl5pPr (§21.1.2.4.17)
 - lvl6pPr (§21.1.2.4.18)
 - lvl7pPr (§21.1.2.4.19)
 - lvl8pPr (§21.1.2.4.20)
 - lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)

 Child elements:

 - [done] spcPct (Spacing Percent) §21.1.2.2.11
 - [done] spcPts (Spacing Points)  §21.1.2.2.12

*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_lnSpc()
{
    READ_PROLOGUE
    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(spcPct)
            ELSE_TRY_READ_IF(spcPts)
        }
    }
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL spcPts
//! spcPts - spacing points
/*!
 Parent elements:
 - lnSpc (§21.1.2.2.5)
 - [done] spcAft (§21.1.2.2.9)
 - [done] spcBef (§21.1.2.2.10)
*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_spcPts()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR_WITHOUT_NS(val)

    bool ok = false;
    const int margin = val.toDouble(&ok);

    if (ok) {
        switch (m_currentSpacingType) {
            case (spacingMarginTop):
                m_currentParagraphStyle.addPropertyPt("fo:margin-top", margin/100);
                break;
            case (spacingMarginBottom):
                m_currentParagraphStyle.addPropertyPt("fo:margin-bottom", margin/100);
                break;
            case (spacingLines):
                m_currentParagraphStyle.addPropertyPt("fo:line-height", margin/100);
                break;
        }
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL spcPct
//! spcPct - spacing percentage
/*!
 Parent elements:
 - [done] lnSpc (§21.1.2.2.5)
 - [done] spcAft (§21.1.2.2.9)
 - [done] spcBef (§21.1.2.2.10)
*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_spcPct()
{
    READ_PROLOGUE

    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR_WITHOUT_NS(val)
    bool ok = false;
    int lineSpace = val.toDouble(&ok)/1000;
    if (ok) {
        QString space = "%1";
        space = space.arg(lineSpace);
        space.append('%');
        switch (m_currentSpacingType) {
            case (spacingMarginTop):
                m_currentParagraphStyle.addProperty("fo:margin-top", space);
                break;
            case (spacingMarginBottom):
                m_currentParagraphStyle.addProperty("fo:margin-bottom", space);
                break;
            case (spacingLines):
                m_currentParagraphStyle.addProperty("fo:line-height", space);
                break;
        }
    }

    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL buNone
//! buNone - No bullets
/*!
 Parent elements:
 - defPPr (§21.1.2.2.2)
 - [done] lvl1pPr (§21.1.2.4.13)
 - [done] lvl2pPr (§21.1.2.4.14)
 - [done] lvl3pPr (§21.1.2.4.15)
 - [done] lvl4pPr (§21.1.2.4.16)
 - [done] lvl5pPr (§21.1.2.4.17)
 - [done] lvl6pPr (§21.1.2.4.18)
 - [done] lvl7pPr (§21.1.2.4.19)
 - [done] lvl8pPr (§21.1.2.4.20)
 - [done] lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)
*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_buNone()
{
    READ_PROLOGUE
    m_lstStyleFound = true;
    m_currentBulletProperties.setBulletChar("");
    m_listStylePropertiesAltered = true;
    readNext();
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL buAutoNum
//! buAutoNum - Bullet Auto Numbering
/*!
 Parent elements:
 - defPPr (§21.1.2.2.2)
 - [done] lvl1pPr (§21.1.2.4.13)
 - [done] lvl2pPr (§21.1.2.4.14)
 - [done] lvl3pPr (§21.1.2.4.15)
 - [done] lvl4pPr (§21.1.2.4.16)
 - [done] lvl5pPr (§21.1.2.4.17)
 - [done] lvl6pPr (§21.1.2.4.18)
 - [done] lvl7pPr (§21.1.2.4.19)
 - [done] lvl8pPr (§21.1.2.4.20)
 - [done] lvl9pPr (§21.1.2.4.21)
 - [done] pPr (§21.1.2.2.7)
*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_buAutoNum()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    TRY_READ_ATTR_WITHOUT_NS(type)

    if (!type.isEmpty()) {
        m_lstStyleFound = true;
        if (type == "arabicPeriod") {
            m_currentBulletProperties.setSuffix(".");
            m_currentBulletProperties.setNumFormat("1");
        }
        else if (type == "arabicParenR") {
            m_currentBulletProperties.setSuffix(")");
            m_currentBulletProperties.setNumFormat("1");
        }
        else if (type == "alphaUcPeriod") {
            m_currentBulletProperties.setSuffix(".");
            m_currentBulletProperties.setNumFormat("A");
        }
        else if (type == "alphaLcPeriod") {
            m_currentBulletProperties.setSuffix(".");
            m_currentBulletProperties.setNumFormat("a");
        }
        else if (type == "alphaUcParenR") {
            m_currentBulletProperties.setSuffix(")");
            m_currentBulletProperties.setNumFormat("A");
        }
        else if (type == "alphaLcParenR") {
            m_currentBulletProperties.setSuffix(")");
            m_currentBulletProperties.setNumFormat("a");
        }
        else if (type == "romanUcPeriod") {
            m_currentBulletProperties.setSuffix(".");
            m_currentBulletProperties.setNumFormat("I");
        }
        else if (type == "romanLcPeriod") {
            m_currentBulletProperties.setSuffix(")");
            m_currentBulletProperties.setNumFormat("i");
        }
        else if (type == "romanUcParenR") {
            m_currentBulletProperties.setSuffix(")");
            m_currentBulletProperties.setNumFormat("I");
        }
        else if (type == "romanLcParenR") {
            m_currentBulletProperties.setSuffix(")");
            m_currentBulletProperties.setNumFormat("i");
        }
    }

    TRY_READ_ATTR_WITHOUT_NS(startAt)
    if (!startAt.isEmpty()) {
        m_currentBulletProperties.m_startValue = startAt.toInt();
    }

    m_listStylePropertiesAltered = true;
    readNext();

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL defRPr
//! defRPr - Default Text Run Properties
/*!

 Parent elements:
     - defPPr (§21.1.2.2.2)
     - [done] lvl1pPr (§21.1.2.4.13)
     - [done] lvl2pPr (§21.1.2.4.14)
     - [done] lvl3pPr (§21.1.2.4.15)
     - [done] lvl4pPr (§21.1.2.4.16)
     - [done] lvl5pPr (§21.1.2.4.17)
     - [done] lvl6pPr (§21.1.2.4.18)
     - [done] lvl7pPr (§21.1.2.4.19)
     - [done] lvl8pPr (§21.1.2.4.20)
     - [done] lvl9pPr (§21.1.2.4.21) 
     - pPr (§21.1.2.2.7)

 Child elements:
     - blipFill (Picture Fill)                         §20.1.8.14
     - cs (Complex Script Font)                        §21.1.2.3.1
     - ea (East Asian Font)                            §21.1.2.3.3
     - effectDag (Effect Container)                    §20.1.8.25
     - effectLst (Effect Container)                    §20.1.8.26
     - extLst (Extension List)                         §20.1.2.2.15
     - gradFill (Gradient Fill)                        §20.1.8.33
     - grpFill (Group Fill)                            §20.1.8.35
     - highlight (Highlight Color)                     §21.1.2.3.4
     - hlinkClick (Click Hyperlink)                    §21.1.2.3.5
     - hlinkMouseOver (Mouse-Over Hyperlink)           §21.1.2.3.6
     - latin (Latin Font)                              §21.1.2.3.7
     - ln (Outline)                                    §20.1.2.2.24
     - noFill (No Fill)                                §20.1.8.44
     - pattFill (Pattern Fill)                         §20.1.8.47
     - rtl (Right to Left Run)                         §21.1.2.2.8
     - [done] solidFill (Solid Fill)                          §20.1.8.54
     - sym (Symbol Font)                               §21.1.2.3.10
     - uFill (Underline Fill)                          §21.1.2.3.12
     - uFillTx (Underline Fill Properties Follow Text) §21.1.2.3.13
     - uLn (Underline Stroke)                          §21.1.2.3.14
     - uLnTx (Underline Follows Text)                  §21.1.2.3.15
*/
//! @todo support all child elements
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_defRPr()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());

    m_currentColor = QColor();

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF(solidFill)
            ELSE_TRY_READ_IF(latin)
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    if (m_currentColor.isValid()) {
        m_currentTextStyleProperties->setForeground(m_currentColor);
        m_currentColor = QColor();
    }

    TRY_READ_ATTR_WITHOUT_NS(sz)
    if (!sz.isEmpty()) {
        int szInt;
        STRING_TO_INT(sz, szInt, "defRPr@sz")
        m_currentTextStyleProperties->setFontPointSize(qreal(szInt) / 100.0);
    }

    if (attrs.hasAttribute("b")) {
        m_currentTextStyleProperties->setFontWeight(
            MSOOXML::Utils::convertBooleanAttr(attrs.value("b").toString()) ? QFont::Bold : QFont::Normal);
    }
    if (attrs.hasAttribute("i")) {
        m_currentTextStyleProperties->setFontItalic(
            MSOOXML::Utils::convertBooleanAttr(attrs.value("i").toString()));
    }

    TRY_READ_ATTR_WITHOUT_NS(cap);
    if (!cap.isEmpty()) {
        m_currentTextStyleProperties->setFontCapitalization(capToOdf(cap));
    }

    TRY_READ_ATTR_WITHOUT_NS(spc)
    if (!spc.isEmpty()) {
        int spcInt;
        STRING_TO_INT(spc, spcInt, "rPr@spc")
        m_currentTextStyleProperties->setFontLetterSpacing(qreal(spcInt) / 100.0);
    }
    TRY_READ_ATTR_WITHOUT_NS(strike)
    if (strike == QLatin1String("sngStrike")) {
        m_currentTextStyleProperties->setStrikeOutType(KoCharacterStyle::SingleLine);
        m_currentTextStyleProperties->setStrikeOutStyle(KoCharacterStyle::SolidLine);
    } else if (strike == QLatin1String("dblStrike")) {
        m_currentTextStyleProperties->setStrikeOutType(KoCharacterStyle::DoubleLine);
        m_currentTextStyleProperties->setStrikeOutStyle(KoCharacterStyle::SolidLine);
    } else {
        // empty or "noStrike"
    }
    // from
    TRY_READ_ATTR_WITHOUT_NS(baseline)
    if (!baseline.isEmpty()) {
        int baselineInt;
        STRING_TO_INT(baseline, baselineInt, "rPr@baseline")
        if (baselineInt > 0)
            m_currentTextStyleProperties->setVerticalAlignment( QTextCharFormat::AlignSuperScript );
        else if (baselineInt < 0)
            m_currentTextStyleProperties->setVerticalAlignment( QTextCharFormat::AlignSubScript );
    }

    TRY_READ_ATTR_WITHOUT_NS(u)
    if (!u.isEmpty()) {
        MSOOXML::Utils::setupUnderLineStyle(u, m_currentTextStyleProperties);
    }

    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL bodyPr
//! bodyPr handler (Body Properties)
/*! ECMA-376, 21.1.2.1.1, p.3556 - DrawingML

 This element defines the body properties for the text body within a shape.

 Parent elements:
 - lnDef (§20.1.4.1.20)
 - rich (§21.2.2.156)
 - spDef (§20.1.4.1.27)
 - t (§21.4.3.8)
 - txBody (§21.3.2.26)
 - txBody(§20.1.2.2.40)
 - txBody (§20.5.2.34)
 - [done] txBody (§19.3.1.51)
 - txDef (§20.1.4.1.28)
 - txPr (§21.2.2.216)

 Child elements:
 - extLst (Extension List) §20.1.2.2.15
 - flatTx (No text in 3D scene) §20.1.5.8
 - noAutofit (No AutoFit) §21.1.2.1.2
 - [done] normAutofit (Normal AutoFit) §21.1.2.1.3
 - prstTxWarp (Preset Text Warp) §20.1.9.19
 - scene3d (3D Scene Properties) §20.1.4.1.26
 - sp3d (Apply 3D shape properties) §20.1.5.12
 - [done] spAutoFit (Shape AutoFit) §21.1.2.1.4

*/
//! @todo support all attributes
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_bodyPr()
{
    READ_PROLOGUE
    const QXmlStreamAttributes attrs(attributes());
    // wrap (Text Wrapping Type)
    // Specifies the wrapping options to be used for this text body. If this attribute is omitted,
    // then a value of square is implied which wraps the text using the bounding text box.
    // The possible values for this attribute are defined by the ST_TextWrappingType simple type (§20.1.10.85):
    // - none (Text Wrapping Type Enum ( None ))
    //        No wrapping occurs on this text body. Words spill out without paying attention to the bounding
    //        rectangle boundaries.
    // - square (Text Wrapping Type Enum ( Square ))
    //        Determines whether we wrap words within the bounding rectangle.
    TRY_READ_ATTR_WITHOUT_NS(wrap)
    TRY_READ_ATTR_WITHOUT_NS(anchor)
    TRY_READ_ATTR_WITHOUT_NS(lIns)
    TRY_READ_ATTR_WITHOUT_NS(rIns)
    TRY_READ_ATTR_WITHOUT_NS(bIns)
    TRY_READ_ATTR_WITHOUT_NS(tIns)
//TODO    TRY_READ_ATTR_WITHOUT_NS(fontAlgn)

    m_shapeTextPosition.clear();
    m_shapeTextTopOff.clear();
    m_shapeTextBottomOff.clear();
    m_shapeTextLeftOff.clear();
    m_shapeTextRightOff.clear();

#ifdef PPTXXMLSLIDEREADER_H
    inheritBodyProperties();
#endif

    if (!lIns.isEmpty()) {
        m_shapeTextLeftOff = lIns;
    }
    if (!rIns.isEmpty()) {
        m_shapeTextRightOff = rIns;
    }
    if (!tIns.isEmpty()) {
        m_shapeTextTopOff = tIns;
    }
    if (!bIns.isEmpty()) {
        m_shapeTextBottomOff = bIns;
    }

    if (!anchor.isEmpty()) {
        if (anchor == "t") {
            m_shapeTextPosition = "top";
        }
        else if (anchor == "b") {
            m_shapeTextPosition = "bottom";
        }
        else if (anchor == "ctr") {
            m_shapeTextPosition = "middle";
        }
        else if (anchor == "just") {
            m_shapeTextPosition = "justify";
        }
    }

//! @todo more atributes

    bool spAutoFit = false;
    bool normAutoFit = false;
    while (!atEnd()) {
        readNext();
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            if (qualifiedName() == QLatin1String("a:spAutoFit")) {
                TRY_READ(spAutoFit)
                spAutoFit = true;
            }
            else if (qualifiedName() == QLatin1String("a:normAutofit")) {
                normAutoFit = true;
            }
        }
    }

#ifdef PPTXXMLSLIDEREADER_H

    saveBodyProperties();

    m_currentPresentationStyle.addProperty("draw:auto-grow-height",
            spAutoFit ? MsooXmlReader::constTrue : MsooXmlReader::constFalse, KoGenStyle::GraphicType);
    m_currentPresentationStyle.addProperty("draw:auto-grow-width",
            (!spAutoFit || wrap == QLatin1String("square"))
            ? MsooXmlReader::constFalse : MsooXmlReader::constTrue, KoGenStyle::GraphicType);
    // text in shape
    m_currentPresentationStyle.addProperty("fo:wrap-option",
        wrap == QLatin1String("none") ? QLatin1String("no-wrap") : QLatin1String("wrap"), KoGenStyle::GraphicType);
    if (normAutoFit) {
        m_currentPresentationStyle.addProperty("draw:fit-to-size", "true", KoGenStyle::GraphicType);
    }
#endif
    READ_EPILOGUE
}

#undef CURRENT_EL
#define CURRENT_EL spAutoFit
//! spAutoFit handler (Shape AutoFit)
/*! ECMA-376, 21.1.2.1.4, p.3567 - DrawingML

 This element specifies that a shape should be auto-fit to fully contain the text described within it.
 Auto-fitting is when text within a shape is scaled in order to contain all the text inside.
 If this element is omitted, then noAutofit or auto-fit off is implied.

 Parent elements:
 - [done] bodyPr (§21.1.2.1.1)

 No child elements.
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_spAutoFit()
{
    READ_PROLOGUE
    skipCurrentElement();
    READ_EPILOGUE
}

#undef MSOOXML_CURRENT_NS
#ifdef DRAWINGML_TXBODY_NS
#define MSOOXML_CURRENT_NS DRAWINGML_TXBODY_NS
#else
#define MSOOXML_CURRENT_NS DRAWINGML_NS
#endif

#undef CURRENT_EL
#define CURRENT_EL txBody
//! txBody handler (Shape Text Body)
/*! ECMA-376, 20.1.2.2.40, p. 3050
 This element specifies the existence of text to be contained within the corresponding cell.
 Only used for text inside a cell.
*/
KoFilter::ConversionStatus MSOOXML_CURRENT_CLASS::read_DrawingML_txBody()
{
    READ_PROLOGUE2(DrawingML_txBody)

    m_lstStyleFound = false;
    m_prevListLevel = 0;
    m_currentListLevel = 0;
    m_pPr_lvl = 0;

    MSOOXML::Utils::XmlWriteBuffer listBuf;
    body = listBuf.setWriter(body);

    while (!atEnd()) {
        readNext();
        kDebug() << *this;
        BREAK_IF_END_OF(CURRENT_EL);
        if (isStartElement()) {
            TRY_READ_IF_NS(a, bodyPr)
            ELSE_TRY_READ_IF_NS(a, lstStyle)
            else if (qualifiedName() == QLatin1String("a:p")) {
                TRY_READ(DrawingML_p);
            }
//! @todo add ELSE_WRONG_FORMAT
        }
    }

    if (m_prevListLevel > 0) {
        // Ending our current level
        body->endElement(); // text:list
        // Ending any additional levels needed
        for(; m_prevListLevel > 1; --m_prevListLevel) {
            body->endElement(); // text:list-item
            body->endElement(); // text:list
        }
        m_prevListLevel = 0;
    }

    READ_EPILOGUE
}

#endif
