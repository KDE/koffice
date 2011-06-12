/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009-2011 Thomas Zander <zander@kde.org>
   Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
   Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2010 KO GmbH <jos.van.den.oever@kogmbh.com>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KOdfGenericStyles.h"

#include <KoStore.h>
#include <KOdfStorageDevice.h>
#include <KoXmlWriter.h>
#include "KOdfWriteStore.h"
#include "KOdfFontData.h"
#include <float.h>
#include <kdebug.h>

static const struct {
    KOdfGenericStyle::Type m_type;
    const char * m_elementName;
    const char * m_propertiesElementName;
    bool m_drawElement;
} styleData[] = {
    { KOdfGenericStyle::TextStyle,            "style:style", "style:text-properties",         false  },
    { KOdfGenericStyle::ParagraphStyle,       "style:style", "style:paragraph-properties",    false  },
    { KOdfGenericStyle::SectionStyle,         "style:style", "style:section-properties",      false  },
    { KOdfGenericStyle::RubyStyle,            "style:style", "style:ruby-properties",         false  },
    { KOdfGenericStyle::TableStyle,           "style:style", "style:table-properties",        false  },
    { KOdfGenericStyle::TableColumnStyle,     "style:style", "style:table-column-properties", false  },
    { KOdfGenericStyle::TableRowStyle,        "style:style", "style:table-row-properties",    false  },
    { KOdfGenericStyle::TableCellStyle,       "style:style", "style:table-cell-properties",   false  },
    { KOdfGenericStyle::GraphicStyle,         "style:style", "style:graphic-properties",      false  },
    { KOdfGenericStyle::PresentationStyle,    "style:style", "style:graphic-properties",      false  },
    { KOdfGenericStyle::DrawingPageStyle,     "style:style", "style:drawing-page-properties", false  },
    { KOdfGenericStyle::ChartStyle,           "style:style", "style:chart-properties",        false  },
    { KOdfGenericStyle::ListStyle,            "text:list-style", 0, false  },
    { KOdfGenericStyle::LinearGradientStyle,  "svg:linearGradient", 0, true  },
    { KOdfGenericStyle::RadialGradientStyle,  "svg:radialGradient", 0, true  },
    { KOdfGenericStyle::ConicalGradientStyle, "koffice:conicalGradient", 0, true  },
    { KOdfGenericStyle::StrokeDashStyle,      "draw:stroke-dash", 0, true  },
    { KOdfGenericStyle::FillImageStyle,       "draw:fill-image", 0, true  },
    { KOdfGenericStyle::HatchStyle,           "draw:hatch", "style:graphic-properties", true  },
    { KOdfGenericStyle::GradientStyle,        "draw:gradient", "style:graphic-properties", true  },
    { KOdfGenericStyle::MarkerStyle,          "draw:marker", "style:graphic-properties", true  },
    { KOdfGenericStyle::PresentationPageLayoutStyle, "style:presentation-page-layout", 0, false  }
};

static const unsigned int numStyleData = sizeof(styleData) / sizeof(*styleData);

