// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <qrect.h>
#include <qstringlist.h>
#include <qtabwidget.h>
#include <qvalidator.h>

#include <kiconview.h>
#include <klistview.h>

class QListViewItem;
class KPrDocument;
class KPresenterView;
class QDropEvent;
class QPopupMenu;
class Outline;
class OutlineSlideItem;
class ThumbBar;
class ThumbToolTip;
class KPrPage;


class RenamePageValidator : public QValidator
{
  public:
    RenamePageValidator( const QStringList & list=QStringList() )
        : QValidator( 0, 0 ), mStringList( list ) {}
    virtual State validate( QString & input, int & pos ) const;

  protected:
    QStringList mStringList;
};

class SideBarBase
{
public:
    SideBarBase(KPrDocument *_doc, KPresenterView *_view);
    void setViewMasterPage( bool _b );
protected:
    KPrDocument *m_doc;
    KPresenterView *m_view;
    bool m_viewMasterPage;
};

class ThumbBar : public KIconView, public SideBarBase
{
    Q_OBJECT

public:
    ThumbBar(QWidget *parent, KPrDocument *d, KPresenterView *v);
    ~ThumbBar();
    void setCurrentPage( int pg );
    void updateItem( int pagenr, bool sticky = false );
    void addItem( int pos );
    void moveItem( int oldPos, int newPos );
    void removeItem( int pos );
    void refreshItems ( bool offset = false );
    QRect tip(const QPoint &pos, QString &title);

    bool uptodate;

signals:
    void showPage( int i );

public slots:
    void rebuildItems();

private slots:
    void itemClicked(QIconViewItem *i);
    void slotContentsMoving(int x, int y);
    void slotRefreshItems();

private:
    QPixmap getSlideThumb(int slideNr) const;

    ThumbToolTip *m_thumbTip;
    int m_offsetX;
    int m_offsetY;
};

class Outline: public KListView, public SideBarBase
{
    Q_OBJECT

public:
    Outline( QWidget *parent, KPrDocument *d, KPresenterView *v );
    ~Outline();
    void setCurrentPage( int pg );
    QSize sizeHint() const { return QSize( 145, KListView::sizeHint().height() ); }
    void updateItem( int pagenr, bool sticky = false);
    void addItem( int pos );
    void moveItem( int oldPos, int newPos );
    void removeItem( int pos );

protected:
    void contentsDropEvent( QDropEvent *e );
    void moveItem( QListViewItem *i, QListViewItem *firstAfter, QListViewItem *newAfter );
    OutlineSlideItem* slideItem( int pageNumber );
    bool acceptDrag( QDropEvent* e ) const;

    virtual QDragObject* dragObject();

signals: // all page numbers 0-based
    void showPage( int i );
    void movePage( int from, int to );
    void selectPage( int i, bool );

public slots:
    void rebuildItems();
    void renamePageTitle();

private slots:
    void itemClicked( QListViewItem *i );
    void slotDropped( QDropEvent *e, QListViewItem *parent, QListViewItem *target );
    void rightButtonPressed( QListViewItem *i, const QPoint &pnt, int c );
    void slotContextMenu( KListView*, QListViewItem *item, const QPoint &p );

private:
    QListViewItem *m_movedItem, *m_movedAfter;
};

class SideBar: public QTabWidget
{
    Q_OBJECT

public:
    SideBar(QWidget *parent, KPrDocument *d, KPresenterView *v);
    void setCurrentPage( int pg ) {
        m_outline->setCurrentPage(pg);
        m_thb->setCurrentPage(pg);
    };
    void setOn( int , bool ) { };
    //QSize sizeHint() const { return QSize( 120, QTabWidget::sizeHint().height() ); };
    void updateItem( KPrPage *page );
    void addItem( int pos );
    void moveItem( int oldPos, int newPos );
    void removeItem( int pos );

    Outline *outline() const { return m_outline; };
    ThumbBar *thumbBar() const { return m_thb; };

    void setViewMasterPage( bool _masterPage );
signals: // all page numbers 0-based
    void showPage( int i );
    void movePage( int from, int to );
    void selectPage( int i, bool );

public slots:
    //void rebuildItems() { m_outline->rebuildItems(); m_thb->rebuildItems();};
    void renamePageTitle() { m_outline->renamePageTitle(); };
    void currentChanged(QWidget *tab);

private:
    Outline *m_outline;
    ThumbBar *m_thb;

    KPrDocument *m_doc;
    KPresenterView *m_view;
};

#endif
