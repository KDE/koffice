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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef docstruct_h
#define docstruct_h

#include <qlistview.h>
#include <qwidget.h>

class KWordDocument;
class KWordGUI;
class QWidget;
class KWFrame;
class KWFrameSet;
class KWParag;

/******************************************************************/
/* Class: KWDocStructParagItem                                    */
/******************************************************************/

class KWDocStructParagItem : public QObject,
                             virtual public QListViewItem
{
    Q_OBJECT

public:
    KWDocStructParagItem( QListViewItem *_parent, QString _text, KWParag *_parag, KWordGUI *__parent );
    KWDocStructParagItem( QListViewItem *_parent, QListViewItem *_after, QString _text, KWParag *_parag, KWordGUI*__parent );

public slots:
    void slotDoubleClicked( QListViewItem *_item );

protected:
    KWParag *parag;
    KWordGUI *gui;

};

/******************************************************************/
/* Class: KWDocStructFrameItem                                    */
/******************************************************************/

class KWDocStructFrameItem : public QObject,
                             virtual public QListViewItem
{
    Q_OBJECT

public:
    KWDocStructFrameItem( QListViewItem *_parent, QString _text, KWFrameSet *_frameset, KWFrame *_frame, KWordGUI *__parent );

public slots:
    void slotDoubleClicked( QListViewItem *_item );

protected:
    KWFrame *frame;
    KWFrameSet *frameset;
    KWordGUI *gui;

};

/******************************************************************/
/* Class: KWDocStructTableItem                                    */
/******************************************************************/

class KWDocStructTableItem : public QObject,
                             virtual public QListViewItem
{
    Q_OBJECT

public:
    KWDocStructTableItem( QListViewItem *_parent, QString _text, KWGroupManager *_table, KWordGUI*__parent );

public slots:
    void slotDoubleClicked( QListViewItem *_item );

protected:
    KWGroupManager *table;
    KWordGUI *gui;

};

/******************************************************************/
/* Class: KWDocStructPictureItem                                  */
/******************************************************************/

class KWDocStructPictureItem : public QObject,
                               virtual public QListViewItem
{
    Q_OBJECT

public:
    KWDocStructPictureItem( QListViewItem *_parent, QString _text, KWPictureFrameSet *_pic, KWordGUI*__parent );

public slots:
    void slotDoubleClicked( QListViewItem *_item );

protected:
    KWPictureFrameSet *pic;
    KWordGUI *gui;

};

/******************************************************************/
/* Class: KWDocStructPartItem                                     */
/******************************************************************/

class KWDocStructPartItem : public QObject,
                            virtual public QListViewItem
{
    Q_OBJECT

public:
    KWDocStructPartItem( QListViewItem *_parent, QString _text, KWPartFrameSet *_part, KWordGUI*__parent );

public slots:
    void slotDoubleClicked( QListViewItem *_item );

protected:
    KWPartFrameSet *part;
    KWordGUI *gui;

};

/******************************************************************/
/* Class: KWDocStructRootItem                                     */
/******************************************************************/

class KWDocStructRootItem : public QListViewItem
{
public:
    enum Type {Arrangement, Tables, Pictures, Cliparts, TextFrames, Embedded};

    KWDocStructRootItem( QListView *_parent, KWordDocument *_doc, Type _type, KWordGUI*__parent );

    void setupArrangement();
    void setupTextFrames();
    void setupTables();
    void setupPictures();
    void setupCliparts();
    void setupEmbedded();

    virtual void setOpen( bool o );

protected:
    KWordDocument *doc;
    Type type;
    KWordGUI *gui;

};

/******************************************************************/
/* Class: KWDocStructTree                                         */
/******************************************************************/

class KWDocStructTree : public QListView
{
    Q_OBJECT

public:
    KWDocStructTree( QWidget *_parent, KWordDocument *_doc, KWordGUI*__parent );

    void setup();

protected:
    KWordDocument *doc;
    KWordGUI *gui;

    KWDocStructRootItem *arrangement, *tables, *pictures, *cliparts, *textfrms, *embedded;

};

/******************************************************************/
/* Class: KWDocStruct                                             */
/******************************************************************/

class KWDocStruct : public QWidget
{
    Q_OBJECT

public:
    KWDocStruct( QWidget *_parent, KWordDocument *_doc, KWordGUI*__parent );

protected:
    KWDocStructTree *tree;
    QGridLayout *layout;

    KWordDocument *doc;
    KWordGUI *parent;

};

#endif
