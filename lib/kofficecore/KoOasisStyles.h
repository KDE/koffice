/* This file is part of the KDE project
   Copyright (C) 2004-2005 David Faure <faure@kde.org>

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

#ifndef KOOASISSTYLES_H
#define KOOASISSTYLES_H

#include <qdom.h>
#include <qdict.h>
#include <qvaluevector.h>
#include <qmap.h>
#include <koffice_export.h>

class KoGenStyles;
class KoXmlWriter;
class QBrush;
class KoGenStyle;
class KoStyleStack;

/**
 * Repository of styles used during loading of OASIS/OOo file
 */
class KOFFICECORE_EXPORT KoOasisStyles
{
public:
    KoOasisStyles();
    ~KoOasisStyles();

    /// Look into @p doc for styles and remember them
    /// @param doc document to look into
    /// @param stylesDotXml true when loading styles.xml, false otherwise
    void createStyleMap( const QDomDocument& doc, bool stylesDotXml );

    /// @return all styles (with tag "style:style", "style:page-layout" or "style:presentation-page-layout")
    /// hashed by name
    const QDict<QDomElement>& styles() const { return m_styles; }

    /// @return all automatic styles with tag "style:style" from styles.xml
    /// hashed by name
    const QDict<QDomElement>& stylesAutoStyles() const { return m_stylesAutoStyles; }

    /// @return the style:styles that are "user styles", i.e. those from office:styles
    /// styles() is used for lookup. userStyles() is used to load all user styles upfront.
    QValueVector<QDomElement> userStyles() const;

    /// @return the default style for a given family ("graphic","paragraph","table" etc.)
    /// Returns 0 if no default style for this family is available
    QDomElement* defaultStyle( const QString& family ) const;

    /// @return the office:style element
    const QDomElement& officeStyle() const { return m_officeStyle; }

    /// @return all list styles ("text:list-style" elements), hashed by name
    const QDict<QDomElement>& listStyles() const { return m_listStyles; }

    /// @return master pages ("style:master-page" elements), hashed by name
    const QDict<QDomElement>& masterPages() const { return m_masterPages; }

    /// @return draw styles ("draw:name" element), hashed by name
    const QDict<QDomElement>& drawStyles() const { return m_drawStyles; }

    //Prefix and suffix are always included into formatStr. Having them as separate fields simply
    //allows to extract them from formatStr, to display them in separate widgets.
    struct NumericStyleFormat
    {
        QString formatStr;
        QString prefix;
        QString suffix;
    };

    typedef QMap<QString, NumericStyleFormat> DataFormatsMap;
    /// Value (date/time/number...) formats found while parsing styles. Used e.g. for fields.
    /// Key: format name. Value:
    const DataFormatsMap& dataFormats() const { return m_dataFormats; }

    static QString saveOasisDateStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat );
    static QString saveOasisTimeStyle( KoGenStyles &mainStyles, const QString & _format, bool klocaleFormat );
    static QString saveOasisFractionStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisScientificStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisPercentageStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisCurrencyStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix = QString::null , const QString &_suffix= QString::null );
    static QString saveOasisTextStyle( KoGenStyles &mainStyles, const QString & _format, const QString &_prefix = QString::null , const QString &_suffix= QString::null );

    static void saveOasisFillStyle( KoGenStyle &styleFill, KoGenStyles& mainStyles, const QBrush & brush );
    static QString saveOasisHatchStyle( KoGenStyles& mainStyles, const QBrush &brush );

    static QBrush loadOasisFillStyle( const KoStyleStack &styleStack, const QString & fill,  const KoOasisStyles & oasisStyles );

private:
    /// Add styles to styles map
    void insertStyles( const QDomElement& styles, bool styleAutoStyles = false );

private:
    void insertOfficeStyles( const QDomElement& styles );
    void insertStyle( const QDomElement& style, bool styleAutoStyles );
    void importDataStyle( const QDomElement& parent );
    static bool saveOasisTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text, bool &antislash );
    static void parseOasisDateKlocale(KoXmlWriter &elementWriter, QString & format, QString & text );
    static bool saveOasisKlocaleTimeFormat( KoXmlWriter &elementWriter, QString & format, QString & text );
    static void parseOasisTimeKlocale(KoXmlWriter &elementWriter, QString & format, QString & text );
    static void addKofficeNumericStyleExtension( KoXmlWriter & elementWriter, const QString &_suffix, const QString &_prefix );

    KoOasisStyles( const KoOasisStyles & ); // forbidden
    KoOasisStyles& operator=( const KoOasisStyles & ); // forbidden

private:
    QDict<QDomElement>   m_styles;
    QDict<QDomElement>   m_stylesAutoStyles;
    QDict<QDomElement>   m_defaultStyle;
    QDomElement m_officeStyle;

    QDict<QDomElement>   m_masterPages;
    QDict<QDomElement>   m_listStyles;

    QDict<QDomElement>   m_drawStyles;
    DataFormatsMap m_dataFormats;

    class Private;
    Private *d;
};

#endif /* KOOASISSTYLES_H */
