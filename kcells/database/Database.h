/* This file is part of the KDE project
   Copyright 2007 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KSPREAD_DATABASE
#define KSPREAD_DATABASE

#include <QSharedDataPointer>
#include <QVariant>

#include <KXmlReader.h>

#include "../kcells_export.h"

class QRect;
class QWidget;

class KXmlWriter;

class KCCell;
class Filter;
class KCMap;
class KCRegion;

/**
 * OpenDocument, 8.6.1 Database Range
 */
class KCELLS_EXPORT Database
{
public:
    /**
     * Constructor.
     * Creates an empty database.
     */
    Database();

    /**
     * Constructor.
     * Creates a database named \p name.
     */
    Database(const QString& name);

    /**
     * Copy Constructor.
     */
    Database(const Database& other);

    /**
     * Destructor.
     */
    ~Database();

    /**
     * \return \c true if this is the default/empty database
     */
    bool isEmpty() const;

    /**
     * \return the database's name
     */
    const QString& name() const;

    /**
     * Sets the database's name.
     */
    void setName(const QString& name);

    /**
     * \return the database's orientation
     */
    Qt::Orientation orientation() const;

    /**
     * \return \c true if the range contains a header column/row
     */
    bool containsHeader() const;

    /**
     * Sets whether the range contains a header column/row
     */
    void setContainsHeader(bool enable);

    /**
     * \return \c true if filter buttons should be displayed
     */
    bool displayFilterButtons() const;

    /**
     * Sets whether filter buttons should be displayed.
     */
    void setDisplayFilterButtons(bool enable);

    /**
     * \return the actual database cell range
     */
    const KCRegion& range() const;

    /**
     * Sets the actual database cell range.
     * \p region has to be contiguous.
     */
    void setRange(const KCRegion& region);

    const Filter& filter() const;
    void setFilter(const Filter& filter);

    bool loadOdf(const KoXmlElement& element, const KCMap* map);
    void saveOdf(KXmlWriter& xmlWriter) const;

    void operator=(const Database& other);
    bool operator==(const Database& other) const;
    bool operator<(const Database& other) const;

    void dump() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

Q_DECLARE_METATYPE(Database)
Q_DECLARE_TYPEINFO(Database, Q_MOVABLE_TYPE);

#endif // KSPREAD_DATABASE
