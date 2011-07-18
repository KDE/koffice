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

#ifndef DOCXXMLDOCUMENTREADER_H
#define DOCXXMLDOCUMENTREADER_H

#include <QBuffer>
#include <QString>

#include <MsooXmlCommonReader.h>
#include <MsooXmlThemesReader.h>
#include "DocxXmlNotesReader.h"

#include <KXmlWriter.h>
#include <KOdfGenericStyle.h>
#include <styles/KCharacterStyle.h>

//#define NO_DRAWINGML_PICTURE // disables pic:pic, etc. in MsooXmlCommonReader

class DocxImport;
class DocxXmlDocumentReaderContext;
namespace MSOOXML
{
class MsooXmlRelationships;
}

//! A class reading MSOOXML DOCX markup - document.xml part.
class DocxXmlDocumentReader : public MSOOXML::MsooXmlCommonReader
{
public:
    explicit DocxXmlDocumentReader(KoOdfWriters *writers);

    virtual ~DocxXmlDocumentReader();

    //! Reads/parses the file of format document.xml.
    //! The output goes mainly to KXmlWriter* KoOdfWriters::body
    virtual KoFilter::ConversionStatus read(MSOOXML::MsooXmlReaderContext* context = 0);

#include <MsooXmlVmlReaderMethods.h> // separated as it is not a part of OOXML

protected:
    KoFilter::ConversionStatus read_body();
    KoFilter::ConversionStatus read_p();
    KoFilter::ConversionStatus read_r();
    KoFilter::ConversionStatus read_rPr();
    KoFilter::ConversionStatus read_pPr();
    KoFilter::ConversionStatus read_vanish();
    KoFilter::ConversionStatus read_numPr();
    KoFilter::ConversionStatus read_numId();
    KoFilter::ConversionStatus read_ilvl();
    KoFilter::ConversionStatus read_sectPr();
    KoFilter::ConversionStatus read_footerReference();
    KoFilter::ConversionStatus read_headerReference();
    KoFilter::ConversionStatus read_cols();
    KoFilter::ConversionStatus read_pgSz();
    KoFilter::ConversionStatus read_textDirection();
    KoFilter::ConversionStatus read_pgMar();
    KoFilter::ConversionStatus read_pgBorders();
    KoFilter::ConversionStatus read_object();
    KoFilter::ConversionStatus read_ind();
    KoFilter::ConversionStatus read_outline();
    KoFilter::ConversionStatus read_framePr();
    KoFilter::ConversionStatus read_OLEObject();
    KoFilter::ConversionStatus read_webHidden();
    KoFilter::ConversionStatus read_bookmarkStart();
    KoFilter::ConversionStatus read_bookmarkEnd();
    //KoFilter::ConversionStatus read_commentRangeEnd();
    KoFilter::ConversionStatus read_commentRangeStart();
    KoFilter::ConversionStatus read_endnoteReference();
    KoFilter::ConversionStatus read_footnoteReference();
    KoFilter::ConversionStatus read_footnotePr();
    KoFilter::ConversionStatus read_endnotePr();
    KoFilter::ConversionStatus read_lnNumType();
    KoFilter::ConversionStatus read_numFmt();
    KoFilter::ConversionStatus read_suppressLineNumbers();
    KoFilter::ConversionStatus read_hyperlink();
    KoFilter::ConversionStatus read_drawing();
    KoFilter::ConversionStatus read_ptab();
    KoFilter::ConversionStatus read_tabs();
    KoFilter::ConversionStatus read_tab();
    KoFilter::ConversionStatus read_i();
    KoFilter::ConversionStatus read_b();
    KoFilter::ConversionStatus read_u();
    KoFilter::ConversionStatus read_sz();
    KoFilter::ConversionStatus read_jc();
    KoFilter::ConversionStatus read_spacing();
    KoFilter::ConversionStatus read_trPr();
    KoFilter::ConversionStatus read_trHeight();
    enum shdCaller {
        shd_rPr,
        shd_pPr,
        shd_tcPr
    };
    KoFilter::ConversionStatus read_shd(shdCaller caller);
    KoFilter::ConversionStatus read_rFonts();
    KoFilter::ConversionStatus read_pStyle();
    KoFilter::ConversionStatus read_rStyle();
    KoFilter::ConversionStatus read_tblStyle();
    KoFilter::ConversionStatus read_tblBorders();
    KoFilter::ConversionStatus read_tblCellMar();
    KoFilter::ConversionStatus read_fldSimple();
    KoFilter::ConversionStatus read_br();
    KoFilter::ConversionStatus read_lastRenderedPageBreak();
    KoFilter::ConversionStatus read_instrText();
    KoFilter::ConversionStatus read_fldChar();
    KoFilter::ConversionStatus read_strike();
    KoFilter::ConversionStatus read_dstrike();
    KoFilter::ConversionStatus read_caps();
    KoFilter::ConversionStatus read_smallCaps();
    KoFilter::ConversionStatus read_w();
    KoFilter::ConversionStatus read_txbxContent();
    KoFilter::ConversionStatus read_color();
    KoFilter::ConversionStatus read_highlight();
    KoFilter::ConversionStatus read_vertAlign();
    KoFilter::ConversionStatus read_lang();
    KoFilter::ConversionStatus read_background();
    KoFilter::ConversionStatus read_pBdr();
    KoFilter::ConversionStatus read_bdr();
    KoFilter::ConversionStatus read_tbl();
    KoFilter::ConversionStatus read_tblPr();
    KoFilter::ConversionStatus read_tblGrid();
    KoFilter::ConversionStatus read_gridCol();
    KoFilter::ConversionStatus read_tr();
    KoFilter::ConversionStatus read_tc();
    KoFilter::ConversionStatus read_tcPr();
    KoFilter::ConversionStatus read_gridSpan();
    int m_gridSpan;

