/* This file is part of the KDE project
   Copyright (C) 2005 Yolla Indria <yolla.indria@gmail.com>
   Copyright (C) 2010 KO GmbH <jos.van.den.oever@kogmbh.com>
   Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
   Contact: Amit Aggarwal <amitcs06@gmail.com>

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

#ifndef PPTTOODP_H
#define PPTTOODP_H

#include "generated/simpleParser.h"
#include "pptstyle.h"
#include "DateTimeFormat.h"
#include "ParsedPresentation.h"

#include <KoFilter.h>
#include <KoGenStyles.h>
#include <KoStore.h>

#include <QtCore/QStack>
#include <QtCore/QPair>
#include <QtCore/QRectF>
#include <QtGui/QColor>
#include <QtCore/QDebug>

/**
 * Converts PPT files to ODP files.
 * This internal class is there to separate the koconverter plugin interface
 * from the koconverter plugin. This allows the conversion code to be reused
 * in other programs without plugin overhead.
 * The converter is re-entrant: if you want to conversions in parallel,
 * you need multiple instances of PptToOdp.
 */
class PptToOdp
{
public:
    /**
     * Constructs a converter.
     */
    PptToOdp();
    /**
     * Destroy the converter.
     */
    ~PptToOdp();
    /**
     * Convert a ppt file to an odp file.
     *
     * @param inputfile PowerPoint file to read.
     * @param ODF file to write.
     * @param storeType type of storage format the ODF file should be stored in.
     * @return result code of the conversion.
     */
    KoFilter::ConversionStatus convert(const QString& inputfile,
                                       const QString& outputfile,
                                       KoStore::Backend storeType);
    /**
     * Convert ppt data to odp data.
     *
     * @param input an open OLE container that contains the ppt data.
     * @param output an open KoStore to write the odp into.
     * @return result code of the conversion.
     */
    KoFilter::ConversionStatus convert(POLE::Storage& input,
                                       KoStore* output);
    QString getPicturePath(int pib) const;
private:
    /**
     * Function that does the actual conversion.
     *
     * It is shared by the two convert() functions.
     * @param input an open OLE container that contains the ppt data.
     * @param output an open KoStore to write the odp into.
     * @return result code of the conversion.
     */
    KoFilter::ConversionStatus doConversion(POLE::Storage& input,
                                            KoStore* output);

    /**
     * Helper class that for writing xml.
     *
     * Besides containing KoXmlWriter, this class keeps track of the coordinate
     * system. It has convenience functions for writing lengths in physical
     * dimensions (currently only mm).
     */
    class Writer
    {
    private:
        qreal xOffset;
        qreal yOffset;
        qreal scaleX;
        qreal scaleY;
    public:
        /**
         * Xml writer that writes into content.xml.
         */
        KoXmlWriter& xml;
        /**
         * Styles for the document that is being created.
         **/
        KoGenStyles& styles;
        /**
         * Tells if the current output is for styles.xml or content.xml
         **/
        bool stylesxml;

        /**
         * Construct a new Writer.
         *
         * @param xmlWriter The xml writer that writes content.xml
         */
        Writer(KoXmlWriter& xmlWriter, KoGenStyles& kostyles,
               bool stylexml = false);
        /**
         * Create a new writer with a new coordinate system.
         *
         * In different contexts in drawings in PPT files, different coordinate
         * systems are used. These are defined by specifying a rectangle in the
         * old coordinate system and the equivalent in the new coordinate
         * system.
         */
        Writer transform(const QRectF& oldCoords, const QRectF &newCoords) const;
        /**
         * Convert local length to global length string.
         *
         * A length without unit in the local coordinate system is converted
         * to a global length with a unit.
         * @param length a local length.
         * @return string of the global length with "mm" appended.
         */
        QString vLength(qreal length);
        /**
         * @see vLength
         */
        QString hLength(qreal length);
        /**
         * @see vLength
         */
        QString vOffset(qreal offset);
        /**
         * @see vLength
         */
        QString hOffset(qreal offset);
    };

