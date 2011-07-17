/* This file is part of the KDE project

   Copyright 2004 Laurent Montel <montel@kde.org>

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
   Boston, MA 02110-1301, USA.
*/


#ifndef KCELLS_GENVALIDATIONSTYLE
#define KCELLS_GENVALIDATIONSTYLE

#include "kcells_export.h"

#include <QMap>
#include <QString>

class KXmlWriter;

class KCValidity;
class KCValueConverter;
class KCGenValidationStyles;

/**
 * \class KCGenValidationStyle
 * \ingroup OpenDocument
 */
class KCGenValidationStyle
{
public:
    explicit KCGenValidationStyle(KCValidity *_val = 0, const KCValueConverter *converter = 0) {
        initVal(_val, converter);
    }


    bool operator<(const KCGenValidationStyle &other) const {
        if (allowEmptyCell != other.allowEmptyCell) return (allowEmptyCell < other.allowEmptyCell);
        if (condition != other.condition) return (condition < other.condition);
        if (titleInfo != other.titleInfo) return (titleInfo < other.titleInfo);
        if (displayValidationInformation != other.displayValidationInformation) return (displayValidationInformation < other.displayValidationInformation);
        if (messageInfo != other.messageInfo) return (messageInfo < other.messageInfo);
        if (messageType != other.messageType) return (messageType < other.messageType);
        if (displayMessage != other.displayMessage) return (displayMessage < other.displayMessage);
        if (message != other.message) return (message < other.message);
        if (title != other.title) return (title < other.title);

        return false;
    }
private:
    QString createValidationCondition(KCValidity* _val, const KCValueConverter *converter);
    QString createTextValidationCondition(KCValidity* _val);
    QString createTimeValidationCondition(KCValidity* _val, const KCValueConverter *converter);
    QString createDateValidationCondition(KCValidity* _val, const KCValueConverter *converter);
    QString createNumberValidationCondition(KCValidity* _val);
    QString createListValidationCondition(KCValidity* _val);

    void initVal(KCValidity *_val, const KCValueConverter *converter);

    QString allowEmptyCell;
    QString condition;
    QString titleInfo;
    QString displayValidationInformation;
    QString messageInfo;
    QString messageType;
    QString displayMessage;
    QString message;
    QString title;
    friend class KCGenValidationStyles;
};

/**
 * \class KCGenValidationStyles
 * \ingroup OpenDocument
 */
class KCELLS_EXPORT KCGenValidationStyles
{
public:
    KCGenValidationStyles();
    ~KCGenValidationStyles();
    QString insert(const KCGenValidationStyle& style);

    typedef QMap<KCGenValidationStyle, QString> StyleMap;
    void writeStyle(KXmlWriter& writer);

private:
    QString makeUniqueName(const QString& base) const;

    /// style definition -> name
    StyleMap m_styles;
    /// name -> style   (used to check for name uniqueness)
    typedef QMap<QString, bool> NameMap;
    NameMap m_names;

};

#endif