    KoFilter::ConversionStatus read_oMath();
    KoFilter::ConversionStatus read_oMathPara();
    KoFilter::ConversionStatus read_oMathParaPr();
    KoFilter::ConversionStatus read_jc_m();
    KoFilter::ConversionStatus read_r_m();
    KoFilter::ConversionStatus read_t_m();

    // 2010 specific, meant to offer a choice between vml/drawingML
    KoFilter::ConversionStatus read_AlternateContent();
    KoFilter::ConversionStatus read_Choice();
    KoFilter::ConversionStatus read_Fallback();

    enum posOffsetCaller {
        posOffset_positionH,
        posOffset_positionV
    };

    enum alignCaller {
        align_positionH,
        align_positionV
    };
    KoFilter::ConversionStatus read_align(alignCaller caller);

    KoFilter::ConversionStatus read_pict();

    KoFilter::ConversionStatus read_inline();
    KoFilter::ConversionStatus read_extent();
    KoFilter::ConversionStatus read_docPr();
    KoFilter::ConversionStatus read_anchor();
    KoFilter::ConversionStatus read_positionH();
    KoFilter::ConversionStatus read_positionV();
    KoFilter::ConversionStatus read_posOffset(posOffsetCaller caller);
    KoFilter::ConversionStatus read_wrapSquare();
    KoFilter::ConversionStatus read_wrapTight();
    KoFilter::ConversionStatus read_wrapThrough();

    bool m_createSectionStyle;
    QString m_currentSectionStyleName;
    bool m_createSectionToNext;
    KOdfGenericStyle m_currentPageStyle;
    KOdfGenericStyle m_masterPageStyle;

    DocxXmlDocumentReaderContext* m_context;

    KoOdfWriters *m_writers; // Needed to create new relationship for header/footer

    KOdfGenericStyle m_currentTableCellStyle;
    KOdfGenericStyle m_currentTableCellStyleLeft;
    KOdfGenericStyle m_currentTableCellStyleRight;
    KOdfGenericStyle m_currentTableCellStyleTop;
    KOdfGenericStyle m_currentTableCellStyleBottom;
    KOdfGenericStyle m_currentTableCellStyleInsideV;
    KOdfGenericStyle m_currentTableCellStyleInsideH;

    enum BorderSide {
        TopBorder, BottomBorder, LeftBorder, RightBorder, InsideH, InsideV
    };

    //! Used for setting up properties for pages and paragraphs.
    //! It is reversed map, so detecting duplicates is easy in applyBorders().
    QMap<QString, BorderSide> m_borderStyles;

