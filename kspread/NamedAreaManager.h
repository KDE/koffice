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

#ifndef KSPREAD_NAMED_AREA_MANAGER
#define KSPREAD_NAMED_AREA_MANAGER

#include <QList>
#include <QObject>

#include <KoXmlReader.h>

#include "Region.h"

#include "kspread_export.h"

class QDomDocument;
class QString;

class KoXmlWriter;

namespace KSpread
{
class Map;
class Region;

/**
 * Manages named cell areas.
 */
class KSPREAD_EXPORT NamedAreaManager : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructor.
     */
    NamedAreaManager(const Map* map);

    /**
     * Destructor.
     */
    virtual ~NamedAreaManager();

    void remove(Sheet* sheet);

    Region namedArea(const QString& name) const;
    Sheet* sheet(const QString& name) const;
    bool contains(const QString& name) const;

    /**
     * Returns the list of all registered named areas.
     * \return the list of named areas
     */
    QList<QString> areaNames() const;

    void regionChanged(const Region& region);
    void updateAllNamedAreas();

    /// \ingroup OpenDocument
    void loadOdf(const KoXmlElement& body);
    /// \ingroup OpenDocument
    void saveOdf(KoXmlWriter& xmlWriter) const;

    /// \ingroup NativeFormat
    void loadXML(const KoXmlElement& element);
    /// \ingroup NativeFormat
    QDomElement saveXML(QDomDocument& doc) const;

public Q_SLOTS:
    /**
     * Adds a named area.
     * \note The name is valid for the whole document.
     * \param region the cell range to be named
     * \param name the name of the new area
     */
    void insert(const Region& region, const QString& name);

    void remove(const QString& name);

Q_SIGNALS:
    void namedAreaAdded(const QString&);
    void namedAreaRemoved(const QString&);
    void namedAreaModified(const QString&);

private:
    class Private;
    Private * const d;
};

} // namespace KSpread

#endif // KSPREAD_NAMED_AREA_MANAGER
