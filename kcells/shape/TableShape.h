/* This file is part of the KDE project
   Copyright 2006 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KCELLS_TABLE_SHAPE
#define KCELLS_TABLE_SHAPE

#include <QObject>

#include <KShape.h>

#define TableShapeId "TableShape"

class KCDamage;
class KCMap;
class KCSheet;
class SheetView;

class TableShape : public QObject, public KShape
{
    Q_OBJECT

public:
    explicit TableShape(int columns = 2, int rows = 8);
    virtual ~TableShape();

    int columns() const;
    int rows() const;
    void setColumns(int columns);
    void setRows(int rows);

    // KShape interface
    virtual void paint(QPainter& painter, const KViewConverter& converter);
    virtual bool loadOdf(const KXmlElement & element, KShapeLoadingContext &context);
    virtual void saveOdf(KShapeSavingContext & context) const;
    virtual void setSize(const QSizeF &size);

    /**
     * \return the map containing the data for this shape
     */
    KCMap* map() const;
    void setMap(KCMap *map);

    /**
     * \return the sheet containing the data for this shape
     */
    KCSheet* sheet() const;

    SheetView* sheetView() const;

    /**
     * Set the current sheet to the sheet with name \p sheetName .
     */
    void setSheet(const QString& sheetName);

    void setVisibleCellRange(const QRect& cellRange);

protected:
    virtual void shapeChanged(ChangeType type);

private Q_SLOTS:
    void handleDamages(const QList<KCDamage*>& damages);

private:
    Q_DISABLE_COPY(TableShape)

    class Private;
    Private * const d;
};

#endif // KCELLS_TABLE_SHAPE