static const struct {
    KOdfGenericStyle::Type m_type;
    const char * m_elementName;
    const char * m_propertiesElementName;
    bool m_drawElement;
} autoStyleData[] = {
    { KOdfGenericStyle::TextAutoStyle,         "style:style", "style:text-properties",         false  },
    { KOdfGenericStyle::ParagraphAutoStyle,    "style:style", "style:paragraph-properties",    false  },
    { KOdfGenericStyle::SectionAutoStyle,      "style:style", "style:section-properties",      false  },
    { KOdfGenericStyle::RubyAutoStyle,         "style:style", "style:ruby-properties",         false  },
    { KOdfGenericStyle::TableAutoStyle,        "style:style", "style:table-properties",        false  },
    { KOdfGenericStyle::TableColumnAutoStyle,  "style:style", "style:table-column-properties", false  },
    { KOdfGenericStyle::TableRowAutoStyle,     "style:style", "style:table-row-properties",    false  },
    { KOdfGenericStyle::TableCellAutoStyle,    "style:style", "style:table-cell-properties",   false  },
    { KOdfGenericStyle::GraphicAutoStyle,      "style:style", "style:graphic-properties",      false  },
    { KOdfGenericStyle::PresentationAutoStyle, "style:style", "style:graphic-properties",      false  },
    { KOdfGenericStyle::DrawingPageAutoStyle,  "style:style", "style:drawing-page-properties", false  },
    { KOdfGenericStyle::ChartAutoStyle,        "style:style", "style:chart-properties",        false  },
    { KOdfGenericStyle::PageLayoutStyle, "style:page-layout", "style:page-layout-properties",  false  },
    { KOdfGenericStyle::ListAutoStyle, "text:list-style", 0, false  },
    { KOdfGenericStyle::NumericNumberStyle, "number:number-style", 0, false  },
    { KOdfGenericStyle::NumericFractionStyle, "number:number-style", 0, false  },
    { KOdfGenericStyle::NumericScientificStyle, "number:number-style", 0, false  },
    { KOdfGenericStyle::NumericDateStyle, "number:date-style", 0, false  },
    { KOdfGenericStyle::NumericTimeStyle, "number:time-style", 0, false  },
    { KOdfGenericStyle::NumericPercentageStyle, "number:percentage-style", 0, false  },
    { KOdfGenericStyle::NumericCurrencyStyle, "number:currency-style", 0, false  },
    { KOdfGenericStyle::NumericBooleanStyle, "number:boolean-style", 0, false  },
    { KOdfGenericStyle::NumericTextStyle, "number:text-style", 0, false  }
};

static const unsigned int numAutoStyleData = sizeof(autoStyleData) / sizeof(*autoStyleData);

static void insertRawOdfStyles(const QByteArray& xml, QByteArray& styles)
{
    if (xml.isEmpty())
        return;
    if (!styles.isEmpty() && !styles.endsWith('\n') && !xml.startsWith('\n')) {
        styles.append('\n');
    }
    styles.append(xml);
}

class KOdfGenericStyles::Private
{
public:
    Private(KOdfGenericStyles *q) : q(q)
    {
    }

    ~Private()
    {
    }

    QList<KOdfGenericStyles::NamedStyle> styles(const QSet<QString>& names, KOdfGenericStyle::Type type) const;
    void saveOdfAutomaticStyles(KoXmlWriter* xmlWriter, const QSet<QString>& styleNames,
                                const QByteArray& rawOdfAutomaticStyles) const;
    void saveOdfDocumentStyles(KoXmlWriter* xmlWriter) const;
    void saveOdfMasterStyles(KoXmlWriter* xmlWriter) const;
    QString makeUniqueName(const QString& base, InsertionFlags flags) const;

    /**
     * Save font face declarations
     *
     * This creates the office:font-face-decls tag containing all font face
     * declarations
     */
    void saveOdfFontFaceDecls(KoXmlWriter* xmlWriter) const;

    /// style definition -> name
    StyleMap styleMap;

    /// Map with the style name as key.
    /// This map is mainly used to check for name uniqueness
    QSet<QString> styleNames;
    QSet<QString> autoStylesInStylesDotXml;

    /// List of styles (used to preserve ordering)
    QList<KOdfGenericStyles::NamedStyle> styleList;

    /// map for saving default styles
    QMap<int, KOdfGenericStyle> defaultStyles;

    /// font faces
    QMap<QString, KOdfFontData> fontFaces;

    StyleMap::iterator insertStyle(const KOdfGenericStyle &style, const QString &name, InsertionFlags flags);

    struct RelationTarget {
        QString target; // the style we point to
        QString attribute; // the attribute name used for the relation
    };
    QHash<QString, RelationTarget> relations; // key is the name of the source style

    QByteArray rawOdfDocumentStyles;
    QByteArray rawOdfAutomaticStyles_stylesDotXml;
    QByteArray rawOdfAutomaticStyles_contentDotXml;
    QByteArray rawOdfMasterStyles;
    QByteArray rawOdfFontFaceDecls;

