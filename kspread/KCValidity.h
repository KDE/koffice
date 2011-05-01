/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>
   Copyright 2004 Tomas Mecir <mecirt@gmail.com>
   Copyright 1999-2002,2004 Laurent Montel <montel@kde.org>
   Copyright 2002,2004 Ariya Hidayat <ariya@kde.org>
   Copyright 2002-2003 Norbert Andres <nandres@web.de>
   Copyright 2003 Stefan Hetzl <shetzl@chello.at>
   Copyright 2001-2002 Philipp Mueller <philipp.mueller@gmx.de>
   Copyright 2002 Harri Porten <porten@kde.org>
   Copyright 2002 John Dailey <dailey@vt.edu>
   Copyright 1999-2001 David Faure <faure@kde.org>
   Copyright 2000-2001 Werner Trobin <trobin@kde.org>
   Copyright 2000 Simon Hausmann <hausmann@kde.org
   Copyright 1998-1999 Torben Weis <weis@kde.org>
   Copyright 1999 Michael Reiher <michael.reiher@gmx.de>
   Copyright 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef KSPREAD_VALIDITY
#define KSPREAD_VALIDITY

// Qt
#include <QDate>
#include <QHash>
#include <QSharedDataPointer>
#include <QStringList>
#include <QTime>
#include <QVariant>

// KOffice
#include "kspread_export.h"

// KSpread
#include "KCCondition.h"

#include "KoXmlReaderForward.h"

class KCOdfLoadingContext;
class KCValueConverter;
class KCValueParser;

/**
 * \class KCValidity
 * \ingroup KCValue
 *
 * Validates cell contents.
 *
 * \author Stefan Nikolaus <stefan.nikolaus@kdemail.net>
 */
class KSPREAD_EXPORT KCValidity
{
public:
    /// The action invoked, if the validity check fails.
    enum Action {
        Stop,       ///< Stop
        Warning,    ///< Warn
        Information ///< Inform
    };
    /// The type of the restriction.
    enum Restriction {
        None,       ///< No restriction
        KCNumber,     ///< Restrict to numbers
        Text,       ///< Restrict to texts
        Time,       ///< Restrict to times
        Date,       ///< Restrict to dates
        Integer,    ///< Restrict to integers
        TextLength, ///< Restrict text length
        List        ///< Restrict to lists
    };

    /**
     * Constructor.
     * Creates a validity check, that allows any content.
     */
    KCValidity();

    /**
     * Copy Constructor.
     * Copies the validity \p other .
     */
    KCValidity(const KCValidity& other);

    /**
     * Destructor.
     */
    ~KCValidity();

    /**
     * \return \c true if this validity check allows any content
     */
    bool isEmpty() const;

    /**
     * Tests whether the content of \p cell is allowed.
     * \return \c true if the content is valid
     */
    bool testValidity(const KCCell* cell) const;

    /**
     * \ingroup NativeFormat
     * Loads validity checks.
     */
    bool loadXML(KCCell* const cell, const KoXmlElement& validityElement);

    /**
     * \ingroup NativeFormat
     * Saves validity checks.
     */
    QDomElement saveXML(QDomDocument& doc, const KCValueConverter *converter) const;

    /**
     * \ingroup OpenDocument
     * Loads validity checks.
     */
    void loadOdfValidation(KCCell* const cell, const QString& validationName,
                           KCOdfLoadingContext& tableContext);

    Action action() const;
    bool allowEmptyCell() const;
    KCConditional::Type condition() const;

    bool displayMessage() const;
    bool displayValidationInformation() const;
    const QString& messageInfo() const;
    const QString& message() const;

    const KCValue &maximumValue() const;
    const KCValue &minimumValue() const;

    Restriction restriction() const;
    const QString& title() const;
    const QString& titleInfo() const;
    const QStringList& validityList() const;

    void setAction(Action action);
    void setAllowEmptyCell(bool allow);
    void setCondition(KCConditional::Type condition);

    void setDisplayMessage(bool display);
    void setDisplayValidationInformation(bool display);
    void setMessage(const QString& message);
    void setMessageInfo(const QString& info);

    void setMaximumValue(const KCValue &value);
    void setMinimumValue(const KCValue &value);

    void setRestriction(Restriction restriction);
    void setTitle(const QString& title);
    void setTitleInfo(const QString& info);
    void setValidityList(const QStringList& list);

    /// \note fake implementation to make QMap happy
    bool operator<(const KCValidity&) const {
        return true;
    }
    void operator=(const KCValidity&);
    bool operator==(const KCValidity& other) const;
    inline bool operator!=(const KCValidity& other) const {
        return !operator==(other);
    }

    static QHash<QString, KoXmlElement> preloadValidities(const KoXmlElement& body);

private:
    /**
     * \ingroup OpenDocument
     * Helper method for loadOdfValidation().
     */
    void loadOdfValidationCondition(QString &valExpression, const KCValueParser *parser);

    /**
     * \ingroup OpenDocument
     * Helper method for loadOdfValidation().
     */
    void loadOdfValidationValue(const QStringList &listVal, const KCValueParser *parser);

    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_METATYPE(KCValidity)
Q_DECLARE_TYPEINFO(KCValidity, Q_MOVABLE_TYPE);

#endif // KSPREAD_VALIDITY
