/*
 * This file is part of Office 2007 Filters for KOffice
 *
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
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

// This is not a normail heder, *don't* add include guards to it.
// This will cause the compiler to get wrong offsets and to corrupt the stack.

// included by DocxXmlDocumentReader

protected:

void initDrawingML();

// All the readers
#ifndef MSOOXMLDRAWINGTABLESTYLEREADER_H
KoFilter::ConversionStatus read_chart();
#endif
KoFilter::ConversionStatus read_pic();
KoFilter::ConversionStatus read_nvPicPr();
enum cNvPrCaller {
    cNvPr_nvSpPr,
    cNvPr_nvPicPr,
    cNvPr_nvCxnSpPr
};
KoFilter::ConversionStatus read_cNvPr(cNvPrCaller caller);
KoFilter::ConversionStatus read_cNvPicPr();
KoFilter::ConversionStatus read_nvSpPr();
KoFilter::ConversionStatus read_style();
KoFilter::ConversionStatus read_fillRef();
KoFilter::ConversionStatus read_lnRef();
KoFilter::ConversionStatus read_cNvSpPr();
KoFilter::ConversionStatus read_nvCxnSpPr();
KoFilter::ConversionStatus read_grpSp();

KoFilter::ConversionStatus read_extLst();

KoFilter::ConversionStatus read_grpSpPr();
void preReadSp();
void generateFrameSp();
KoFilter::ConversionStatus read_cxnSp();
KoFilter::ConversionStatus read_sp();
KoFilter::ConversionStatus read_spPr();
KoFilter::ConversionStatus read_xfrm();
bool m_xfrm_read;
KoFilter::ConversionStatus read_off();
KoFilter::ConversionStatus read_chOff();
KoFilter::ConversionStatus read_chExt();
KoFilter::ConversionStatus read_ext();
KoFilter::ConversionStatus read_blip();
KoFilter::ConversionStatus read_stretch();
KoFilter::ConversionStatus read_biLevel();
KoFilter::ConversionStatus read_grayscl();
KoFilter::ConversionStatus read_lum();
KoFilter::ConversionStatus read_tint();
KoFilter::ConversionStatus read_alpha();

KoFilter::ConversionStatus read_satMod();
KoFilter::ConversionStatus read_tile();
KoFilter::ConversionStatus read_fillRect();
KoFilter::ConversionStatus read_graphic();
KoFilter::ConversionStatus read_graphicData();
enum blipFillCaller {
    blipFill_spPr = 'a',
    blipFill_pic = 'p', //dml in pptx; for dml in docx use 'pic'
    blipFill_rPr = 'p', //dml
    blipFill_bgPr = 'a', // pptx
};
KoFilter::ConversionStatus read_blipFill(blipFillCaller caller);

KoFilter::ConversionStatus read_DrawingML_p();
read_p_args m_read_DrawingML_p_args;

KoFilter::ConversionStatus read_DrawingML_rPr();

KoFilter::ConversionStatus read_hlinkClick();

KoFilter::ConversionStatus read_DrawingML_pPr();

KoFilter::ConversionStatus read_DrawingML_r();
KoFilter::ConversionStatus read_DrawingML_highlight();
KoFilter::ConversionStatus read_DrawingML_txBody();
KoFilter::ConversionStatus read_lstStyle();
KoFilter::ConversionStatus read_latin();
KoFilter::ConversionStatus read_solidFill();
int m_gradPosition;
KoFilter::ConversionStatus read_gradFill();
bool m_gradRotation; //whethere there should be angle with gradient
KoFilter::ConversionStatus read_gsLst();
KoFilter::ConversionStatus read_gs();
KoFilter::ConversionStatus read_prstGeom();
enum noFillCaller {
        noFill_rPr
};
KoFilter::ConversionStatus read_noFill(noFillCaller caller);
KoFilter::ConversionStatus read_schemeClr();
KoFilter::ConversionStatus read_prstClr();
KoFilter::ConversionStatus read_sysClr();
KoFilter::ConversionStatus read_lumMod();
KoFilter::ConversionStatus read_lumOff();
KoFilter::ConversionStatus read_shade();
KoFilter::ConversionStatus read_ln();
KoFilter::ConversionStatus read_srgbClr();
KoFilter::ConversionStatus read_scrgbClr();

qreal m_currentShadeLevel;
qreal m_currentTint; // value of current tint
int m_currentAlpha; // current alpha color value
qreal m_currentSatMod; //value of current saturation modulation

QString m_contentType; // read in prstGeom

KoFilter::ConversionStatus read_fld();

enum spacingType {
    spacingMarginTop, spacingLines, spacingMarginBottom
};
spacingType m_currentSpacingType; // determines how spcPct and spcPts should behave

MSOOXML::Utils::autoFitStatus m_normAutoFit; // Whether text should be fitted to fit the shape

KoFilter::ConversionStatus read_lnSpc();
KoFilter::ConversionStatus read_spcPct();
KoFilter::ConversionStatus read_spcBef();
KoFilter::ConversionStatus read_spcAft();
KoFilter::ConversionStatus read_spcPts();

QString m_shapeTextPosition;
QString m_shapeTextTopOff;
QString m_shapeTextBottomOff;
QString m_shapeTextLeftOff;
QString m_shapeTextRightOff;

bool m_listStylePropertiesAltered;
bool m_previousListWasAltered;

KoFilter::ConversionStatus read_buClr();
KoFilter::ConversionStatus read_buSzPct();
KoFilter::ConversionStatus read_buChar();
KoFilter::ConversionStatus read_buBlip();
KoFilter::ConversionStatus read_buNone();
KoFilter::ConversionStatus read_buFont();
KoFilter::ConversionStatus read_buAutoNum();
KoFilter::ConversionStatus lvlHelper(const QString& level);
KoFilter::ConversionStatus read_lvl1pPr();
KoFilter::ConversionStatus read_lvl2pPr();
KoFilter::ConversionStatus read_lvl3pPr();
KoFilter::ConversionStatus read_lvl4pPr();
KoFilter::ConversionStatus read_lvl5pPr();
KoFilter::ConversionStatus read_lvl6pPr();
KoFilter::ConversionStatus read_lvl7pPr();
KoFilter::ConversionStatus read_lvl8pPr();
KoFilter::ConversionStatus read_lvl9pPr();
KoFilter::ConversionStatus read_defRPr();
KoFilter::ConversionStatus read_bodyPr();
KoFilter::ConversionStatus read_spAutoFit();

KoFilter::ConversionStatus read_overrideClrMapping();

//! Sets style:wrap attribute of style:style/style:graphic-properties element. Used in read_anchor()
void saveStyleWrap(const char * style);

void algnToODF(const char * odfEl, const QString& emuValue);

//! Sets fo:margin-* attribute of style:style/style:graphic-properties element. Used in read_anchor()
void distToODF(const char * odfEl, const QString& emuValue);

//! ODF 1.1., 15.14.9 Fill Image Rendering Style
//! Set by read_stretch()
bool m_fillImageRenderingStyleStretch;

//! Used by read_wrap*()
void readWrap();

//! Copies file to destination directory. @a destinationName is set.
KoFilter::ConversionStatus copyFile(
    const QString& sourceName, const QString& destinationDir, QString& destinationName, bool oleType=false);

bool m_drawing_anchor; //! set by read_drawing() to indicate if we have encountered drawing/anchor, used by read_pic()
bool m_drawing_inline; //! set by read_drawing() to indicate if we have encountered drawing/inline, used by read_pic()

int m_prevListLevel; //! set by drawingML_ppr
int m_currentListLevel; //! set by drawingML_ppr

// Shape properties
int m_svgX; //!< set by read_off()
int m_svgY; //!< set by read_off()
int m_svgWidth; //! set by read_ext()
int m_svgHeight; //! set by read_ext()
int m_svgChX; //!< set by read_chOff()
int m_svgChY; //!< set by read_chOff()
int m_svgChWidth; //! set by read_chExt()
int m_svgChHeight; //! set by read_chExt()
// These have to be in a vector in order to support group shapes within
// a group shape
bool m_inGrpSpPr; //Whether we are in group shape, affects transformations
struct GroupProp {
    qreal svgXOld;
    qreal svgYOld;
    qreal svgWidthOld;
    qreal svgHeightOld;
    qreal svgXChOld;
    qreal svgYChOld;
    qreal svgWidthChOld;
    qreal svgHeightChOld;
};
QVector<GroupProp> m_svgProp; //! value of the parent
bool m_flipH; //! set by read_xfrm()
bool m_flipV; //! set by read_xfrm()
int m_rot; //! set by read_xfrm()

//! true if no fill should be applied for element; used e.g. by pic:spPr/a:noFill elem.
bool m_noFill;

QString m_xlinkHref; //!< set by read_blip()
QString m_cNvPrId; //!< set by read_cNvPr()
QString m_cNvPrName; //!< set by read_cNvPr()
QString m_cNvPrDescr; //!< set by read_cNvPr()

//! When dealing with colors there's no way to know what type of attribute
//! we are setting. While MSOOXML doesn't need to know the context in which a
//! color is used, ODF does need to know this.
enum ColorType {
    BackgroundColor,
    OutlineColor,
    TextColor,
    GradientColor
};

//! set by one of the color readers, read by read_solidFill. Read and set by one of the color transformations.
QColor m_currentColor;

qreal* m_currentDoubleValue;

bool    m_hyperLink;
QString m_hyperLinkTarget;

