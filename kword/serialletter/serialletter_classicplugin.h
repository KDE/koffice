/* This file is part of the KDE project
   Original file (serialletter.h): Copyright (C) 1998, 1999 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>

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

#ifndef _SERIALLETTER_CLASSIC_PLUGIN_H_
#define _SERIALLETTER_CLASSIC_PLUGIN_H_

#include <qdom.h>
#include <qlistview.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qhbox.h>
#include <qpushbutton.h>
#include <qlineedit.h>
#include <qvalidator.h>
#include <qheader.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <kdebug.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <qspinbox.h>
#include <qvbox.h>

#include "kwdoc.h"
#include "variabledlgs.h"
#include "kwutils.h"
#include "defs.h"
#include "serialletter_interface.h"

/******************************************************************
 *
 * Class: KWClassicSerialDataSource
 *
 ******************************************************************/
typedef QValueList< DbRecord > Db;

class KWClassicSerialDataSource: public KWSerialLetterDataSource
{
    public:
    KWClassicSerialDataSource(/*KWSerialLetterDataBase *kwsldb*/);
    ~KWClassicSerialDataSource();

    virtual void save( QDomElement& parentElem );
    virtual void load( QDomElement& elem );
    virtual class QString getValue( const class QString &name, int record = -1 ) const;
    virtual int getNumRecords() const {
        return (int)db.count();
    }

    virtual  void showConfigDialog(QWidget *,int);

    protected:
    friend class KWClassicSerialLetterEditor;
    friend class KWClassicSerialLetterEditorList;

    void setValue( const QString &name, const QString &value, int record = -1 );
    void appendRecord();
    void addEntry( const QString &name );
    void removeEntry( const QString &name );
    void removeRecord( int i );
    Db db;
};

/******************************************************************
 *
 * Class: KWClassicSerialLetterEditorListItem
 *
 ******************************************************************/

class KWClassicSerialLetterEditorListItem : public QListViewItem
{
public:
    KWClassicSerialLetterEditorListItem( QListView *parent );
    KWClassicSerialLetterEditorListItem( QListView *parent, QListViewItem *after );
    virtual ~KWClassicSerialLetterEditorListItem();

    virtual void setText( int i, const QString &text );
    virtual QString text( int i ) const;
    void setup();
    void update();

protected:
    QLineEdit *editWidget;

};

/******************************************************************
 *
 * Class: KWClassicSerialLetterEditorList
 *
 ******************************************************************/

class KWClassicSerialLetterEditorList : public QListView
{
    Q_OBJECT

public:
    KWClassicSerialLetterEditorList( QWidget *parent, KWClassicSerialDataSource *db_ );
    virtual ~KWClassicSerialLetterEditorList();

    void updateItems();
    void displayRecord( int i );

    void setSorting( int, bool increasing = TRUE ) {
        QListView::setSorting( -1, increasing );
    }

protected slots:
    void columnSizeChange( int c, int os, int ns );
    void sectionClicked( int c );

protected:
    KWClassicSerialDataSource *db;
    int currentRecord;

};

/******************************************************************
 *
 * Class: KWClassicSerialLetterEditor
 *
 ******************************************************************/

class KWClassicSerialLetterEditor : public KDialogBase
{
    Q_OBJECT

public:
    KWClassicSerialLetterEditor( QWidget *parent, KWClassicSerialDataSource *db_ );

protected:
    void resizeEvent( QResizeEvent *e );

    QSpinBox *records;
    KWClassicSerialLetterEditorList *dbList;
    QVBox *back;
    KWClassicSerialDataSource *db;

    QToolButton *first;
    QToolButton *back_;
    QToolButton *forward;
    QToolButton *finish;
    QToolButton *newRecord;
    QToolButton *newEntry;
    QToolButton *deleteRecord;
    QToolButton *deleteEntry;

protected slots:
    void changeRecord( int i );
    void addEntry();
    void addRecord();
    void removeEntry();
    void removeRecord();
    void firstRecord() { records->setValue(1); }
    void prevRecord() { records->setValue(records->value()-1); }
    void nextRecord() { records->setValue(records->value()+1); }
    void lastRecord() { records->setValue(records->maxValue()); }
};

#endif