    void createMainStyles(KoGenStyles& styles);
    void defineDefaultTextStyle(KoGenStyles& styles);
    void defineDefaultParagraphStyle(KoGenStyles& styles);
    void defineDefaultSectionStyle(KoGenStyles& styles);
    void defineDefaultRubyStyle(KoGenStyles& styles);
    void defineDefaultTableStyle(KoGenStyles& styles);
    void defineDefaultTableColumnStyle(KoGenStyles& styles);
    void defineDefaultTableRowStyle(KoGenStyles& styles);
    void defineDefaultTableCellStyle(KoGenStyles& styles);
    void defineDefaultGraphicStyle(KoGenStyles& styles);
    void defineDefaultPresentationStyle(KoGenStyles& styles);
    void defineDefaultDrawingPageStyle(KoGenStyles& styles);
    void defineDefaultChartStyle(KoGenStyles& styles);

    /** define automatic styles for text, paragraphs, graphic and presentation
      families
      */
    void defineMasterStyles(KoGenStyles& styles);
    void defineAutomaticDrawingPageStyles(KoGenStyles& styles);

    // we assume that these functions are the same for all style families
    void defineDefaultTextProperties(KoGenStyle& style);
    void defineDefaultParagraphProperties(KoGenStyle& style);
    void defineDefaultGraphicProperties(KoGenStyle& style);

    /* Extract data from TextCFException into the style */
    void defineTextProperties(KoGenStyle& style, const MSO::TextCFException* cf,
                              const MSO::TextCFException9* cf9,
                              const MSO::TextCFException10* cf10,
                              const MSO::TextSIException * si);

    /* Extract data from TextPFException into the style */
    void defineParagraphProperties(KoGenStyle& style,
                                   const PptTextPFRun& pf);
    /* Extract data into the style */
    template <typename T>
    void defineDrawingPageStyle(KoGenStyle& style, const T* o,
                                const MSO::HeadersFootersAtom* hf);

    /* Extract data into the style */
    template <typename T>
    void defineGraphicProperties(KoGenStyle& style, const T& o,
                                 const QString& listStyle = QString());

    /**
     * Structure that influences all information that affects the style of a
     * text:style.
     * This is a convenience container for passing this information.
     **/
    class ListStyleInput {
    public:
        PptTextPFRun pf;
        const MSO::TextPFException9* pf9;
        const MSO::TextCFException* cf;
        const MSO::TextCFException9* cf9;
        const MSO::TextCFException10* cf10;
        const MSO::TextSIException* si;

        ListStyleInput() :pf9(0), cf(0), cf9(0), cf10(0), si(0) {}
    };
    /* Extract data into the style element style:list */
    void defineListStyle(KoGenStyle& style,
                         const MSO::TextMasterStyleAtom& levels,
                         const MSO::TextMasterStyle9Atom* levels9 = 0,
                         const MSO::TextMasterStyle10Atom* levels10 = 0);
    void defineListStyle(KoGenStyle& style, quint8 depth,
                         ListStyleInput input,
                         const MSO::TextMasterStyleLevel* level = 0,
                         const MSO::TextMasterStyle9Level* level9 = 0,
                         const MSO::TextMasterStyle10Level* level10 = 0);

    void defineListStyle(KoGenStyle& style, quint8 depth,
                         const ListStyleInput& info,
                         const ListStyleInput& parent);

    const MSO::StyleTextProp9* getStyleTextProp9(quint32 slideIdRef,
                                                quint32 textType, quint8 pp9rt);
    QString defineAutoListStyle(Writer& out, const PptTextPFRun& pf,
                                const MSO::TextPFException9* pf9);

    quint32 getTextType(const MSO::PptOfficeArtClientTextBox* clientTextbox,
                        const MSO::PptOfficeArtClientData* clientData) const;
    void addGraphicStyleToDrawElement(Writer& out, const MSO::OfficeArtSpContainer& o);
    void addPresentationStyleToDrawElement(Writer& out, const MSO::OfficeArtSpContainer& o);