    KOdfGenericStyles *q;
};

QList<KOdfGenericStyles::NamedStyle> KOdfGenericStyles::Private::styles(const QSet<QString>& names, KOdfGenericStyle::Type type) const
{
    QList<KOdfGenericStyles::NamedStyle> lst;
    QList<KOdfGenericStyles::NamedStyle>::const_iterator it = styleList.constBegin();
    const QList<KOdfGenericStyles::NamedStyle>::const_iterator end = styleList.constEnd();
    const QSet<QString>::const_iterator mapEnd = names.constEnd();
    for (; it != end ; ++it) {
        if ((*it).style->type() == type && names.constFind((*it).name) != mapEnd) {
            lst.append(*it);
        }
    }
    return lst;
}

void KOdfGenericStyles::Private::saveOdfAutomaticStyles(KoXmlWriter* xmlWriter, const QSet<QString>& styleNames,
                                                  const QByteArray& rawOdfAutomaticStyles) const
{
    xmlWriter->startElement("office:automatic-styles");

    for (uint i = 0; i < numAutoStyleData; ++i) {
        QList<KOdfGenericStyles::NamedStyle> stylesList = styles(styleNames, autoStyleData[i].m_type);
        QList<KOdfGenericStyles::NamedStyle>::const_iterator it = stylesList.constBegin();
        for (; it != stylesList.constEnd() ; ++it) {
            (*it).style->writeStyle(xmlWriter, *q, autoStyleData[i].m_elementName, (*it).name,
                                    autoStyleData[i].m_propertiesElementName, true, autoStyleData[i].m_drawElement);
        }
    }

    if (!rawOdfAutomaticStyles.isEmpty()) {
        xmlWriter->addCompleteElement(rawOdfAutomaticStyles.constData());
    }

    xmlWriter->endElement(); // office:automatic-styles
}

void KOdfGenericStyles::Private::saveOdfDocumentStyles(KoXmlWriter* xmlWriter) const
{
    xmlWriter->startElement("office:styles");

    for (uint i = 0; i < numStyleData; ++i) {
        const QMap<int, KOdfGenericStyle>::const_iterator it(defaultStyles.constFind(styleData[i].m_type));
        if (it != defaultStyles.constEnd()) {
            it.value().writeStyle(xmlWriter, *q, "style:default-style", "",
                                  styleData[i].m_propertiesElementName, true, styleData[i].m_drawElement);
        }
    }

    for (uint i = 0; i < numStyleData; ++i) {
        QList<KOdfGenericStyles::NamedStyle> stylesList(q->styles(styleData[i].m_type));
        QList<KOdfGenericStyles::NamedStyle>::const_iterator it = stylesList.constBegin();
        for (; it != stylesList.constEnd() ; ++it) {
            if (relations.contains(it->name)) {
                KOdfGenericStyles::Private::RelationTarget relation = relations.value(it->name);
                KOdfGenericStyle styleCopy = *(*it).style;
                styleCopy.addAttribute(relation.attribute, relation.target);
                styleCopy.writeStyle(xmlWriter, *q, styleData[i].m_elementName, (*it).name,
                                    styleData[i].m_propertiesElementName, true, styleData[i].m_drawElement);
            } else {
                (*it).style->writeStyle(xmlWriter, *q, styleData[i].m_elementName, (*it).name,
                                    styleData[i].m_propertiesElementName, true, styleData[i].m_drawElement);
            }
        }
    }

    if (!rawOdfDocumentStyles.isEmpty()) {
        xmlWriter->addCompleteElement(rawOdfDocumentStyles.constData());
    }

    xmlWriter->endElement(); // office:styles
}