    //! Same as above but for r element
    QMap<QString, BorderSide> m_textBorderStyles;

    //! Used for setting up properties for pages and paragraphs.
    //! It is reversed map, so detecting duplicates is easy in applyBorders().
    QMap<QString, BorderSide> m_borderPaddings;

    // ! Same as above but for border padding
    QMap<QString, BorderSide> m_textBorderPaddings;

private:
    void init();

    QColor m_backgroundColor; //Documet background color

    //! Reads CT_Border complex type (p.392), used by children of pgBorders and children of pBdr
    KoFilter::ConversionStatus readBorderElement(BorderSide borderSide, const char *borderSideName);

    //! Creates border style for readBorderElement().
    //! Result is added to m_borderStyles and m_borderPaddings
    void createBorderStyle(const QString& size, const QString& color,
                           const QString& lineStyle, BorderSide borderSide);

    //! Used by read_strike() and read_dstrike()
    void readStrikeElement(KCharacterStyle::LineType type);

    void setParentParagraphStyleName(const QXmlStreamAttributes& attrs);

    //! Applies border styles and paddings obtained in readBorderElement()
    //! to style @a style (paragraph or page...)
    void applyBorders(KOdfGenericStyle *style, QMap<QString, BorderSide> sourceBorder, QMap<QString, BorderSide> sourcePadding);

    enum ComplexFieldCharType {
       NoComplexFieldCharType, HyperlinkComplexFieldCharType, ReferenceComplexFieldCharType,
       ReferenceNextComplexFieldCharType, InternalHyperlinkComplexFieldCharType,
       CurrentPageComplexFieldCharType, NumberOfPagesComplexFieldCharType
    };
    //! Type of complex field characters we have
    ComplexFieldCharType m_complexCharType;

    //! Value of the complex field char if applicable
    QString m_complexCharValue;

    enum ComplexCharStatus {
        NoneAllowed, InstrAllowed, InstrExecute, ExecuteInstrNow
    };
    //! State of fldChar
    ComplexCharStatus m_complexCharStatus;

    enum DropCapStatus {
        NoDropCap, DropCapRead, DropCapDone
    };
    //! State of dropCap
    DropCapStatus m_dropCapStatus;

    //! Buffer where first letters of drop cap are read
    QBuffer* m_dropCapBuffer;
    KXmlWriter* m_dropCapWriter;
    QString m_dropCapLines;
    qreal   m_dropCapDistance;

    QMap<QString, QString> m_bookmarks; //!< Bookmarks

    uint m_currentTableNumber; //!< table counter, from 0
    uint m_currentTableRowNumber; //!< row counter, from 0, initialized in read_tbl()
    uint m_currentTableColumnNumber; //!< column counter, from 0, initialized in read_tr()
    KOdfGenericStyle m_currentTableRowStyle;
    QString m_currentTableName;
    qreal m_currentTableWidth; //!< in cm
    bool m_wasCaption; // bookkeeping to ensure next para is suppressed if a caption is encountered

    bool m_closeHyperlink; // should read_r close hyperlink
    bool m_listFound; // was there numPr element in ppr
    QString m_currentListStyleName;

    QMap<QString, QString> m_headers;
    QMap<QString, QString> m_footers;

#include <MsooXmlCommonReaderMethods.h>
#include <MsooXmlCommonReaderDrawingMLMethods.h>

    class Private;
    Private* const d;
};

//! Context for DocxXmlDocumentReader
class DocxXmlDocumentReaderContext : public MSOOXML::MsooXmlReaderContext
{
public:
    //! Creates the context object.
    DocxXmlDocumentReaderContext(
        DocxImport& _import,
        const QString& _path, const QString& _file,
        MSOOXML::MsooXmlRelationships& _relationships,
        MSOOXML::DrawingMLTheme* _themes
    );
    DocxImport* import;
    const QString path;
    const QString file;

    MSOOXML::DrawingMLTheme* themes;

    // Contains footnotes when read, the styles of footnotes are already put to correct files.
    QMap<QString, QString> m_footnotes;

    QMap<QString, QString> m_comments;

    QMap<QString, QString> m_endnotes;

private:
};

#endif //DOCXXMLDOCUMENTREADER_H
