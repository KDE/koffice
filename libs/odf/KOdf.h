/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOODF_H
#define KOODF_H

#include "kodf_export.h"
#include "KXmlReader.h"

#include <QtCore/QString>
#include <QtCore/QPair>
#include <QtGui/QTransform>


class QBrush;
class QPen;
class QString;
class QSizeF;

class KOdfGenericStyle;
class KOdfGenericStyles;
class KOdfStyleStack;

class KOdfStylesReader;
class KOdfLoadingContext;
class KXmlWriter;

namespace KOdf
{
    enum DocumentType {
        TextDocument,
        GraphicsDocument,
        PresentationDocument,
        SpreadsheetDocument,
        ChartDocument,
        ImageDocument,
        FormulaDocument,
        OpenOfficeClipboard
    };

    /**
     * Get the mime type
     *
     * @param documentType the document type
     * @return the mime type used for the given document type
     */
    KODF_EXPORT const char * mimeType(DocumentType documentType);

    /**
     * Get the mime type
     *
     * @param documentType the document type
     * @return the mime type used for templates of the given document type
     */
    KODF_EXPORT const char * templateMimeType(DocumentType documentType);

    /**
     * Get the mime type
     *
     * @param documentType the document type
     * @param withNamespace if true the namespace before the element is also returned
     *                      if false only the element is returned
     * @return the body element name for the given document type
     */
    KODF_EXPORT const char * bodyContentElement(DocumentType documentType, bool withNamespace);

    // -------------------- number styles methods.
    enum Format {
        NumberFormat,
        ScientificFormat,
        FractionFormat,
        CurrencyFormat,
        PercentageFormat,
        DateFormat,
        TimeFormat,
        BooleanFormat,
        TextFormat
    };

    /// Prefix and suffix are always included into formatStr. Having them as separate fields simply
    /// allows to extract them from formatStr, to display them in separate widgets.
    struct NumericStyleFormat {
        QString formatStr;
        QString prefix;
        QString suffix;
        Format type;
        int precision;
        QString currencySymbol;
        QList<QPair<QString,QString> > styleMaps; // conditional formatting, first=condition, second=applyStyleName
    };

    KODF_EXPORT QPair<QString, NumericStyleFormat> loadOdfNumberStyle(const KoXmlElement &parent);

    KODF_EXPORT QString saveOdfDateStyle(KOdfGenericStyles &mainStyles, const QString &format, bool klocaleFormat, const QString &prefix = QString(), const QString &suffix = QString());
    KODF_EXPORT QString saveOdfTimeStyle(KOdfGenericStyles &mainStyles, const QString &format, bool klocaleFormat, const QString &prefix = QString(), const QString &suffix = QString());
    KODF_EXPORT QString saveOdfFractionStyle(KOdfGenericStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KODF_EXPORT QString saveOdfScientificStyle(KOdfGenericStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KODF_EXPORT QString saveOdfNumberStyle(KOdfGenericStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KODF_EXPORT QString saveOdfPercentageStyle(KOdfGenericStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());
    KODF_EXPORT QString saveOdfCurrencyStyle(KOdfGenericStyles &mainStyles, const QString &format, const QString &symbol, const QString &prefix = QString(), const QString &suffix = QString());
    KODF_EXPORT QString saveOdfTextStyle(KOdfGenericStyles &mainStyles, const QString &format, const QString &prefix = QString(), const QString &suffix = QString());

    // -------------------- Graphics Styles
    KODF_EXPORT void saveOdfFillStyle(KOdfGenericStyle &styleFill, KOdfGenericStyles& mainStyles, const QBrush &brush);

    KODF_EXPORT void saveOdfStrokeStyle(KOdfGenericStyle &styleStroke, KOdfGenericStyles &mainStyles, const QPen &pen);

    KODF_EXPORT QString saveOdfHatchStyle(KOdfGenericStyles &mainStyles, const QBrush &brush);

    /// Saves gradient style of brush into mainStyles and returns the styles name
    KODF_EXPORT QString saveOdfGradientStyle(KOdfGenericStyles &mainStyles, const QBrush &brush);

    /// Loads gradient style from style stack and stylesReader adapted to the given size and returns a brush
    KODF_EXPORT QBrush loadOdfGradientStyle(const KOdfStyleStack &styleStack, const KOdfStylesReader &stylesReader, const QSizeF &size);

    /// Loads gradient style with the given name from style stack and stylesReader adapted to the given size and returns a brush
    KODF_EXPORT QBrush loadOdfGradientStyleByName(const KOdfStylesReader &stylesReader, const QString &styleName, const QSizeF &size);

    KODF_EXPORT QBrush loadOdfFillStyle(const KOdfStyleStack &styleStack, const QString &fill,  const KOdfStylesReader &stylesReader);

    KODF_EXPORT QPen loadOdfStrokeStyle(const KOdfStyleStack &styleStack, const QString &stroke, const KOdfStylesReader &stylesReader);

    /// Helper function to parse a transformation attribute
    KODF_EXPORT QTransform loadTransformation(const QString &transformation);

    /// Helper function to create a transformation attribute
    KODF_EXPORT QString saveTransformation(const QTransform &transformation, bool appendTranslateUnit = true);

    KODF_EXPORT bool isEncryptionSupported();
}

#endif /* KOODF_H */
