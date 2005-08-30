/*
 * Kivio - Visual Modelling and Flowcharting
 * Copyright (C) 2000-2001 theKompany.com & Dave Marotti
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef KIVIO_ICON_VIEW_H
#define KIVIO_ICON_VIEW_H

#include <qiconview.h>

class QDragObject;

namespace Kivio
{
  class Object;
  class ShapeCollection;
}

class KivioIconView : public QIconView
{
  Q_OBJECT
  public:
    KivioIconView(bool _readWrite, QWidget *parent = 0, const char *name = 0);
    virtual ~KivioIconView();

    void setShapeCollection(Kivio::ShapeCollection* collection);
    Kivio::ShapeCollection* shapeCollection() const { return m_shapeCollection; }

  protected:
    QDragObject* dragObject();

  protected slots:
    void slotDoubleClicked(QIconViewItem*);

  private:
    bool isReadWrite;
    Kivio::ShapeCollection* m_shapeCollection;
};

class KivioIconViewItem : public QIconViewItem
{
  public:
    KivioIconViewItem( QIconView *parent );
    virtual ~KivioIconViewItem();

    void setShape(Kivio::Object* newShape);
    Kivio::Object* shape() const { return m_shape; }

    virtual bool acceptDrop( const QMimeSource *e ) const;

  protected:
    Kivio::Object* m_shape;
};


#endif

