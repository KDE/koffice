/* This file is part of the KDE project
   Copyright (c) 2005 Tomislav Lukman <tomislav.lukman@ck.t-com.hr>
   Copyright (c) 2008 Jan Hambrecht <jaham@gmx.net>

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


#ifndef ARTWORKSMALLSTYLEPREVIEW_H
#define ARTWORKSMALLSTYLEPREVIEW_H

#include <KoCheckerBoardPainter.h>
#include <QtGui/QWidget>

class QPaintEvent;
class ArtworkFillStyleWidget;
class ArtworkStrokeStyleWidget;
class KCanvasBase;

/// This is a small widget used on the statusbar, to display fill/stroke colors etc.
class ArtworkSmallStylePreview : public QWidget
{
    Q_OBJECT
public:
    explicit ArtworkSmallStylePreview(QWidget* parent = 0L);
    virtual ~ArtworkSmallStylePreview();

signals:
    void fillApplied();
    void strokeApplied();

private slots:
    void selectionChanged();
    void canvasChanged(const KCanvasBase *canvas);

private:
    ArtworkFillStyleWidget * m_fillFrame;
    ArtworkStrokeStyleWidget * m_strokeFrame;
};

#endif // ARTWORKSMALLSTYLEPREVIEW_H
