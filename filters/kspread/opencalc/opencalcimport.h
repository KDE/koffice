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
#include <KoXmlReader.h>
#include <QVariantList>

class KoStyleStack;
class KoStore;

class KCCell;
class KCConditional;
class Doc;
class KCSheet;
class KCStyle;
class KCValidity;
class ValueParser;

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

    Doc *    m_doc;
    KCStyle *  m_defaultStyle;

    KoXmlDocument   m_content;
    KoXmlDocument   m_meta;
    KoXmlDocument   m_settings;

    QHash<QString, KoXmlElement*>   m_styles;
    QHash<QString, KCStyle*> m_defaultStyles;
    QHash<QString, QString*>        m_formats;
    QMap<QString, KoXmlElement> m_validationList;

    QStringList          m_namedAreas;

    int  readMetaData();
    bool parseBody(int numOfTables);
    void insertStyles(KoXmlElement const & element);
    bool createStyleMap(KoXmlDocument const & styles);
    bool readRowFormat(KoXmlElement & rowNode, KoXmlElement * rowStyle,
                       KCSheet * table, int & row, int & number, bool last);
    bool readColLayouts(KoXmlElement & content, KCSheet * table);
    bool readRowsAndCells(KoXmlElement & content, KCSheet * table);
    bool readCells(KoXmlElement & rowNode, KCSheet  * table, int row, int & columns);
    void convertFormula(QString & text, QString const & f) const;
    void loadFontStyle(KCStyle * layout, KoXmlElement const * font) const;
    void readInStyle(KCStyle * layout, KoXmlElement const & style);
    void loadStyleProperties(KCStyle * layout, KoXmlElement const & property) const;
    void loadBorder(KCStyle * layout, QString const & borderDef, bPos pos) const;
    void loadTableMasterStyle(KCSheet * table, QString const & stylename);
    QString * loadFormat(KoXmlElement * element,
                         KCFormat::Type & formatType,
                         QString name);
    void checkForNamedAreas(QString & formula) const;
    void loadOasisCellValidation(const KoXmlElement&body, const ValueParser *parser);
    void loadOasisValidation(KCValidity val, const QString& validationName, const ValueParser *parser);
    void loadOasisValidationCondition(KCValidity val, QString &valExpression, const ValueParser *parser);
    void loadOasisAreaName(const KoXmlElement&body);
    void loadOasisMasterLayoutPage(KCSheet * table, KoStyleStack &styleStack);
    void loadOasisValidationValue(KCValidity val, const QStringList &listVal, const ValueParser *parser);
    QString translatePar(QString & par) const;
    void loadCondition(const KCCell& cell, const KoXmlElement &property);
    void loadOasisCondition(const KCCell& cell, const KoXmlElement &property);
    void loadOasisConditionValue(const QString &styleCondition, KCConditional &newCondition, const ValueParser *parser);
    void loadOasisCondition(QString &valExpression, KCConditional &newCondition, const ValueParser *parser);
    KoFilter::ConversionStatus loadAndParse(KoXmlDocument& doc, const QString& fileName, KoStore *m_store);

    KoFilter::ConversionStatus openFile();
};

#endif // OpenCalc_IMPORT_H__