void KOdfGenericStyles::Private::saveOdfMasterStyles(KoXmlWriter* xmlWriter) const
{
    xmlWriter->startElement("office:master-styles");

    QList<KOdfGenericStyles::NamedStyle> stylesList = q->styles(KOdfGenericStyle::MasterPageStyle);
    QList<KOdfGenericStyles::NamedStyle>::const_iterator it = stylesList.constBegin();
    for (; it != stylesList.constEnd() ; ++it) {
        (*it).style->writeStyle(xmlWriter, *q, "style:master-page", (*it).name, 0);
    }

    if (!rawOdfMasterStyles.isEmpty()) {
        xmlWriter->addCompleteElement(rawOdfMasterStyles.constData());
    }

    xmlWriter->endElement(); // office:master-styles
}

void KOdfGenericStyles::Private::saveOdfFontFaceDecls(KoXmlWriter* xmlWriter) const
{
    if (fontFaces.isEmpty())
        return;

    xmlWriter->startElement("office:font-face-decls");
    for (QMap<QString, KOdfFontData>::ConstIterator it(fontFaces.constBegin());
         it != fontFaces.constEnd(); ++it)
    {
        it.value().saveOdf(xmlWriter);
    }

    if (!rawOdfFontFaceDecls.isEmpty()) {
        xmlWriter->addCompleteElement(rawOdfFontFaceDecls.constData());
    }

    xmlWriter->endElement(); // office:font-face-decls
}

QString KOdfGenericStyles::Private::makeUniqueName(const QString& base, InsertionFlags flags) const
{
    // If this name is not used yet, and numbering isn't forced, then the given name is ok.
    if ((flags & DontAddNumberToName)
            && !autoStylesInStylesDotXml.contains(base)
            && !styleNames.contains(base))
        return base;
    int num = 1;
    QString name;
    do {
        name = base + QString::number(num++);
    } while (autoStylesInStylesDotXml.contains(name)
             || styleNames.contains(name));
    return name;
}

//------------------------

KOdfGenericStyles::KOdfGenericStyles()
        : d(new Private(this))
{
}

KOdfGenericStyles::~KOdfGenericStyles()
{
    delete d;
}

QString KOdfGenericStyles::insert(const KOdfGenericStyle& style, const QString& baseName, InsertionFlags flags)
{
    // if it is a default style it has to be saved differently
    if (style.isDefaultStyle()) {
        // we can have only one default style per type
        Q_ASSERT(!d->defaultStyles.contains(style.type()));
        // default style is only possible for style:style in office:style types
        Q_ASSERT(style.type() == KOdfGenericStyle::TextStyle ||
                 style.type() == KOdfGenericStyle::ParagraphStyle ||
                 style.type() == KOdfGenericStyle::SectionStyle ||
                 style.type() == KOdfGenericStyle::RubyStyle ||
                 style.type() == KOdfGenericStyle::TableStyle ||
                 style.type() == KOdfGenericStyle::TableColumnStyle ||
                 style.type() == KOdfGenericStyle::TableRowStyle ||
                 style.type() == KOdfGenericStyle::TableCellStyle ||
                 style.type() == KOdfGenericStyle::GraphicStyle ||
                 style.type() == KOdfGenericStyle::PresentationStyle ||
                 style.type() == KOdfGenericStyle::DrawingPageStyle ||
                 style.type() == KOdfGenericStyle::ChartStyle);

        d->defaultStyles.insert(style.type(), style);
        // default styles don't have a name
        return QString();
    }

    if (flags & AllowDuplicates) {
        StyleMap::iterator it = d->insertStyle(style, baseName, flags);
        return it.value();
    }

    StyleMap::iterator it = d->styleMap.find(style);
    if (it == d->styleMap.end()) {
        // Not found, try if this style is in fact equal to its parent (the find above
        // wouldn't have found it, due to m_parentName being set).
        if (!style.parentName().isEmpty()) {
            KOdfGenericStyle testStyle(style);
            const KOdfGenericStyle* parentStyle = this->style(style.parentName(), style.m_familyName);   // ## linear search
            if (!parentStyle) {
                kDebug(30003) << "baseName=" << baseName << "parent style" << style.parentName()
                              << "not found in collection";
            } else {
                Q_ASSERT(testStyle.m_familyName == parentStyle->m_familyName);

                testStyle.m_parentName = parentStyle->m_parentName;
                // Exclude the type from the comparison. It's ok for an auto style
                // to have a user style as parent; they can still be identical
                testStyle.m_type = parentStyle->m_type;
                // Also it's ok to not have the display name of the parent style
                // in the auto style
                QMap<QString, QString>::const_iterator it = parentStyle->m_attributes.find("style:display-name");
                if (it != parentStyle->m_attributes.end())
                    testStyle.addAttribute("style:display-name", *it);

                if (*parentStyle == testStyle)
                    return style.parentName();
            }
        }

        it = d->insertStyle(style, baseName, flags);
    }
    return it.value();
}

