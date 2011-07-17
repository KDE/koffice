/* This file is part of the KDE project
   Copyright 2008 Stefan Nikolaus <stefan.nikolaus@kdemail.net>

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

#ifndef KCELLS_CALENDAR_TOOL
#define KCELLS_CALENDAR_TOOL

#include <part/KCCellTool.h>

class CalendarTool : public KCCellTool
{
    Q_OBJECT
public:
    explicit CalendarTool(KCanvasBase* canvas);
    ~CalendarTool();

public Q_SLOTS:
    virtual void activate(ToolActivation toolActivation, const QSet<KShape*> &shapes);
    virtual void deactivate();

    /**
     * This actually inserts the calendar. It reads the configuration
     * from the insert calendar dialog and builds an calendar in the
     * spreadsheet accordingly.
     */
    void insertCalendar(const QDate &start, const QDate &end);

private:
    virtual QWidget* createOptionWidget();
    void setText(KCSheet* sheet, int row, int column, const QString& text, bool asString = false);

private:
    Q_DISABLE_COPY(CalendarTool)

    class Private;
    Private * const d;
};

#endif // KCELLS_CALENDAR_TOOL
