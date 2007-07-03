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

#include <QObject>
#include <QSharedDataPointer>

#include <KoXmlReader.h>

class QRect;
class QWidget;

class KoXmlWriter;

namespace KSpread
{
class Cell;
class Filter;
class Region;
class Sheet;

/**
 * OpenDocument, 8.6.1 Database Range
 */
class Database : public QObject
{
    Q_OBJECT
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
    Database( const QString& name );

    /**
     * Copy Constructor.
     */
    Database( const Database& other );

    /**
     * Destructor.
     */
    virtual ~Database();

    /**
     * \return \c true if this is the default/empty database
     */
    bool isEmpty() const;

    /**
     * \return the database's orientation
     */
    Qt::Orientation orientation() const;

    /**
     * \return \c true if filter buttons should be displayed
     */
    bool displayFilterButtons() const;

    /**
     * Sets wether filter buttons should be displayed.
     */
    void setDisplayFilterButtons( bool enable );

    /**
     * \return the actual database cell range
     */
    const Region& range() const;

    /**
     * Sets the actual database cell range.
     * \p region has to be contiguous.
     */
    void setRange( const Region& region );

    /**
     * Shows the associated popup menu.
     */
    void showPopup(QWidget* parent, const Cell& cell, const QRect& cellRect);

    Filter* filter();

    bool loadOdf(const KoXmlElement& element, Sheet* const sheet);
    void saveOdf(KoXmlWriter& xmlWriter) const;

    void operator=( const Database& other );
    bool operator==( const Database& other ) const;
    bool operator<( const Database& other ) const;

    void dump() const;

private:
    class Private;
    QSharedDataPointer<Private> d;
};

} // namespace KSpread

#endif // KSPREAD_DATABASE