    QByteArray createContent(KoGenStyles& styles);
    void processSlideForBody(unsigned slideNo, Writer& out);
    void processObjectForBody(const MSO::OfficeArtSpgrContainerFileBlock& o, Writer& out);
    void processObjectForBody(const MSO::OfficeArtSpgrContainer& o, Writer& out);
    void processObjectForBody(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processDrawingObjectForBody(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processTextForBody(const MSO::OfficeArtSpContainer& o,
                            const MSO::TextContainer& tc, Writer& out);
    void processTextObjectForBody(const MSO::OfficeArtSpContainer& o, const MSO::TextContainer& tc, Writer& out);

    int processTextSpan(const MSO::TextContainer& tc, Writer& out,
                        const QString& text, const int start, int end);
    int processTextSpans(const MSO::TextContainer& tc, Writer& out,
                        const QString& text, int start, int end);
    void processTextLine(Writer& out, const MSO::OfficeArtSpContainer& o,
                         const MSO::TextContainer& tc, const QString& text,
                        int start, int end, QStack<QString>& levels);

     /**
     * @brief Write declaration in the content body presentation
     * @param xmlWriter XML writer to write
     */
     void processDeclaration(KoXmlWriter* xmlWriter);

     /**
     * @brief Write Frame element.
     * @param KoGenStyle& style To represent the style
     * @param Writer& out writer xml.
     * @param presentatonName represent the class for ex: page-number etc
     * @param QRect -  co-ordinates of the frame
     * @param mStyle - presentation style
     * @param pStyle - paragraph style
     * @param tStyle - text style
     */
     void addFrame(KoGenStyle& style, Writer& out, const char* presentationName,
            const QRect& rect, QString mStyle, QString pStyle, QString tStyle);

    /**
      * @brief An enumeration that specifies an action that can be performed
      * when interacting with an object during a slide show.
      */
    enum {
        /**
          * @brief No effect.
          */
        II_NoAction,

        /**
          * @brief A macro is executed.
          */
        II_MacroAction,

        /**
          * @brief A program is run.
          */
        II_RunProgramAction,

        /**
          * @brief The current presentation slide of the slide show jumps to
          * another presentation slide in the same presentation.
          */
        II_JumpAction,

        /**
          * @brief A URL is executed.
          */
        II_HyperlinkAction,

        /**
          * @brief An OLE action (only valid if the object this applies to is an
          * OLE embedded object).
          */
        II_OLEAction,

        /**
          * @brief A media object is played.
          */
        II_MediaAction,

        /**
          * @brief A named show is displayed.
          */
        II_CustomShowAction,
    }InteractiveInfoActionEnum;

    /**
      * @brief Converts vector of quint16 to String
      *
      * Powerpoint files have text as utf16.
      * @param data Vector to convert
      * @return data as string
      */
    inline QString utf16ToString(const QVector<quint16> &data);

    /**
      * @brief Find hyperlink with specified id
      *
      * @param id Id of the hyperlink to find
      * @return QPair where first element is hyperlink's target, second is
      * user readable name. If both are empty, hyperlink is not found
      */
    QPair<QString, QString> findHyperlink(const quint32 id);

    /**
    * @brief Write text deindentations the specified amount. Actually it just
    * closes elements.
    *
    * Doesn't close the last text:list-item though.
    * @param xmlWriter XML writer to write closing tags
    * @param count how many lists and list items to leave open
    * @param levels the list of levels to remove from
    */
    void writeTextObjectDeIndent(KoXmlWriter& xmlWriter,
                                 const unsigned int count,
                                 QStack<QString>& levels);

    /**
    * @brief Convert paraspacing value to centimeters
    *
    * ParaSpacing is a 2-byte signed integer that specifies text paragraph
    * spacing. It MUST be a value from the following table:
    * Range                     Meaning
    * 0 to 13200, inclusive.    The value specifies spacing as a percentage of the
    *                           text line height.
    * Less than 0.              The absolute value specifies spacing in master
    *                           units.
    *
    * master unit: A unit of linear measurement that is equal to 1/576 inch.
    *
    * @brief value Value to convert
    * @return value converted to centimeters
    */
    QString paraSpacingToCm(int value) const;

    /**
    * @brief Convert TextAlignmentEnum value to a string from ODF specification
    * An enumeration that specifies paragraph alignments.
    * Name                              Value       Meaning
    * Tx_ALIGNLeft              0x0000    For horizontal text, left aligned.
    *                                     For vertical text, top aligned.
    * Tx_ALIGNCenter            0x0001    For horizontal text, centered.
    *                                     For vertical text, middle aligned.
    * Tx_ALIGNRight             0x0002    For horizontal text, right aligned.
    *                                     For vertical text, bottom aligned.
    * Tx_ALIGNJustify           0x0003    For horizontal text, flush left and
    *                                     right.
    *                                     For vertical text, flush top and bottom.
    * Tx_ALIGNDistributed       0x0004    Distribute space between characters.
    * Tx_ALIGNThaiDistributed   0x0005    Thai distribution justification.
    * Tx_ALIGNJustifyLow        0x0006    Kashida justify low.
    *
    * NOTE values Tx_ALIGNDistributed, Tx_ALIGNThaiDistributed and
    * Tx_ALIGNJustifyLow were not found in ODF specification v1.1 and are
    * ignored at the moment. Values "end" and "start" from ODF specification v1.1
    * were not found in [MS-PPT].pdf and are also ignored.
    *
    * @param value TextAlignmentEnum value to convert
    * @return value as string from ODF specification or an empty string if
    * conversion failed
    */
    QString textAlignmentToString(unsigned int value) const;

    /**
    * @brief convert Master Unit values to centimeters
    *
    * From [MS-PPT].pdf:
    * master unit: A unit of linear measurement that is equal to 1/576 inch.
    *
    * @param value master unit value to convert
    * @return value as centimeters with cm appended to the end
    */
    QString pptMasterUnitToCm(unsigned int value) const;

    /**
    * @brief Converts ColorIndexStruct to QColor
    *
    * Color index struct can either contain rgb values or a index to color
    * in MainMasterContainer's SlideSchemeColorSchemeAtom. This method
    * returns the rgb values the specified struct refers to.
    * @param color Color to convert
    * @return QColor value, may be undefined
    */
    QColor toQColor(const MSO::ColorIndexStruct &color);
    QColor toQColor(const MSO::OfficeArtCOLORREF& c);

    /**
    * TextAutoNumberSchemeEnum
    * Referenced by: TextAutoNumberScheme
    * An enumeration that specifies the character sequence and delimiters to use for automatic
    * numbering.
    */
    enum {
        ANM_AlphaLcPeriod,                  //0x0000     Example: a., b., c., ...Lowercase Latin character followed by a period.
        ANM_AlphaUcPeriod ,                 //0x0001     Example: A., B., C., ...Uppercase Latin character followed by a period.
        ANM_ArabicParenRight,               //0x0002     Example: 1), 2), 3), ...Arabic numeral followed by a closing parenthesis.
        ANM_ArabicPeriod,                   //0x0003     Example: 1., 2., 3., ...Arabic numeral followed by a period.
        ANM_RomanLcParenBoth,               //0x0004     Example: (i), (ii), (iii), ...Lowercase Roman numeral enclosed in parentheses.
        ANM_RomanLcParenRight,              //0x0005     Example: i), ii), iii), ... Lowercase Roman numeral followed by a closing parenthesis.
        ANM_RomanLcPeriod,                  //0x0006     Example: i., ii., iii., ...Lowercase Roman numeral followed by a period.
        ANM_RomanUcPeriod ,                 //0x0007     Example: I., II., III., ...Uppercase Roman numeral followed by a period.
        ANM_AlphaLcParenBoth,               //0x0008     Example: (a), (b), (c), ...Lowercase alphabetic character enclosed in parentheses.
        ANM_AlphaLcParenRight,              //0x0009     Example: a), b), c), ...Lowercase alphabetic character followed by a closing
        ANM_AlphaUcParenBoth,               //0x000A     Example: (A), (B), (C), ...Uppercase alphabetic character enclosed in parentheses.
        ANM_AlphaUcParenRight,              //0x000B     Example: A), B), C), ...Uppercase alphabetic character followed by a closing
        ANM_ArabicParenBoth,                //0x000C     Example: (1), (2), (3), ...Arabic numeral enclosed in parentheses.
        ANM_ArabicPlain,                    //0x000D     Example: 1, 2, 3, ...Arabic numeral.
        ANM_RomanUcParenBoth,               //0x000E     Example: (I), (II), (III), ...Uppercase Roman numeral enclosed in parentheses.
        ANM_RomanUcParenRight,              //0x000F     Example: I), II), III), ...Uppercase Roman numeral followed by a closing parenthesis.
        //Future
    }TextAutoNumberSchemeEnum;

    /**
    * Declaration Type
    * Referenced by: Declaration Type
    * A declaration type ex:- Header,Footer,DateTime
    */
    enum DeclarationType {
        Footer,
        Header,
        DateTime
    };

    /**
    * @brief processTextAutoNumberScheme : process the Textautoscheme to display the Bullet and numbering.
    * @param TextAutomNumberSchemeEnum - enum values of textautoscheme.
    * @param numFormat - Format of the bulletand numbering scheme
    * @param numSuffix - Suffix of the numFormat
    * @param numPrefix - suffix of prefix
    * @return none
    */
    void processTextAutoNumberScheme(int val, QString& numFormat, QString& numSuffix, QString& numPrefix);


    void processEllipse(const MSO::OfficeArtSpContainer& fsp, Writer& out);
    void processRectangle(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processRoundRectangle(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processDiamond(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processTriangle(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processTrapezoid(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processParallelogram(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processHexagon(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processOctagon(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processArrow(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processLine(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processSmiley(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processHeart(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processFreeLine(const MSO::OfficeArtSpContainer& o, Writer& out);
    void processPictureFrame(const MSO::OfficeArtSpContainer& o, Writer& out);

    /**
    * @brief Struct that contains precalculated style names based on
    * TextCFException and TextPFException combinations.
    *
    * For each individual character in this object's text three styles apply:
    * Paragraph style, List style and Character style. These are parsed from
    * TextCFException and TextPFException. For each character there is a
    * corresponding pair of TextCFException and TextPFException.
    *
    * Saving of styles is done before saving text contents so we'll cache
    * the style names and pairs of TextCFException and TextPFException.
    *
    *
    */
    class StyleName
    {
    public:
        /**
        * @brief Text style name (e.g. T1)
        *
        */
        QString text;

        /**
        * @brief Paragraph style (e.g. P1)
        *
        */
        QString paragraph;

        /**
        * @brief List style (e.g. L1)
        *
        */
        QString list;
        StyleName() {}
        StyleName(const QString& t, const QString& p, const QString& l)
                : text(t), paragraph(p), list(l) {}
    };

    QMap<QByteArray, QString> pictureNames;
    QMap<quint16, QString> bulletPictureNames;
    DateTimeFormat dateTime;
    QString declarationStyleName;

    /**
      * name for to use in the style:page-layout-name attribute for master
      * slides (style:master-page)
      */
    QString slidePageLayoutName;
    /**
      * name for to use in the style:page-layout-name attribute for notes
      * and handout slides (presentation:notes and style:handout-master)
      */
    QString notesPageLayoutName;

    const ParsedPresentation* p;

    const MSO::SlideListWithTextSubContainerOrAtom* currentSlideTexts;

    const MSO::MasterOrSlideContainer* currentMaster;

    bool parse(POLE::Storage& storage);

    /**
     * There is at most one SlideHeadersFootersContainer, but for some slides
     * it is in a strange positions. This convenience function returns a pointer
     * to the SlideHeadersFootersContainer or NULL if there is none.
     **/
    const MSO::SlideHeadersFootersContainer* getSlideHF() const {
        return (p->documentContainer->slideHF)
               ? p->documentContainer->slideHF.data()
               : p->documentContainer->slideHF2.data();
    }

    /**
      * Look in blipStore for the id mapping to this object
      **/
    QByteArray getRgbUid(quint16 pib) const {
        // return 16 byte rgbuid for this given blip id
        if (p->documentContainer->drawingGroup.OfficeArtDgg.blipStore) {
            const MSO::OfficeArtBStoreContainer* b
            = p->documentContainer->drawingGroup.OfficeArtDgg.blipStore.data();
            if (pib < b->rgfb.size()
                    && b->rgfb[pib].anon.is<MSO::OfficeArtFBSE>()) {
                return b->rgfb[pib].anon.get<MSO::OfficeArtFBSE>()->rgbUid;
            }
        }
        qDebug() << "Could not find image for pib " << pib;
        return QByteArray();
    }
    const MSO::FontEntityAtom*
    getFont(quint16 fontRef) {
        const MSO::FontCollectionContainer* f =
            p->documentContainer->documentTextInfo.fontCollection.data();
        if (f && f->rgFontCollectionEntry.size() > fontRef) {
            return &f->rgFontCollectionEntry[fontRef].fontEntityAtom;
        }
        return 0;
    }
    QRect getRect(const MSO::OfficeArtSpContainer &o);

    QMap<const void*, QString> presentationPageLayouts;
    QMap<const void*, QString> drawingPageStyles;
    typedef QMap<const MSO::MasterOrSlideContainer*, QMap<int, QString> >
            MasterStyles;
    MasterStyles masterGraphicStyles;
    MasterStyles masterPresentationStyles;
    QMap<const MSO::MasterOrSlideContainer*, QString> masterNames;
    QString notesMasterName;

    /**
    * @brief An usedDeclaration.
    * settings for slideNo &  usedeclaration name.
    */
    QHash<unsigned int/*slideNo*/,QString /*usedDeclarationName*/>usedFooterDeclaration;

    /**
    * @brief An usedDeclaration.
    * settings for slideNo &  usedeclaration name.
    */
    QHash<unsigned int/*slideNo*/,QString/*usedDeclarationName*/>usedHeaderDeclaration;

    /**
    * @brief An usedDeclaration.
    * settings for slideNo &  usedeclaration name.
    */
    QHash<unsigned int/*slideNo*/,QString/*usedDeclarationName*/>usedDateTimeDeclaration;

    /**
    * @brief An declaration.
    * settings for declaration text and usedeclaration name.
    */
    QHash<DeclarationType/*type*/,QPair<QString/*declarationName*/,QString/*text*/> >declaration;

    /**
    * @brief An notesDeclaration.
    * settings for notes declaration  text and usenotes declaration name.
    */
    QHash<DeclarationType/*type*/,QPair<QString /*declarationName*/,QString/*text*/> >notesDeclaration;

    /**
    * @brief find the text from  Declaration.
    * @return pointer of the use name.
    */
    QString findDeclaration(DeclarationType type, const QString &text) const;
    /**
    * @brief find the text from  notesDeclaration.
    * @return pointer of the use notes name.
    */
    QString findNotesDeclaration(DeclarationType type, const QString &) const;

    /**
    * @brief insert the text into  Declaration.
    * @param QString declaration use name string ex: ftr1
    * @param QString declaration text to displayed.
    */
    void insertDeclaration(DeclarationType type, const QString &name, const QString &text);
    /**
    * @brief insert the text into  notesDeclaration.
    * @param QString notes declaration use name string ex: ftr2
    * @param QString notes text to displayed.
    */
    void insertNotesDeclaration(DeclarationType type, const QString &name, const QString &text);

    /**
    * @brief set the width, height rotation and starting point for the given container
    */
    void set2dGeometry(const MSO::OfficeArtSpContainer& o, Writer& out);
};


/**
 * Retrieve an option from an options containing class B
 *
 * @p b must have a member fopt that is an array of
 * type OfficeArtFOPTEChoice.
 * A is the type of the required option. The option containers
 * in PPT have only one instance of each option in a option container.
 * @param b class that contains options.
 * @return pointer to the option of type A or 0 if there is none.
 */
template <typename A, typename B>
const A*
get(const B& b)
{
    foreach(const MSO::OfficeArtFOPTEChoice& a, b.fopt) {
        const A *ptr = a.anon.get<A>();
        if (ptr) return ptr;
    }
    return 0;
}
/**
 * Retrieve an option from an OfficeArtSpContainer
 *
 * Look in all option containers in @p o for an option of type A.
 * @param o OfficeArtSpContainer instance which contains options.
 * @return pointer to the option of type A or 0 if there is none.
 */
template <typename A>
const A*
get(const MSO::OfficeArtSpContainer& o)
{
    const A* a = 0;
    if (o.shapePrimaryOptions) a = get<A>(*o.shapePrimaryOptions);
    if (!a && o.shapeSecondaryOptions1) a = get<A>(*o.shapeSecondaryOptions1);
    if (!a && o.shapeSecondaryOptions2) a = get<A>(*o.shapeSecondaryOptions2);
    if (!a && o.shapeTertiaryOptions1) a = get<A>(*o.shapeTertiaryOptions1);
    if (!a && o.shapeTertiaryOptions2) a = get<A>(*o.shapeTertiaryOptions2);
    return a;
}
/**
 * Retrieve an option from an OfficeArtDggContainer
 *
 * Look in all option containers in @p o for an option of type A.
 * @param o OfficeArtDggContainer instance which contains options.
 * @return pointer to the option of type A or 0 if there is none.
 */
template <typename A>
const A*
get(const MSO::OfficeArtDggContainer& o)
{
    const A* a = 0;
    if (o.drawingPrimaryOptions) {
        get<A>(*o.drawingPrimaryOptions);
    }
    if (!a && o.drawingTertiaryOptions) a = get<A>(*o.drawingTertiaryOptions);
    return a;
}
/**
 * Retrieve an option from a container
 *
 * Look in all option containers in @p o for an option of type A.
 * @param o OfficeArtDggContainer instance which contains options.
 * @return pointer to the option of type A or 0 if there is none.
 */
template <typename A, typename T>
const A*
get(const T* o)
{
    return (o) ?get<A>(*o) :0;
}

/**
 * Convert FixedPoint to a qreal
 */
inline qreal toQReal(const MSO::FixedPoint& f)
{
    return f.integral + f.fractional / 65536.0;
}

#endif // POWERPOINTIMPORT_H