KOdfGenericStyles::StyleMap::iterator KOdfGenericStyles::Private::insertStyle(const KOdfGenericStyle &style,
                                                                  const QString& baseName, InsertionFlags flags)
{
    QString styleName(baseName);
    if (styleName.isEmpty()) {
        switch (style.type()) {
        case KOdfGenericStyle::ParagraphAutoStyle: styleName = 'P'; break;
        case KOdfGenericStyle::ListAutoStyle: styleName = 'L'; break;
        case KOdfGenericStyle::TextAutoStyle: styleName = 'T'; break;
        default:
            styleName = 'A'; // for "auto".
        }
        flags &= ~DontAddNumberToName; // i.e. force numbering
    }
    styleName = makeUniqueName(styleName, flags);
    if (style.autoStyleInStylesDotXml())
        autoStylesInStylesDotXml.insert(styleName);
    else
        styleNames.insert(styleName);
    KOdfGenericStyles::StyleMap::iterator it = styleMap.insert(style, styleName);
    NamedStyle s;
    s.style = &it.key();
    s.name = styleName;
    styleList.append(s);
    return it;
}

KOdfGenericStyles::StyleMap KOdfGenericStyles::styles() const
{
    return d->styleMap;
}

QList<KOdfGenericStyles::NamedStyle> KOdfGenericStyles::styles(KOdfGenericStyle::Type type) const
{
    return d->styles(d->styleNames, type);
}

QList<KOdfGenericStyles::NamedStyle> KOdfGenericStyles::stylesForStylesXml(KOdfGenericStyle::Type type) const
{
    return d->styles(d->autoStylesInStylesDotXml, type);
}

const KOdfGenericStyle* KOdfGenericStyles::style(const QString &name, const QByteArray &family) const
{
    QList<KOdfGenericStyles::NamedStyle>::const_iterator it = d->styleList.constBegin();
    const QList<KOdfGenericStyles::NamedStyle>::const_iterator end = d->styleList.constEnd();
    for (; it != end ; ++it) {
        if ((*it).name == name && (family.isEmpty() || (*it).style->m_familyName == family))
            return (*it).style;
    }
    return 0;
}

KOdfGenericStyle* KOdfGenericStyles::styleForModification(const QString &name)
{
    return const_cast<KOdfGenericStyle *>(style(name, QByteArray()));
}

void KOdfGenericStyles::markStyleForStylesXml(const QString& name)
{
    Q_ASSERT(d->styleNames.contains(name));
    d->styleNames.remove(name);
    d->autoStylesInStylesDotXml.insert(name);
    styleForModification(name)->setAutoStyleInStylesDotXml(true);
}

void KOdfGenericStyles::insertFontFace(const KOdfFontData &face)
{
    Q_ASSERT(!face.isNull());
    if (face.isNull()) {
        kWarning() << "This font face is null and will not be added to styles: set at least the name";
        return;
    }
    d->fontFaces.insert(face.name(), face); // replaces prev item
}

KOdfFontData KOdfGenericStyles::fontFace(const QString& name) const
{
    return d->fontFaces.value(name);
}

