/* This file is part of the KDE project
   Copyright (C) 2003 Ariya Hidayat <ariya@kde.org>
   Copyright (C) 2003 Norbert Andres <nandres@web.de>
   Copyright (C) 2002 Laurent Montel <montel@kde.org>
   Copyright (C) 1999 David Faure <faure@kde.org>
   Copyright (C) 1999 Boris Wedl <boris.wedl@kfunigraz.ac.at>
   Copyright (C) 1998-2000 Torben Weis <weis@kde.org>

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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef KSPREAD_TABBAR
#define KSPREAD_TABBAR

#include <qwidget.h>
#include <qpainter.h>
#include <qstrlist.h>

namespace KSpread
{


/**
 * This tab bar is used by @ref KSpreadView. It is used to choose between all
 * available tables.
 *
 * Adding, removing or renaming of tabs does not automatically add, rename or
 * remove KSpreadSheet objects. The tabbar is just a GUI element.
 *
 * But activating a tab emits a signal which in turn will show this table
 * in the associated KSpreadView.
 *
 * @short A bar with tabs and scroll buttons.
 */
class TabBar : public QWidget
{
    Q_OBJECT
public:
    TabBar( KSpreadView *_parent );
    virtual ~TabBar();

    /**
     * Adds a tab to the bar and paints it. The tab does not become active.
     * call @ref #setActiveTab to do so.
     */
    void addTab( const QString& _text );
    /**
     * Adds a hidden tab.
     *
     * @see KSpreadView::setActiveTable
     */
    void addHiddenTab( const QString & text );
    void removeHiddenTab(const QString & text);
    /**
     * Removes the tab from the bar. If the tab was the active one then the one
     * left of it ( or if not available ) the one right of it will become active.
     * It is recommended to call @ref #setActiveTab after a call to this function.
     */
    void removeTab( const QString& _text );

    /**
     * Renames a tab.
     */
    void renameTab( const QString& old_name, const QString& new_name );

    void rename( KSpreadSheet * table, QString name, QString const & activeName, bool ok );

    /**
     * Moves the tab with number _from befor tab number _to if @param _before is
     * true _from is inserted before _to. If false it is inserted after.
     */
    void moveTab( int _from, int _to, bool _before = true );

    /**
     * Removes all tabs from the bar and repaints the widget.
     */
    void removeAllTabs();

    bool canScrollLeft() const;

    bool canScrollRight() const;

    void scrollLeft();
    void scrollRight();
    void scrollFirst();
    void scrollLast();

    /**
     * Highlights this tab.
     */
    void setActiveTab( const QString& _text );

    /**
     * Open a context menu.
     */
    void openPopupMenu( const QPoint &_global );
    /**
    * Remove table name from tabList and
    * put tablename in tablehide
    * and highlights first name in tabList
    */
    void hideTable();
    void hideTable(const QString& tableName );


    /**
     * Shows the table. This makes only sense if
     * the table was hiddem before.
     *
     * The table does not become automatically the active one.
     */
    void showTable(const QString& _text);
    void showTable(QStringList list);
    void displayTable(const QString& _text);

    /**
     * @return the name of all visible tables.
     */
    QStringList listshow()const{return  tabsList;}
    /**
     * @return the name of all hidden tables.
     */
    QStringList listhide()const{return  tablehide;}


signals:
    /**
     * Emitted if the active tab changed. This will cause the
     * KSpreadView to change its active table, too.
     */
    void tabChanged( const QString& _text );

public slots:
    void slotRename( );

protected slots:
    /**
     * Opens a dialog to rename active tab.
     */
    void slotAdd();
    void slotAutoScroll( );

protected:
    virtual void paintEvent ( QPaintEvent* _ev );
    virtual void mousePressEvent ( QMouseEvent* _ev );
    virtual void mouseReleaseEvent ( QMouseEvent* _ev );
    virtual void mouseDoubleClickEvent ( QMouseEvent* _ev );
    virtual void mouseMoveEvent( QMouseEvent* _ev );

    void paintTab( QPainter & painter, int x, const QString& text, int text_width, int text_y, bool isactive, bool ismovemarked = false );

    void openPopupMenu( QPoint &_global );

    KSpreadView* m_pView;

    enum { autoScrollNo = 0, autoScrollLeft, autoScrollRight };
    enum { moveTabNo = 0, moveTabBefore, moveTabAfter };

    /**
     * List with the names of all tabs. The order in this list determines the
     * order of appearance.
     */
    QStringList tabsList;
     /*
    * list which contain names of table hide
    */
    QStringList tablehide;
    /**
     * Timer that causes the tabbar to scroll when the user drag a tab.
     */
    QTimer* m_pAutoScrollTimer;

    /**
     * This is the first visible tab on the left of the bar.
     */
    int leftTab;

    /**
     * This is the last fully visible tab on the right of the bar.
     */
    int m_rightTab;

    /**
     * The active tab in the range form 1..n.
     * If this value is 0, that means that no tab is active.
     */
    int activeTab;

    /**
     * Indicates whether a tab may be scrolled while moving a table.
     * Used to provide a timeout.
     */
    bool m_mayScroll;

    /**
     * The number of the tab being moved using the mouse.
     * If no tab is being moved this value is 0.
     */
    int m_moveTab;

    /**
     * Indicates whether a tab is being moved using the mouse and in which
     * direction.
     */
    int m_moveTabFlag;

    /**
     * Indicates the direction the tabs are scrolled to.
     */
    int m_autoScroll;
};

};

// for source compatibility only, remove in the future
typedef KSpread::TabBar KSpreadTabBar;

#endif // KSPREAD_TABBAR
