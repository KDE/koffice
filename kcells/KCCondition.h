/* This file is part of the KDE project
   Copyright 2010 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
   Copyright 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 1999- 2006 The KCells Team <koffice-devel@kde.org>

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

#ifndef KC_CONDITION_H
#define KC_CONDITION_H

#include "KCStyle.h"
#include "KCValue.h"

#include <QDomElement>
#include <QLinkedList>
#include <QSharedData>
#include <QVariant>

#include "kcells_export.h"
#include <KXmlReader.h>

class QColor;
class QDomDocument;
class QFont;
class QString;
class KOdfGenericStyle;

class KCCell;
class KCValueConverter;
class KCValueParser;

/**
 * \class KCConditional
 * \ingroup KCStyle
 * KCConditional formatting.
 * Holds the actual condition and the applicable style for conditional
 * KCCell formattings.
 */
class KCELLS_EXPORT KCConditional
{
public:
    enum Type { None, Equal, Superior, Inferior, SuperiorEqual,
                InferiorEqual, Between, Different, DifferentTo,
                IsTrueFormula
              };

    KCValue          value1;
    KCValue          value2;
    QString        styleName;
    Type           cond;
    QString        baseCellAddress;

    KCConditional();

    bool operator==(const KCConditional &other) const;
};


class KCConditions;
uint qHash(const KCConditions& conditions);
uint qHash(const KCConditional& condition);

/**
 * \class KCConditions
 * \ingroup KCStyle
 * Manages a set of conditions for a cell.
 */
class KCELLS_EXPORT KCConditions
{
public:
    /**
     * Constructor.
     */
    KCConditions();

    /**
     * Copy Constructor.
     */
    KCConditions(const KCConditions& other);

    /**
     * Destructor..
     */
    ~KCConditions();

    /**
     * \return \c true if there are no conditions defined
     */
    bool isEmpty() const;

    /**
     * \return the style that matches first (or 0 if no condition matches)
     */
    KCStyle testConditions(const KCCell &cell) const;

    /**
     * Retrieve the current list of conditions we're checking
     */
    QLinkedList<KCConditional> conditionList() const;

    /**
     * Replace the current list of conditions with this new one
     */
    void setConditionList(const QLinkedList<KCConditional> & list);

    /**
     * Returns an optional default style, which is returned by testConditons if none of
     * the conditions matches.
     */
    KCStyle defaultStyle() const;

    /**
     * Set an optional default style. This style is returned by testConditions if none of
     * the conditions matches.
     */
    void setDefaultStyle(const KCStyle& style);

    /**
     * \ingroup NativeFormat
     * Takes a parsed DOM element and recreates the conditions structure out of
     * it
     */
    void loadConditions(const KXmlElement &element, const KCValueParser *parser);

    /**
     * \ingroup NativeFormat
     * Saves the conditions to a DOM tree structure.
     * \return the DOM element for the conditions.
     */
    QDomElement saveConditions(QDomDocument &doc, KCValueConverter *converter) const;

    /**
     * \ingroup OpenDocument
     * Loads the condtional formatting.
     */
    KCConditional loadOdfCondition(const QString &conditionValue, const QString &applyStyleName,
                                 const QString &baseCellAddress, const KCValueParser *parser);

    /**
     * \ingroup OpenDocument
     * Loads the condtional formattings.
     */
    void loadOdfConditions(const KXmlElement &element, const KCValueParser *parser, const KCStyleManager* styleManager);

    /**
     * \ingroup OpenDocument
     * Saves the condtional formattings.
     */
    void saveOdfConditions(KOdfGenericStyle &currentCellStyle, KCValueConverter *converter) const;

    /// \note implementation to make QMap happy (which is needed by KCRectStorage)
    bool operator<(const KCConditions& conditions) const {
        return qHash(*this) < qHash(conditions);
    }
    void operator=(const KCConditions& other);
    bool operator==(const KCConditions& other) const;
    inline bool operator!=(const KCConditions& other) const {
        return !operator==(other);
    }

private:
    /**
     * Use this function to see what conditions actually apply currently
     *
     * \param condition a reference to a condition that will be set to the
     *                  matching condition.  If none of the conditions are true
     *                  then this parameter is undefined on exit (check the
     *                  return value).
     *
     * \return true if one of the conditions is true, false if not.
     */
    bool currentCondition(const KCCell& cell, KCConditional & condition) const;

    bool isTrueFormula(const KCCell& cell, const QString& formula, const QString& baseCellAddress) const;

    /**
     * \ingroup OpenDocument
     */
    void loadOdfCondition(QString &valExpression, KCConditional &newCondition, const KCValueParser *parser);

    /**
     * \ingroup OpenDocument
     */
    void loadOdfConditionValue(const QString &styleCondition, KCConditional &newCondition, const KCValueParser *parser);

    /**
     * \ingroup OpenDocument
     */
    void loadOdfValidationValue(const QStringList &listVal, KCConditional &newCondition, const KCValueParser *parser);

    /**
     * \ingroup OpenDocument
     */
    QString saveOdfConditionValue(const KCConditional &conditionalStyle, KCValueConverter *converter) const;

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_METATYPE(KCConditions)
Q_DECLARE_TYPEINFO(KCConditions, Q_MOVABLE_TYPE);

#endif // KC_CONDITION_H
