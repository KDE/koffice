/* This file is part of the KDE project
   Copyright (C) 2008 Pierre Stirnweiss <pierre.stirnweiss_koffice@gadz.org>
   Copyright (C) 2010 Thomas Zander <zander@kde.org>

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

#ifndef KODFGENERICCHANGES_H
#define KODFGENERICCHANGES_H

#include "flake_export.h"
#include "KOdfGenericStyle.h"			//krazy:exclude=includes
#include "KOdfGenericChange.h"			//krazy:exclude=includes

/**
 * @brief Repository of changes used during saving of OASIS/OOo file.
 *
 * Inspired from KOdfGenericStyles.h
 *
 * Is used to store all the change regions, which will be saved at the beginning of <office:body><office:text> elements
 * We use a container outside the changeTracker, as the change tracker is linked to the document of a TextShapeData and is then not aware of the other TextShapeData.
 *
 */
class FLAKE_EXPORT KOdfGenericChanges
{
public:
    KOdfGenericChanges();
    ~KOdfGenericChanges();

    /**
     * Look up a change in the collection, inserting it if necessary.
     * This assigns a name to the change and returns it.
     *
     * @param change the change to look up.
     * @param name proposed internal name for the change.
     * If this name is already in use (for another change), then a number is appended
     * to it until unique.
     *
     * @return the name for this change
     */
    QString insert(const KOdfGenericChange &change, const QString &name = QString());

    /**
     * Return the entire collection of styles
     * Use this for saving the styles
     */
    QMap<KOdfGenericChange, QString> changes() const;

    /**
     * @return an existing change by name
     */
    const KOdfGenericChange *change(const QString &name) const;

    /**
     * Save changes.
     *
     * This creates the text:changed-region tag containing all
     * changes.
     *
     * @param xmlWriter
     */
    void saveOdfChanges(KXmlWriter *xmlWriter) const;

private:
    class Private;
    Private * const d;
};

#endif /* KODFGENERICCHANGES_H */