bool KOdfGenericStyles::saveOdfStylesDotXml(KoStore* store, KoXmlWriter* manifestWriter) const
{
    if (!store->open("styles.xml"))
        return false;

    manifestWriter->addManifestEntry("styles.xml",  "text/xml");

    KOdfStorageDevice stylesDev(store);
    KoXmlWriter* stylesWriter = KOdfWriteStore::createOasisXmlWriter(&stylesDev, "office:document-styles");

    d->saveOdfFontFaceDecls(stylesWriter);
    d->saveOdfDocumentStyles(stylesWriter);
    d->saveOdfAutomaticStyles(stylesWriter, d->autoStylesInStylesDotXml, d->rawOdfAutomaticStyles_stylesDotXml);
    d->saveOdfMasterStyles(stylesWriter);

    stylesWriter->endElement(); // root element (office:document-styles)
    stylesWriter->endDocument();
    delete stylesWriter;

    if (!store->close())   // done with styles.xml
        return false;

    return true;
}

void KOdfGenericStyles::saveOdfStyles(StylesPlacement placement, KoXmlWriter* xmlWriter) const
{
    switch (placement) {
    case DocumentStyles:
        d->saveOdfDocumentStyles(xmlWriter);
        break;
    case MasterStyles:
        d->saveOdfMasterStyles(xmlWriter);
        break;
    case DocumentAutomaticStyles:
        d->saveOdfAutomaticStyles(xmlWriter, d->styleNames, d->rawOdfAutomaticStyles_contentDotXml);
        break;
    case StylesXmlAutomaticStyles:
        d->saveOdfAutomaticStyles(xmlWriter, d->autoStylesInStylesDotXml, d->rawOdfAutomaticStyles_stylesDotXml);
        break;
    case FontFaceDecls:
        d->saveOdfFontFaceDecls(xmlWriter);
        break;
    }
}

void KOdfGenericStyles::insertRawOdfStyles(StylesPlacement placement, const QByteArray& xml)
{
    switch (placement) {
    case DocumentStyles:
        ::insertRawOdfStyles(xml, d->rawOdfDocumentStyles);
        break;
    case MasterStyles:
        ::insertRawOdfStyles(xml, d->rawOdfMasterStyles);
        break;
    case DocumentAutomaticStyles:
        ::insertRawOdfStyles(xml, d->rawOdfAutomaticStyles_contentDotXml);
        break;
    case StylesXmlAutomaticStyles:
        ::insertRawOdfStyles(xml, d->rawOdfAutomaticStyles_stylesDotXml);
        break;
    case FontFaceDecls:
        ::insertRawOdfStyles(xml, d->rawOdfFontFaceDecls);
        break;
    }
}

void KOdfGenericStyles::insertStyleRelation(const QString &source, const QString &target, const char *tagName)
{
    KOdfGenericStyles::Private::RelationTarget relation;
    relation.target = target;
    relation.attribute = QString(tagName);
    d->relations.insert(source, relation);
}

QDebug operator<<(QDebug dbg, const KOdfGenericStyles& styles)
{
    dbg.nospace() << "KOdfGenericStyles:";
    QList<KOdfGenericStyles::NamedStyle>::const_iterator it = styles.d->styleList.constBegin();
    const QList<KOdfGenericStyles::NamedStyle>::const_iterator end = styles.d->styleList.constEnd();
    for (; it != end ; ++it) {
        dbg.nospace() << (*it).name;
    }
    for (QSet<QString>::const_iterator it = styles.d->styleNames.constBegin(); it != styles.d->styleNames.constEnd(); ++it) {
        dbg.space() << "style:" << *it;
    }
#ifndef NDEBUG
    for (QSet<QString>::const_iterator it = styles.d->autoStylesInStylesDotXml.constBegin();
         it != styles.d->autoStylesInStylesDotXml.constEnd(); ++it)
    {
        dbg.space() << "auto style for style.xml:" << *it;
        const KOdfGenericStyle* s = styles.style(*it, QByteArray());
        Q_ASSERT(s);
        Q_ASSERT(s->autoStyleInStylesDotXml());
    }
#endif
    return dbg.space();
}
