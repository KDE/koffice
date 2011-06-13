/* This file is part of the KDE project
   Copyright (C) 2002 Norbert Andres <nandres@web.de>
   Copyright (C) 2004 Montel Laurent <montel@kde.org>

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

#ifndef OpenCalc_IMPORT_H__
#define OpenCalc_IMPORT_H__

#include "KCFormat.h"

#include <KoFilter.h>

#include <QHash>
#include <qdom.h>
#include <QByteArray>
#include <KXmlReader.h>
#include <QVariantList>

class KOdfStyleStack;
class KOdfStore;

class KCCell;
class KCConditional;
class KCDoc;
class KCSheet;
class KCStyle;
class KCValidity;
class KCValueParser;

class OpenCalcImport : public KoFilter
{
    Q_OBJECT
public:
    OpenCalcImport(QObject * parent, const QVariantList &);
    virtual ~OpenCalcImport();

    virtual KoFilter::ConversionStatus convert(QByteArray const & from, QByteArray const & to);


private:

    class OpenCalcPoint
    {
    public:
        OpenCalcPoint(QString const & str);

        QString table;
        QString translation;
        QPoint  topLeft;
        QPoint  botRight;
        bool    isRange;
    };

    enum bPos { Left, Top, Right, Bottom, Fall, GoUp, Border };

    KCDoc *    m_doc;
    KCStyle *  m_defaultStyle;

    KXmlDocument   m_content;
    KXmlDocument   m_meta;
    KXmlDocument   m_settings;

    QHash<QString, KXmlElement*>   m_styles;
    QHash<QString, KCStyle*> m_defaultStyles;
    QHash<QString, QString*>        m_formats;
    QMap<QString, KXmlElement> m_validationList;

    QStringList          m_namedAreas;

    int  readMetaData();
    bool parseBody(int numOfTables);
    void insertStyles(KXmlElement const & element);
    bool createStyleMap(KXmlDocument const & styles);
    bool readRowFormat(KXmlElement & rowNode, KXmlElement * rowStyle,
                       KCSheet * table, int & row, int & number, bool last);
    bool readColLayouts(KXmlElement & content, KCSheet * table);
    bool readRowsAndCells(KXmlElement & content, KCSheet * table);
    bool readCells(KXmlElement & rowNode, KCSheet  * table, int row, int & columns);
    void convertFormula(QString & text, QString const & f) const;
    void loadFontStyle(KCStyle * layout, KXmlElement const * font) const;
    void readInStyle(KCStyle * layout, KXmlElement const & style);
    void loadStyleProperties(KCStyle * layout, KXmlElement const & property) const;
    void loadBorder(KCStyle * layout, QString const & borderDef, bPos pos) const;
    void loadTableMasterStyle(KCSheet * table, QString const & stylename);
    QString * loadFormat(KXmlElement * element,
                         KCFormat::Type & formatType,
                         QString name);
    void checkForNamedAreas(QString & formula) const;
    void loadOasisCellValidation(const KXmlElement&body, const KCValueParser *parser);
    void loadOasisValidation(KCValidity val, const QString& validationName, const KCValueParser *parser);
    void loadOasisValidationCondition(KCValidity val, QString &valExpression, const KCValueParser *parser);
    void loadOasisAreaName(const KXmlElement&body);
    void loadOasisMasterLayoutPage(KCSheet * table, KOdfStyleStack &styleStack);
    void loadOasisValidationValue(KCValidity val, const QStringList &listVal, const KCValueParser *parser);
    QString translatePar(QString & par) const;
    void loadCondition(const KCCell& cell, const KXmlElement &property);
    void loadOasisCondition(const KCCell& cell, const KXmlElement &property);
    void loadOasisConditionValue(const QString &styleCondition, KCConditional &newCondition, const KCValueParser *parser);
    void loadOasisCondition(QString &valExpression, KCConditional &newCondition, const KCValueParser *parser);
    KoFilter::ConversionStatus loadAndParse(KXmlDocument& doc, const QString& fileName, KOdfStore *m_store);

    KoFilter::ConversionStatus openFile();
};

#endif // OpenCalc_IMPORT_H__

