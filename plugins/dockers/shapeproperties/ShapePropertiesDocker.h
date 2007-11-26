/* This file is part of the KDE project
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef SHAPEPROPERTIESDOCKER_H
#define SHAPEPROPERTIESDOCKER_H

#include <KoDockFactory.h>
#include <KoCanvasObserver.h>
#include <QtGui/QDockWidget>

class KoShape;

/// The shape properties docker show the properties
/// of the currently selected shape
class ShapePropertiesDocker : public QDockWidget, public KoCanvasObserver
{
    Q_OBJECT
public:
    explicit ShapePropertiesDocker(QWidget *parent = 0);
    ~ShapePropertiesDocker();

    /// reimplemented
    virtual void setCanvas( KoCanvasBase *canvas );

private slots:
    void selectionChanged();
    void addWidgetForShape( KoShape * shape );
    void shapePropertyChanged();
    virtual void resourceChanged( int key, const QVariant & res );
private:
    class Private;
    Private * const d;
};

#endif // SHAPEPROPERTIESDOCKER_H
