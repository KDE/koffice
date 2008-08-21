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

#ifndef KSPREADAPPLICATIONSETTINGS
#define KSPREADAPPLICATIONSETTINGS

#include <KGlobalSettings>

#include <QColor>
#include <QObject>

#include "Global.h"
#include "kspread_export.h"

namespace KSpread
{

/**
 * Visual settings.
 */
class KSPREAD_EXPORT ApplicationSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showVerticalScrollBar READ showVerticalScrollBar WRITE setShowVerticalScrollBar)
    Q_PROPERTY(bool showHorizontalScrollBar READ showHorizontalScrollBar WRITE setShowHorizontalScrollBar)
    Q_PROPERTY(bool showColumnHeader READ showColumnHeader WRITE setShowColumnHeader)
    Q_PROPERTY(bool showRowHeader READ showRowHeader WRITE setShowRowHeader)
    Q_PROPERTY(bool showStatusBar READ showStatusBar WRITE setShowStatusBar)
    Q_PROPERTY(bool showTabBar READ showTabBar WRITE setShowTabBar)

public:
    /**
     * Constructor.
     */
    ApplicationSettings();

    /**
     * Destructor.
     */
    ~ApplicationSettings();

    void load();
    void save() const;

    /**
     * If \c enable is true, vertical scrollbar is visible, otherwise
     * it will be hidden.
     */
    void setShowVerticalScrollBar(bool enable);

    /**
     * Returns true if vertical scroll bar is visible.
     */
    bool showVerticalScrollBar() const;

    /**
     * If \c enable is true, horizontal scrollbar is visible, otherwise
     * it will be hidden.
     */
    void setShowHorizontalScrollBar(bool enable);

    /**
     * Returns true if horizontal scroll bar is visible.
     */
    bool showHorizontalScrollBar() const;

    /**
     * If \c enable is true, column header is visible, otherwise
     * it will be hidden.
     */
    void setShowColumnHeader(bool enable);

    /**
     * Returns true if column header is visible.
     */
    bool showColumnHeader() const;

    /**
     * If \c enable is true, row header is visible, otherwise
     * it will be hidden.
     */
    void setShowRowHeader(bool enable);

    /**
     * Returns true if row header is visible.
     */
    bool showRowHeader() const;

    /**
     * Sets the color of the grid.
     */
    void setGridColor(const QColor& color);

    /**
     * Returns the color of the grid.
     */
    QColor gridColor() const;

    /**
     * Sets the indentation value.
     */
    void setIndentValue(double val);

    /**
     * Returns the indentation value.
     */
    double indentValue() const;

    /**
     * If \c enable is true, status bar is visible, otherwise
     * it will be hidden.
     */
    void setShowStatusBar(bool enable);

    /**
     * Returns true if status bar is visible.
     */
    bool showStatusBar() const;

    /**
     * If \c enable is true, tab bar is visible, otherwise
     * it will be hidden.
     */
    void setShowTabBar(bool enable);

    /**
     * Returns true if tab bar is visible.
     */
    bool showTabBar() const;

    /**
     * @return completion mode
     */
    KGlobalSettings::Completion completionMode() const;

    /**
     * Sets the completion mode.
     * @param mode the mode to be set
     */
    void setCompletionMode(KGlobalSettings::Completion mode);

    KSpread::MoveTo moveToValue() const;
    void setMoveToValue(KSpread::MoveTo moveTo);

    /**
     * Method of calc
     */
    void setTypeOfCalc(MethodOfCalc calc);
    MethodOfCalc getTypeOfCalc() const;

    QColor pageBorderColor() const;
    void changePageBorderColor(const QColor& color);

    void setCaptureAllArrowKeys(bool capture);
    bool captureAllArrowKeys() const;

private:
    class Private;
    Private * const d;
};

} // namespace KSpread

#endif // KSPREADAPPLICATIONSETTINGS
